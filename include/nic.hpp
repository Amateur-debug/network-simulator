#ifndef NIC_HPP
#define NIC_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

#include "dma.hpp"
#include "network_port.hpp"
#include "stream_out.hpp"

namespace netsim
{

class NIC : public sc_core::sc_module
{
public:

    tlm_utils::multi_passthrough_initiator_socket<NIC> data_initiator;
    tlm_utils::multi_passthrough_target_socket<NIC> desc_target;

    sc_core::sc_vector<tlm_utils::multi_passthrough_initiator_socket<NIC>> tx_initiators;
    sc_core::sc_vector<tlm_utils::multi_passthrough_target_socket<NIC>> rx_targets;

    NIC(sc_core::sc_module_name name, const int &num_tx_port = 1, const int &num_rx_port = 1,
        const int &desc_fifo_size = 200, const int &tx_queue_size = 200, const int &rx_queue_size = 200);

private:

    sc_core::sc_vector<TxPort> tx_ports;
    sc_core::sc_vector<RxPort> rx_ports;

    DMA dma;

    void send();
    void receive();
};

}

#endif