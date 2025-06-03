#include "tx_port.hpp"

tx_port::tx_port(sc_module_name name)
    : sc_module(name),
      tx_port_fifo("tx_port_fifo", 32)
{
    // 注册 b_transport 方法到 socket
    tx_port_in.register_b_transport(this, &tx_port::receive);

    // 注册 SystemC 线程
    SC_THREAD(send);
}

void tx_port::receive(tlm_generic_payload &payload, sc_time &delay)
{
    // 检查命令类型
    if (!payload.is_write())
    {
        payload.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        return;
    }

    // 获取数据
    unsigned char *data_ptr = payload.get_data_ptr();
    unsigned int data_length = payload.get_data_length();

    if (data_ptr == nullptr)
    {
        payload.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        return;
    }

    Packet *packet_ptr = reinterpret_cast<Packet *>(data_ptr);

    // 非阻塞写入 FIFO
    tx_port_fifo.write(*packet_ptr);
    delay += sc_time(1, SC_NS);

    payload.set_response_status(TLM_OK_RESPONSE);
}

void tx_port::send()
{
    while (true)
    {
        // 从 FIFO 中读取数据包（如果 FIFO 为空会自动阻塞等待）
        Packet packet = tx_port_fifo.read();

        // 创建 TLM transaction
        tlm_generic_payload payload;
        sc_time delay = sc_time(0, SC_NS);

        // 设置 transaction 参数
        payload.set_command(TLM_WRITE_COMMAND);
        payload.set_address(0);
        payload.set_data_ptr(reinterpret_cast<unsigned char *>(&packet));
        payload.set_data_length(sizeof(Packet));
        payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

        // 发送数据包
        tx_port_out->b_transport(payload, delay);

        // 检查响应
        if (payload.get_response_status() != TLM_OK_RESPONSE)
        {
            SC_REPORT_ERROR("TX_PORT", "Transaction failed");
        }

        // 等待延迟时间
        wait(delay);

        // 等待下次发送
        wait(100, SC_NS);
    }
}