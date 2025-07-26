#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

#include "backend.hpp"

namespace netsim
{

EndPoint::EndPoint(sc_module_name name)
    : sc_module(name),
      tx_ports_initiators("tx_ports_initiators", 1),
      rx_ports_targets("rx_ports_targets", 1),
      nic("nic", 1, 1, 200, 200),
      nic_initiator("nic_initiator")
{
    for (int i = 0; i < nic.tx_ports_initiators.size(); i++)
    {
        tx_ports_initiators[i](nic.tx_ports_initiators[i]);
    }

    for (int i = 0; i < nic.rx_ports_targets.size(); i++)
    {
        rx_ports_targets[i](nic.rx_ports_targets[i]);
    }

    nic_initiator(nic.nic_target);
}

/* udp_server */
void udp_server::start_udp_server()
{
    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server_addr.sin_port = htons(SERVER_PORT);

    // 绑定 socket
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        return;
    }
}

void udp_server::receive()
{
    char buffer[1500]; // 接收缓冲区
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 接收数据
    ssize_t recv_len = recvfrom(sock_fd, buffer, sizeof(buffer), 0,
                                (struct sockaddr *)&client_addr, &client_addr_len);

    cout << "udp_server received packet from "
         << inet_ntoa(client_addr.sin_addr) << ":"
         << ntohs(client_addr.sin_port) << endl;
}

void udp_server::send(Packet &packet)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = packet.udp_header.dest_port;
    client_addr.sin_addr.s_addr = packet.ipv4_header.dest_ip;

    // 发送数据
    ssize_t sent = sendto(sock_fd, packet.payload.data(), packet.payload.size(), 0,
                          (struct sockaddr *)&client_addr, sizeof(client_addr));

    cout << "udp_server send packet to "
         << inet_ntoa(client_addr.sin_addr) << ":"
         << ntohs(client_addr.sin_port) << endl;
}

udp_server::udp_server(sc_module_name name)
    : sc_module(name),
      rx_target("rx_target")
{
    rx_target.register_nb_transport()

        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    start_udp_server();
}

/* udp_client */
udp_client::udp_client()
{
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
}

void udp_client::send(Packet &packet)
{
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = packet.udp_header.dest_port;
    server_addr.sin_addr.s_addr = packet.ipv4_header.dest_ip;

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = packet.udp_header.src_port;
    server_addr.sin_addr.s_addr = packet.ipv4_header.src_ip;

    bind(sock_fd, (struct sockaddr *)&client_addr, sizeof(client_addr));

    // 发送数据
    ssize_t sent = sendto(sock_fd, packet.payload.data(), packet.payload.size(), 0,
                          (struct sockaddr *)&server_addr, sizeof(server_addr));

    cout << "udp_client send packet to "
         << inet_ntoa(server_addr.sin_addr) << ":"
         << ntohs(server_addr.sin_port) << endl;
}

void udp_client::receive()
{
    char buffer[1500]; // 接收缓冲区
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    // 接收数据
    ssize_t recv_len = recvfrom(sock_fd, buffer, sizeof(buffer), 0,
                                (struct sockaddr *)&server_addr, &server_addr_len);

    cout << "udp_client received packet from "
         << inet_ntoa(server_addr.sin_addr) << ":"
         << ntohs(server_addr.sin_port) << endl;
}

/* backend */
backend::backend(sc_module_name name)
    : sc_module(name),
      _udp_server(),
      _udp_client()
{
    SC_THREAD(send_to_rx);

    // 注册 b_transport 方法到 socket
    tx_target.register_b_transport(this, &backend::receive_from_tx);
}

void backend::receive_from_tx(tlm_generic_payload &payload, sc_time &delay)
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

    _udp_client.send(*packet_ptr);

    _udp_server.receive();

    delay += sc_time(10, SC_NS);

    payload.set_response_status(TLM_OK_RESPONSE);
}

void backend::send_to_rx()
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

        udp_header.src_port = htons(SERVER_PORT);
        udp_header.dest_port = htons(CLIENT_PORT);
        ipv4_header.src_ip = inet_addr(SERVER_ADDR);
        ipv4_header.dest_ip = inet_addr(CLIENT_ADDR);

        Packet packet(udp_header, ipv4_header, eth_ii, payload_data);

        // 设置 transaction 参数
        payload.set_command(TLM_WRITE_COMMAND);
        payload.set_address(0);
        payload.set_data_ptr(reinterpret_cast<unsigned char *>(&packet));
        payload.set_data_length(sizeof(Packet));
        payload.set_response_status(TLM_INCOMPLETE_RESPONSE);
        _udp_server.send();

        // 创建 TLM transaction
        tlm_generic_payload payload;
        sc_time delay = sc_time(0, SC_NS);

        // 设置 transaction 参数
        payload.set_command(TLM_WRITE_COMMAND);
        payload.set_address(0);
        payload.set_data_ptr(reinterpret_cast<unsigned char *>(&_udp_server));
        payload.set_data_length(sizeof(Packet));
        payload.set_response_status(TLM_INCOMPLETE_RESPONSE);

        // 发送数据包到 rx_port
        rx_initiator->b_transport(payload, delay);
    }
}

}
