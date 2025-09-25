#include "nic.hpp"

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

NIC::NIC(sc_module_name name, const int &num_tx_port, const int &num_rx_port,
         const int &desc_fifo_szie, const int &tx_queue_size, const int &rx_queue_size)
    : sc_module(name),
      data_initiator("data_initiator"),
      desc_target("desc_target"),
      tx_initiators("tx_initiators", num_tx_port),
      rx_targets("rx_targets", num_rx_port),
      tx_ports("tx_ports", num_tx_port),
      rx_ports("rx_ports", num_rx_port),
      dma("dma", desc_fifo_szie, tx_queue_size, rx_queue_size)
{
    
    dma.data_initiator.bind(data_initiator);
    desc_target.bind(dma.desc_target);
    
    for (int i = 0; i < tx_ports.size(); i++)
    {
        tx_ports[i].tx_initiator.bind(tx_initiators[i]);
    }
    for (int i = 0; i < rx_ports.size(); i++)
    {
        rx_targets[i].bind(rx_ports[i].rx_target);
    }

    if (num_tx_port > 0)
    {
        SC_THREAD(send);
    }
    if (num_rx_port > 0)
    {
        SC_THREAD(receive);
    }
}

void NIC::send()
{   
    int port_id = 0;
    while (true)
    {
        SC_REPORT_INFO("NIC", "Starting send process...");

        vector<unsigned char> pkt = dma.read_packet_fifo.read();

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

        if (port_id < tx_ports.size() - 1)
        {
            port_id++;
        }
        else
        {
            port_id = 0;
        }

        wait(delay);
    }
}

void NIC::receive()
{
    int port_id = 0;
    while (true)
    {
        SC_REPORT_INFO("NIC", "Starting receive process...");

        vector<unsigned char> pkt = rx_ports[port_id].packet_fifo.read();

        dma.write_packet_fifo.write(move(pkt));

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
