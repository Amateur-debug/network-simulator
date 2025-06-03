#ifndef PACKET_GENERATOR_HPP
#define PACKET_GENERATOR_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

class packet_generator : public sc_module
{
public:
    // TLM initiator socket for sending packets
    simple_initiator_socket<packet_generator> packet_initiator;

    // Register SystemC thread
    SC_HAS_PROCESS(packet_generator);

    // Constructor
    packet_generator(sc_module_name name);

private:
    void send();
};

#endif // PACKET_GENERATOR_HPP
