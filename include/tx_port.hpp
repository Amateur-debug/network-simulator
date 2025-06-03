#ifndef TX_PORT_HPP
#define TX_PORT_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"

#include "packet.hpp"

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

class tx_port : public sc_module
{
public:
    // TLM target socket for receiving packets
    simple_target_socket<tx_port> tx_port_in;

    // TLM initiator socket for sending packets
    simple_initiator_socket<tx_port> tx_port_out;

    // Register SystemC thread
    SC_HAS_PROCESS(tx_port);

    // Constructor
    tx_port(sc_module_name name);

private:

    sc_fifo<Packet> tx_port_fifo; // FIFO for packet storage

    void receive(tlm_generic_payload &payload, sc_time &delay);

    void send();

};

#endif // TX_PORT_HPP