#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "crossbar.hpp"

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

Crossbar::Crossbar(sc_module_name name, unsigned num_masters, unsigned num_slaves)
    : sc_module(name),
      s_targets("s_targets", num_masters),
      m_initiators("m_initiators", num_slaves),
      map(num_slaves)
{
    for (unsigned i = 0; i < s_targets.size(); i++)
    {
        s_targets[i].register_nb_transport_fw(this, &Crossbar::receive);
    }

    cout << "Crossbar address map:" << endl;
    for (unsigned i = 0; i < map.size(); i++)
    {
        map[i].base = 4ULL * 1024 * 1024 * 1024 * i;
        map[i].size = 4ULL * 1024 * 1024 * 1024;

        uint64_t size_gb = 4ULL;

        cout << i << ": base 0x" << hex << map[i].base << 
        "   size " << size_gb << "GB" << endl;
    }
}

void Crossbar::set_mapping(unsigned slave, uint64_t base, uint64_t size)
{
    if (slave >= map.size())
    {
        return;
    }

    map[slave].base = base;
    map[slave].size = size;
}

int Crossbar::decode(uint64_t addr, uint64_t &offset)
{
    for (unsigned i = 0; i < map.size(); i++)
    {
        if (map[i].size == 0)
        {
            continue;
        }
        else if (addr >= map[i].base && addr < map[i].base + map[i].size)
        {
            offset = addr - map[i].base;
            return static_cast<int>(i);
        }
    }
    return -1;
}

tlm_sync_enum Crossbar::receive(tlm_generic_payload &trans,
                                tlm_phase &phase,
                                sc_time &delay)
{

    if (phase != BEGIN_REQ)
    {
        trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
        return TLM_COMPLETED;
    }

    uint64_t addr = trans.get_address();
    uint64_t offset = 0;
    int sid = decode(addr, offset);
    if (sid < 0 || static_cast<unsigned>(sid) >= s_targets.size())
    {
        SC_REPORT_WARNING("Crossbar", "Address decode failed");
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        phase = END_RESP;
        return TLM_COMPLETED;
    }

    trans.set_address(offset);
    tlm_sync_enum status = m_initiators[static_cast<unsigned>(sid)]->nb_transport_fw(trans, phase, delay);

    trans.set_address(addr);

    return status;
}

}