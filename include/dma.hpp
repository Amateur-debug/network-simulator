#ifndef DMA_HPP
#define DMA_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

#include "packet.hpp"
#include "stream_out.hpp"

namespace netsim
{

enum class DescriptorType : std::uint64_t
{
    invalid = 0,
    mem2device = 1,
    device2mem = 2,
    mem2mem = 3,
};

struct Descriptor
{
    DescriptorType type{DescriptorType::invalid};
    uint64_t src_addr{0};
    uint64_t dest_addr{0};
    unsigned int length{0};

    friend std::ostream &operator<<(std::ostream &os, const Descriptor &desc);
};

class DMA : public sc_core::sc_module
{
public:

    tlm_utils::multi_passthrough_initiator_socket<DMA> data_initiator;
    tlm_utils::multi_passthrough_target_socket<DMA> desc_target;

    sc_core::sc_fifo<std::vector<unsigned char>> read_packet_fifo;
    sc_core::sc_fifo<std::vector<unsigned char>> write_packet_fifo;

    DMA(sc_core::sc_module_name name, const int &desc_fifo_size = 64, const int &read_fifo_size = 64, const int &write_fifo_size = 64);

private:
    sc_core::sc_fifo<Descriptor> desc_fifo;

    tlm::tlm_sync_enum nb_transport_desc(int id, tlm::tlm_generic_payload &trans,
                                         tlm::tlm_phase &phase,
                                         sc_core::sc_time &delay);

    void process();
};

}

#endif