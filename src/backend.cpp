#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "backend.hpp"

void udp_server::start_udp_server()
{
    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(22222);

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

    cout << "Received packet from "
         << inet_ntoa(client_addr.sin_addr) << ":"
         << ntohs(client_addr.sin_port) << endl;
}

udp_server::udp_server()
{
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    start_udp_server();
}

udp_client::udp_client()
{
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
}

void udp_client::send(Packet &packet)
{
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = packet.udp_header.dest_port;
    server_address.sin_addr.s_addr = packet.ipv4_header.dest_ip;

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = packet.udp_header.src_port; 
    server_address.sin_addr.s_addr = packet.ipv4_header.src_ip;

    bind(sock_fd, (struct sockaddr*)&client_addr, sizeof(client_addr));

    // 发送数据包
    ssize_t sent = sendto(sock_fd, packet.payload.data(), packet.payload.size(), 0,
                          (struct sockaddr *)&server_address, sizeof(server_address));
}

backend::backend(sc_module_name name)
    : sc_module(name),
      _udp_server(),
      _udp_client()
{
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