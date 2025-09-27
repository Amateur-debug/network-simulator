#ifndef SEND_TEST_HPP
#define SEND_TEST_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "endpoint.hpp"

namespace netsim
{

#define CLIENT_PORT 22222
#define CLIENT_IP_ADDR "192.168.0.1"
#define CLIENT_MAC_ADDR 0x123456789ABC
#define SERVER_PORT 22222
#define SERVER_IP_ADDR "192.168.0.2"
#define SERVER_MAC_ADDR 0xCBA987654321
#define PAYLOAD "Hello, World!"

class Client : public sc_core::sc_module
{
public:
    tlm_utils::multi_passthrough_initiator_socket<Client> send_initiator;
    tlm_utils::multi_passthrough_target_socket<Client> receive_target;

    Client(sc_core::sc_module_name name);

private:
    UDPClient udp_client;
    void send();
};

class Server : public sc_core::sc_module
{
public:
    tlm_utils::multi_passthrough_initiator_socket<Server> send_initiator;
    tlm_utils::multi_passthrough_target_socket<Server> receive_target;

    Server(sc_core::sc_module_name name);

private:
    UDPServer udp_server;
    void receive();
};

}

#endif
