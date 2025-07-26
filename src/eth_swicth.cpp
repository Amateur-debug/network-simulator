#include "eth_switch.hpp"

namespace netsim
{

MacAddrTable::MacAddrTable(int size)
    : table(size)
{
}

int MacAddrTable::insert(const string &mac_addr, int &port_id)
{
    for (int i = 0; i < table.size(); i++)
    {

        if (!table[i].valid)
        {
            table[i].port_id = port_id;
            table[i].mac_addr = mac_addr;
            table[i].time_stamp = sc_time_stamp();
            table[i].valid = true;

            return 0;
        }
    }

    return -1;
}

void MacAddrTable::insert(const string &mac_addr, int &port_id, int &pos)
{
    table[pos].mac_addr = mac_addr;
    table[pos].port_id = port_id;
    table[pos].time_stamp = sc_time_stamp();
    table[pos].valid = true;
}

void MacAddrTable::remove(int pos)
{
    table[pos].valid = false;
}

int MacAddrTable::find_oldest_entry()
{
    sc_time oldest_time = table[0].time_stamp;
    int oldest_pos = 0;

    for (int i = 1; i < table.size(); i++)
    {
        if (table[i].valid && table[i].time_stamp < oldest_time)
        {
            oldest_time = table[i].time_stamp;
            oldest_pos = i;
        }
    }

    return oldest_pos;
}

int MacAddrTable::lookup(const string &mac_addr)
{
    for (int i = 0; i < table.size(); i++)
    {
        if (table[i].valid && table[i].mac_addr == mac_addr)
        {
            return i;
        }
    }

    return -1;
}

EthSwitch::EthSwitch(sc_module_name name, const int &num_tx_port, const int &num_rx_port, const int &mac_addr_table_size, const int &fifo_size)
    : sc_module(name),
      tx_ports_initiators("tx_ports_initiators", num_tx_port),
      rx_ports_targets("rx_ports_targets", num_rx_port),
      tx_ports("tx_ports", num_tx_port),
      rx_ports("rx_ports", num_rx_port),
      forward_initiator("forward_initiator"),
      fetch_initiator("fetch_initiator"),
      process_fifo("process_fifo", fifo_size),
      mac_addr_table(mac_addr_table_size)
{
    SC_THREAD(process);
    SC_THREAD(schedule);

    for (int i = 0; i < tx_ports.size(); i++)
    {
        tx_ports_initiators[i](tx_ports[i].send_initiator);
    }

    for (int i = 0; i < tx_ports.size(); i++)
    {
        forward_initiator(tx_ports[i].receive_target);
    }

    for (int i = 0; i < rx_ports.size(); i++)
    {
        rx_ports_targets[i](rx_ports[i].receive_target);
    }

    for (int i = 0; i < rx_ports.size(); i++)
    {
        fetch_initiator(rx_ports[i].read_target);
    }
}

void EthSwitch::learn(const string &mac_addr, int &port_id)
{
    // 查找 MAC 地址是否已存在
    int pos = mac_addr_table.lookup(mac_addr);
    if (pos >= 0)
    {
        // 如果存在，更新条目
        mac_addr_table.insert(mac_addr, port_id, pos);
        return;
    }

    // 如果不存在，插入新条目
    if (mac_addr_table.insert(mac_addr, port_id) < 0)
    {
        int oldest_pos = mac_addr_table.find_oldest_entry();
        mac_addr_table.remove(oldest_pos);
        mac_addr_table.insert(mac_addr, port_id, oldest_pos);
    }
}

void EthSwitch::forward(int &port_id, Packet &packet)
{
    // 创建 TLM transaction
    tlm_generic_payload payload;
    sc_time delay = sc_time(0, SC_NS);
    tlm_phase phase = BEGIN_REQ;

    // 设置 transaction 参数
    payload.set_command(TLM_WRITE_COMMAND);
    payload.set_data_ptr(reinterpret_cast<unsigned char *>(&packet));
    payload.set_data_length(sizeof(Packet));
    payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

    // 发送数据包到tx_port
    tlm_sync_enum status = forward_initiator[port_id]->nb_transport_fw(payload, phase, delay);

    wait(delay);
}

void EthSwitch::schedule()
{
    int port_id = 0;
    while (true)
    {
        // 创建 TLM transaction
        tlm_generic_payload payload;
        sc_time delay = sc_time(0, SC_NS);
        tlm_phase phase = BEGIN_REQ;
        Packet packet;

        // 设置 transaction 参数
        payload.set_command(TLM_READ_COMMAND);
        payload.set_data_ptr(reinterpret_cast<unsigned char *>(&packet));
        payload.set_data_length(sizeof(Packet));
        payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

        // 从rx port取数据包
        tlm_sync_enum status = fetch_initiator[port_id]->nb_transport_fw(payload, phase, delay);

        ProcFifoEntry proc_fifo_entry = {port_id, packet};

        process_fifo.write(proc_fifo_entry);

        wait(delay);

        if (port_id < rx_ports.size())
        {
            port_id++;
        }
        else
        {
            port_id = 0;
        }
    }
}

void EthSwitch::process()
{
    while (true)
    {
        // 从 FIFO 中读取数据包（如果 FIFO 为空会自动阻塞等待）
        ProcFifoEntry proc_fifo_entry = process_fifo.read();

        int src_port_id = proc_fifo_entry.port_id;
        Packet packet = proc_fifo_entry.packet;

        string src_mac_addr = packet.eth_ii.mac_addr_to_string(packet.eth_ii.src_mac_addr);
        string dest_mac_addr = packet.eth_ii.mac_addr_to_string(packet.eth_ii.dest_mac_addr);

        learn(src_mac_addr, src_port_id);

        int dest_port_id = mac_addr_table.lookup(dest_mac_addr);

        if (dest_port_id > 0)
        {
            // 如果目的 MAC 地址在 MAC 地址表中，直接转发
            forward(dest_port_id, packet);
        }
        else
        {
            // 如果目的 MAC 地址不在 MAC 地址表中，广播到所有端口
            for (int i = 0; i < tx_ports.size(); i++)
            {
                if (i != src_port_id) // 不发送到源端口
                {
                    forward(i, packet);
                }
            }
        }

        wait(10, SC_NS);
    }
}

}

