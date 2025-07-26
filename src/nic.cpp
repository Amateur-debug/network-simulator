#include "nic.hpp"

namespace netsim
{

NIC::NIC(sc_module_name name, const int &num_tx_port, const int &num_rx_port,
         const int &tx_queue_size, const int &rx_queue_size)
    : sc_module(name),
      nic_target("nic_target"),
      tx_ports("tx_ports", num_tx_port),
      rx_ports("rx_ports", num_rx_port),
      send_initiator("send_initiator"),
      fetch_initiator("fetch_initiator"),
      tx_queue("tx_queue", tx_queue_size),
      rx_queue("rx_queue", rx_queue_size)
{
    SC_THREAD(tx_schedule);
    SC_THREAD(rx_schedule);

    nic_target.register_nb_transport_fw(this, &NIC::nic_slave);

    for (int i = 0; i < tx_ports.size(); i++)
    {
        send_initiator(tx_ports[i].receive_target);
    }

    for (int i = 0; i < rx_ports.size(); i++)
    {
        fetch_initiator(rx_ports[i].read_target);
    }
}

tlm_sync_enum NIC::nic_slave(tlm_generic_payload &payload, tlm_phase &phase, sc_time &delay)
{
    // 获取数据
    unsigned char *data_ptr = payload.get_data_ptr();
    unsigned int data_length = payload.get_data_length();
    Packet *packet_ptr = reinterpret_cast<Packet *>(data_ptr);

    if (payload.get_command() == TLM_READ_COMMAND)
    {
        // 读取数据
        *packet_ptr = rx_queue.read();
    }
    else if (payload.get_command() == TLM_WRITE_COMMAND)
    {
        // 写入数据
        tx_queue.write(*packet_ptr);
    }

    // early response
    payload.set_response_status(TLM_OK_RESPONSE);
    phase = END_RESP;
    delay += sc_time(1, SC_NS);
    return TLM_COMPLETED;
}

void NIC::tx_schedule()
{
    int port_id = 0;
    while (true)
    {
        // 创建 TLM transaction
        tlm_generic_payload payload;
        sc_time delay = sc_time(0, SC_NS);
        tlm_phase phase = BEGIN_REQ;
        Packet packet = tx_queue.read();

        // 设置 transaction 参数
        payload.set_command(TLM_READ_COMMAND);
        payload.set_data_ptr(reinterpret_cast<unsigned char *>(&packet));
        payload.set_data_length(sizeof(Packet));
        payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

        // 向tx port发送数据包
        tlm_sync_enum status = send_initiator[port_id]->nb_transport_fw(payload, phase, delay);

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

void NIC::rx_schedule()
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

        // 从rx port获取数据包
        tlm_sync_enum status = fetch_initiator[port_id]->nb_transport_fw(payload, phase, delay);

        rx_queue.write(packet);

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

}
