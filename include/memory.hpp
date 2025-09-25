#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_target_socket.h"

namespace netsim
{

class Memory : public sc_core::sc_module
{
public:
    tlm_utils::simple_target_socket<Memory> mem_target;

    Memory(sc_core::sc_module_name name, std::size_t size);

private:
    std::vector<uint8_t> mem;

    tlm::tlm_sync_enum nb_transport_mem(tlm::tlm_generic_payload &trans,
                                        tlm::tlm_phase &phase,
                                        sc_core::sc_time &delay);
};

}

#endif // MEMORY_HPP