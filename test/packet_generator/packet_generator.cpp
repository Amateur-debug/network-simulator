#include <sys/socket.h>
#include <arpa/inet.h>

#include "packet_generator.hpp"
#include "tx_port.hpp"
#include "backend.hpp"
#include "packet.hpp"

packet_generator::packet_generator(sc_module_name name)
    : sc_module(name)
{
    // 注册 SystemC 线程
    SC_THREAD(send);
}

void packet_generator::send()
{
    while (true)
    {
        // 创建 TLM transaction
        tlm_generic_payload payload;
        sc_time delay = sc_time(0, SC_NS);

        // 准备数据包
        UDPHeader udp_header;
        IPv4Header ipv4_header;
        EthernetII eth_ii;
        vector<uint8_t> payload_data(64, 0xAB);

        udp_header.src_port = htons(12345);
        udp_header.dest_port = htons(22222);
        ipv4_header.src_ip = inet_addr("127.0.0.1");
        ipv4_header.dest_ip = inet_addr("127.0.0.1");

        Packet packet(udp_header, ipv4_header, eth_ii, payload_data);

        // 设置 transaction 参数
        payload.set_command(TLM_WRITE_COMMAND);
        payload.set_address(0);
        payload.set_data_ptr(reinterpret_cast<unsigned char *>(&packet));
        payload.set_data_length(sizeof(Packet));
        payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

        // 通过 initiator socket 发送数据
        packet_initiator->b_transport(payload, delay);

        // 检查响应
        if (payload.get_response_status() != TLM_OK_RESPONSE)
        {
            SC_REPORT_ERROR("PACKET_GEN", "Transaction failed");
        }

        // 等待延迟时间
        wait(delay);

        // 等待下次发送
        wait(100, SC_NS);
    }
}

int sc_main(int argc, char *argv[]) {
    packet_generator packet_generator("packet_generator");
    tx_port tx_port("tx_port");
    backend backend("backend");

    packet_generator.packet_initiator(tx_port.tx_port_in);
    tx_port.tx_port_out(backend.tx_target);

    sc_start(1000, SC_NS);
    return 0;
}

