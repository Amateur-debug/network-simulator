#include "network_port.hpp"

namespace netsim
{

NetworkPort::NetworkPort(sc_module_name name, const int &fifo_size)
    : sc_module(name),
      receive_target("receive_target"),
      packet_fifo("packet_fifo", fifo_size)
{
    // 注册 nb_transport 方法 receive 到 target_socket
    receive_target.register_nb_transport_fw(this, &NetworkPort::receive);
}

tlm_sync_enum NetworkPort::receive(tlm_generic_payload &payload, tlm_phase &phase, sc_time &delay)
{
    // 获取数据
    unsigned char *data_ptr = payload.get_data_ptr();
    unsigned int data_length = payload.get_data_length();
    Packet *packet_ptr = reinterpret_cast<Packet *>(data_ptr);

    // 阻塞写入 FIFO
    packet_fifo.write(*packet_ptr);

    // early response
    payload.set_response_status(TLM_OK_RESPONSE);
    phase = END_RESP;
    delay += sc_time(1, SC_NS);
    return TLM_COMPLETED;
}

TxPort::TxPort(sc_module_name name, const int &fifo_size)
    : NetworkPort(name, fifo_size),
      send_initiator("send_initiator")
{
    SC_THREAD(send);
}

void TxPort::send()
{
    while (true)
    {
        Packet packet = packet_fifo.read();

        // 创建 TLM transaction
        tlm_generic_payload payload;
        sc_time delay = sc_time(0, SC_NS);
        tlm_phase phase = BEGIN_REQ;

        // 设置 transaction 参数
        payload.set_command(TLM_WRITE_COMMAND);
        payload.set_data_ptr(reinterpret_cast<unsigned char *>(&packet));
        payload.set_data_length(sizeof(Packet));
        payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

        // 发送数据包
        tlm_sync_enum status = send_initiator->nb_transport_fw(payload, phase, delay);

        // 模拟传输延迟
        delay += sc_time(10, SC_NS);
        wait(delay);
    }
}

RxPort::RxPort(sc_module_name name, const int &fifo_size)
    : NetworkPort(name, fifo_size),
      read_target("read_target")
{
    read_target.register_nb_transport_fw(this, &RxPort::read);
}

tlm_sync_enum RxPort::read(tlm_generic_payload &payload, tlm_phase &phase, sc_time &delay)
{

    // 获取数据
    unsigned char *data_ptr = payload.get_data_ptr();
    Packet *packet_ptr = reinterpret_cast<Packet *>(data_ptr);
    *packet_ptr = packet_fifo.read();
    
    payload.set_response_status(TLM_OK_RESPONSE);
    phase = END_RESP;
    
    delay += sc_time(1, SC_NS);
    return TLM_COMPLETED;
}

}
