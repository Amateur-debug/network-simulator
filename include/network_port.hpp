#ifndef NETWORK_PORT_HPP
#define NETWORK_PORT_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "packet.hpp"

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

namespace netsim
{

class NetworkPort : public sc_module
{
public:
    simple_target_socket<NetworkPort> receive_target;

    // Register SystemC thread
    SC_HAS_PROCESS(NetworkPort);

    // Constructor
    NetworkPort(sc_module_name name, const int &fifo_size = 16);

protected:
    // FIFO for packet storage
    sc_fifo<Packet> packet_fifo;

private:
    tlm_sync_enum receive(tlm_generic_payload &payload,
                          tlm_phase &phase,
                          sc_time &delay);
};

class TxPort : public NetworkPort
{
public:
    simple_initiator_socket<TxPort> send_initiator;

    // Register SystemC thread
    SC_HAS_PROCESS(TxPort);

    // Constructor
    TxPort(sc_module_name name, const int &fifo_size = 16);

private:
    void send();
};

class RxPort : public NetworkPort
{
public:
    simple_target_socket<RxPort> read_target;

    // Constructor
    RxPort(sc_module_name name, const int &fifo_size = 16);

private:
    tlm_sync_enum read(tlm_generic_payload &payload, tlm_phase &phase, sc_time &delay);
};

}

#endif
