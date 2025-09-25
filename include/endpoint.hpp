#ifndef ENDPOINT_HPP
#define ENDPOINT_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

#include "dma.hpp"
#include "memory.hpp"
#include "nic.hpp"
#include "crossbar.hpp"
#include "packet.hpp"
#include "stream_out.hpp"

namespace netsim
{

class EndPoint : public sc_core::sc_module
{
public:
    sc_core::sc_vector<tlm_utils::multi_passthrough_initiator_socket<EndPoint>> tx_initiators;
    sc_core::sc_vector<tlm_utils::multi_passthrough_target_socket<EndPoint>> rx_targets;

    EndPoint(sc_core::sc_module_name name, const int &num_tx_port = 1, const int &num_rx_port = 1);

protected:
    tlm_utils::simple_initiator_socket<EndPoint> cpu_data_initiator;
    tlm_utils::simple_initiator_socket<EndPoint> cpu_desc_initiator;

private:
    NIC nic;
    Memory mem;
    Crossbar xbar;
};

class UDPClient : public EndPoint
{
public:
    // Constructor
    UDPClient(sc_core::sc_module_name name, const int &num_tx_port = 1, const int &num_rx_port = 1);

    void send(Packet pkt);
};

class UDPServer : public EndPoint
{
public:
    UDPServer(sc_core::sc_module_name name, const int &num_tx_port = 1, const int &num_rx_port = 1);

    Packet receive();
};




}

#endif // BACKEND_HPP