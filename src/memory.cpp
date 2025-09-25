#include "memory.hpp"

namespace netsim
{
using namespace std;

using namespace sc_core;
using namespace tlm;

Memory::Memory(sc_module_name name, std::size_t size)
    : sc_module(name),
      mem_target("mem_target"),
      mem(size)
{
    mem_target.register_nb_transport_fw(this, &Memory::nb_transport_mem);
}

tlm_sync_enum Memory::nb_transport_mem(tlm_generic_payload &trans,
                                       tlm_phase &phase,
                                       sc_time &delay)
{
    if (phase != BEGIN_REQ)
    {
        trans.set_response_status(TLM_GENERIC_ERROR_RESPONSE);
        return TLM_COMPLETED;
    }

    uint64_t addr = trans.get_address();
    unsigned char *data_ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();
    unsigned int sw = trans.get_streaming_width();

    if (addr + len > mem.size())
    {
        SC_REPORT_WARNING("Memory", "Access out of bounds");
        trans.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
        return TLM_COMPLETED;
    }

    if (trans.is_read())
    {
        memcpy(data_ptr, &mem[addr], len);
    }
    else if (trans.is_write())
    {
        memcpy(&mem[addr], data_ptr, len);
    }
    else
    {
        trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        return TLM_COMPLETED;
    }

    trans.set_response_status(TLM_OK_RESPONSE);
    phase = END_RESP;
    return TLM_COMPLETED;

    unsigned int segment = (sw == 0 || sw > len) ? len : sw;
    unsigned int num_segments = (len + segment - 1) / segment;
    delay += num_segments * sc_time(1, SC_NS);
}

}
