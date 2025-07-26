#ifndef NIC_HPP
#define NIC_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "network_port.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

namespace netsim
{

class NIC : public sc_module
{
public:
    simple_target_socket<NIC> nic_target;

    sc_vector<simple_initiator_socket<NIC>> tx_ports_initiators;
    sc_vector<simple_target_socket<NIC>> rx_ports_targets;

    // Register SystemC thread
    SC_HAS_PROCESS(NIC);

    // Constructor
    NIC(sc_module_name name, const int &num_tx_port = 1, const int &num_rx_port = 1,
        const int &tx_queue_size = 200, const int &rx_queue_size = 200);

private:
    tlm_sync_enum nic_slave(tlm_generic_payload &payload, tlm_phase &phase, sc_time &delay);
    
    sc_vector<TxPort> tx_ports;
    sc_vector<RxPort> rx_ports;

    multi_passthrough_initiator_socket<NIC> send_initiator;
    multi_passthrough_initiator_socket<NIC> fetch_initiator;

    sc_fifo<Packet> tx_queue;
    sc_fifo<Packet> rx_queue;

    void tx_schedule();
    void rx_schedule();
};

}

#endif