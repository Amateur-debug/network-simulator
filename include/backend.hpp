#ifndef BACKEND_HPP
#define BACKEND_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"

#include "packet.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

class udp_server
{
public:
    // Constructor
    udp_server();

    void receive();

private:

    int sock_fd; // Socket file descriptor

    void start_udp_server();
};

class udp_client
{
public:
    // Constructor
    udp_client();

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
    // simple_initiator_socket<backend> rx_initiator;

    // Register SystemC thread
    SC_HAS_PROCESS(backend);

    // Constructor
    backend(sc_module_name name);

private:
    udp_server _udp_server;

    udp_client _udp_client;

    void receive_from_tx(tlm_generic_payload &payload, sc_time &delay);

    void send_to_rx(tlm_generic_payload &payload, sc_time &delay);
};

#endif // BACKEND_HPP