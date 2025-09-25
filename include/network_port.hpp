#ifndef NETWORK_PORT_HPP
#define NETWORK_PORT_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "packet.hpp"
#include "stream_out.hpp"

namespace netsim
{

class NetworkPort : public sc_core::sc_module
{
public:
    sc_core::sc_fifo<std::vector<unsigned char>> packet_fifo;

    NetworkPort(sc_core::sc_module_name name, const int &fifo_size = 16);
};

class TxPort : public NetworkPort
{
public:
    tlm_utils::multi_passthrough_initiator_socket<TxPort> tx_initiator;

    TxPort(sc_core::sc_module_name name, const int &fifo_size = 16);

    SC_HAS_PROCESS(TxPort);

private:
    void send();
};

class RxPort : public NetworkPort
{
public:
    tlm_utils::multi_passthrough_target_socket<RxPort> rx_target;

    RxPort(sc_core::sc_module_name name, const int &fifo_size = 16);

private:
    tlm::tlm_sync_enum receive(int id, tlm::tlm_generic_payload &trans,
                               tlm::tlm_phase &phase,
                               sc_core::sc_time &delay);
};

}

#endif
