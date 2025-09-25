#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "stream_out.hpp"

namespace netsim
{

class Crossbar : public sc_core::sc_module
{
public:
    sc_core::sc_vector<tlm_utils::simple_target_socket<Crossbar>> s_targets;

    sc_core::sc_vector<tlm_utils::simple_initiator_socket<Crossbar>> m_initiators;

    SC_HAS_PROCESS(Crossbar);

    Crossbar(sc_core::sc_module_name name, unsigned num_masters = 2, unsigned num_slaves = 2);

    void set_mapping(unsigned slave, std::uint64_t base, std::uint64_t size);

private:
    struct Map
    {
        std::uint64_t base{0};
        std::uint64_t size{0};
    };

    std::vector<Map> map;

    int decode(std::uint64_t addr, std::uint64_t &offset);

    tlm::tlm_sync_enum receive(tlm::tlm_generic_payload &trans,
                               tlm::tlm_phase &phase,
                               sc_core::sc_time &delay);

};

}
