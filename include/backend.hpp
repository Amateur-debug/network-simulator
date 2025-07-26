#ifndef BACKEND_HPP
#define BACKEND_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "nic.hpp"
#include "packet.hpp"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 22222
#define CLIENT_ADDR "127.0.0.1"
#define CLIENT_PORT 12345

using namespace std;
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

namespace netsim
{

class EndPoint : public sc_module
{
public:
    sc_vector<simple_initiator_socket<EndPoint>> tx_ports_initiators;
    sc_vector<simple_target_socket<EndPoint>> rx_ports_targets;

    EndPoint(sc_module_name name);

private:
    NIC nic;

    simple_initiator_socket<EndPoint> nic_initiator;

    int sock_fd; // Socket file descriptor

    void start_udp_server();
};

class UDPServer : public sc_module
{
public:
    sc_vector<simple_initiator_socket<UDPServer>> tx_ports_initiators;
    sc_vector<simple_target_socket<UDPServer>> rx_ports_targets;

    UDPServer(sc_module_name name);

private:
    NIC nic;

    simple_initiator_socket<UDPServer> nic_write_initiator;
    simple_initiator_socket<UDPServer> nic_read_initiator;

    int sock_fd; // Socket file descriptor

    void start_udp_server();
};

class udp_client
{
public:
    // Constructor
    udp_client();

    void receive();

    void send(Packet &packet);

private:
    int sock_fd; // Socket file descriptor
};

class backend : public sc_module
{
public:
    // TLM target socket for receiving packets from tx_port
    simple_target_socket<backend> tx_target;

    // TLM initiator socket for sending packets to rx_port
    simple_initiator_socket<backend> rx_initiator;

    // Register SystemC thread
    SC_HAS_PROCESS(backend);

    // Constructor
    backend(sc_module_name name);

private:
    udp_server _udp_server;

    udp_client _udp_client;

    void receive_from_tx(tlm_generic_payload &payload, sc_time &delay);

    void send_to_rx();
};

}

#endif // BACKEND_HPP