#include "eth_switch.hpp"

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

MacAddrTable::MacAddrTable(int size) : table(size) {}

int MacAddrTable::insert(const sc_uint<48> &mac_addr, const int &port_id)
{
    for (int i = 0; i < table.size(); i++)
    {
        if (!table[i].valid)
        {
            table[i] = {mac_addr, port_id, sc_time_stamp(), true};
            return i;
        }
    }
    return -1;
}

void MacAddrTable::insert(const sc_uint<48> &mac_addr, const int &port_id, const int &pos)
{
    if (pos >= 0 && pos < table.size())
    {
        table[pos] = {mac_addr, port_id, sc_time_stamp(), true};
    }
}

void MacAddrTable::remove(int pos)
{
    if (pos >= 0 && pos < table.size())
        table[pos].valid = false;
}

int MacAddrTable::find_oldest_entry()
{
    int oldest = 0;
    for (int i = 1; i < table.size(); i++)
    {
        if (table[i].valid && table[i].time_stamp < table[oldest].time_stamp)
            oldest = i;
    }
    return oldest;
}

int MacAddrTable::lookup(const sc_uint<48> &mac_addr)
{
    for (int i = 0; i < table.size(); i++)
    {
        if (table[i].valid && table[i].mac_addr == mac_addr)
            return table[i].port_id;
    }
    return -1;
}

ostream &operator<<(ostream &os, const ProcFifoEntry &entry)
{
    os << "ProcFifoEntry[port_id=" << entry.port_id
       << ", packet=" << entry.pkt << "]";
    return os;
}

EthSwitch::EthSwitch(sc_module_name name,
                     const int &num_tx_port,
                     const int &num_rx_port,
                     const int &fifo_size,
                     const int &mac_addr_table_size)
    : sc_module(name),
      tx_initiators("tx_initiators", num_tx_port),
      rx_targets("rx_targets", num_rx_port),
      tx_ports("tx_ports", num_tx_port),
      rx_ports("rx_ports", num_rx_port),
      mac_addr_table(mac_addr_table_size),
      process_fifo("process_fifo", fifo_size),
      mac_addr_table_size(mac_addr_table_size),
      used_mac_addr_entries_num(0)
{
    for (int i = 0; i < tx_ports.size(); i++)
    {
        tx_ports[i].tx_initiator.bind(tx_initiators[i]);
    }
    for (int i = 0; i < rx_ports.size(); i++)
    {
        rx_targets[i].bind(rx_ports[i].rx_target);
    }

    SC_THREAD(forward);
    SC_THREAD(schedule);
}

void EthSwitch::set_mac_addr_table(const sc_dt::sc_uint<48> &mac_addr, const int &port_id){
    mac_addr_table.insert(mac_addr, port_id);
}

void EthSwitch::learn(const sc_uint<48> &mac_addr, int &port_id)
{
    SC_REPORT_INFO("EthSwitch", ("Learn mac_addr from port " + to_string(port_id)).c_str());

    if (mac_addr_table.lookup(mac_addr) < 0)
    {
        mac_addr_table.insert(mac_addr, port_id);
    }
}

void EthSwitch::send(int port_id, vector<unsigned char> &pkt)
{
    SC_REPORT_INFO("EthSwitch", ("Sending packet to port " + to_string(port_id)).c_str());

    tlm_generic_payload trans;
    trans.set_command(TLM_WRITE_COMMAND);
    trans.set_data_ptr(pkt.data());
    trans.set_data_length(pkt.size());
    trans.set_streaming_width(pkt.size());
    trans.set_byte_enable_ptr(nullptr);
    trans.set_dmi_allowed(false);
    tlm_phase phase = BEGIN_REQ;

    sc_time delay = SC_ZERO_TIME;

    tx_initiators[port_id]->nb_transport_fw(trans, phase, delay);

    wait(delay);
}

void EthSwitch::forward()
{   
    while (true)
    {
        SC_REPORT_INFO("EthSwitch", "Starting forward process...");

        ProcFifoEntry proc_fifo_entry = process_fifo.read();

        int src_port_id = proc_fifo_entry.port_id;
        vector<unsigned char> pkt = proc_fifo_entry.pkt;

        Packet packet;
        packet.deserialize(pkt);

        sc_uint<48> src_mac_addr = packet.eth_ii.src_mac_addr;
        sc_uint<48> dest_mac_addr = packet.eth_ii.dest_mac_addr;

        learn(src_mac_addr, src_port_id);

        int dest_port_id = mac_addr_table.lookup(dest_mac_addr);

        if (dest_port_id >= 0)
        {
            send(dest_port_id, pkt);
        }
        else
        {
            for (int i = 0; i < tx_ports.size(); i++)
            {
                if (i != src_port_id)
                {
                    send(i, pkt);
                }
            }
        }

        sc_time delay = sc_time(10, SC_NS);
        wait(delay);
    }
}

void EthSwitch::schedule()
{
    int port_id = 0;
    while (true)
    {
        SC_REPORT_INFO("EthSwitch", "Starting schedule process...");

        vector<unsigned char> pkt;
        if(rx_ports[port_id].packet_fifo.nb_read(pkt)){
            ProcFifoEntry proc_fifo_entry = {port_id, move(pkt)};
            process_fifo.write(move(proc_fifo_entry));
        }

        if (port_id < rx_ports.size() - 1)
        {
            port_id++;
        }
        else
        {
            port_id = 0;
        }

        sc_time delay = sc_time(1, SC_NS);
        wait(delay);
    }
}

}