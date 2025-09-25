#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "dma.hpp"
#include "packet.hpp"

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

ostream &operator<<(std::ostream &os, const Descriptor &desc)
{
    os << "Descriptor[type=" << (desc.type == DescriptorType::mem2device ? "mem2device" : "device2mem")
       << ", src_addr=0x" << hex << desc.src_addr
       << ", dest_addr=0x" << hex << desc.dest_addr
       << ", length=" << dec << desc.length << "]";
    return os;
}

DMA::DMA(sc_module_name name, const int &desc_fifo_size, const int &read_fifo_size, const int &write_fifo_size)
    : sc_module(name),
      data_initiator("data_initiator"),
      desc_target("desc_target"),
      desc_fifo(desc_fifo_size),
      read_packet_fifo(read_fifo_size),
      write_packet_fifo(write_fifo_size)
{
    SC_THREAD(process);

    desc_target.register_nb_transport_fw(this, &DMA::nb_transport_desc);
}

tlm_sync_enum DMA::nb_transport_desc(int id, tlm_generic_payload &trans, tlm_phase &phase, sc_core::sc_time &delay)
{
    if (trans.get_command() != TLM_WRITE_COMMAND)
    {
        SC_REPORT_WARNING("DMA", "Descriptor command is not WRITE");
        trans.set_response_status(TLM_COMMAND_ERROR_RESPONSE);
        return TLM_COMPLETED;
    }

    if (trans.get_data_length() != sizeof(Descriptor))
    {
        SC_REPORT_WARNING("DMA", "Descriptor length is incorrect");
        trans.set_response_status(TLM_BURST_ERROR_RESPONSE);
        return TLM_COMPLETED;
    }

    unsigned char *data_ptr = trans.get_data_ptr();
    unsigned int data_length = trans.get_data_length();
    Descriptor *desc_ptr = reinterpret_cast<Descriptor *>(data_ptr);

    desc_fifo.write(*desc_ptr);

    trans.set_response_status(TLM_OK_RESPONSE);
    phase = END_RESP;
    delay += sc_time(1, SC_NS);
    return TLM_COMPLETED;
}

void DMA::process()
{
    while (true)
    {
        SC_REPORT_INFO("DMA", "Starting DMA process...");

        Descriptor desc = desc_fifo.read();

        sc_time delay = SC_ZERO_TIME;

        switch (desc.type)
        {
        case DescriptorType::mem2device:
        {
            vector<unsigned char> buf;
            buf.resize(desc.length);

            tlm_generic_payload trans;
            trans.set_command(TLM_READ_COMMAND);
            trans.set_address(desc.src_addr);
            trans.set_data_ptr(buf.data());
            trans.set_data_length(desc.length);
            trans.set_streaming_width(desc.length);
            trans.set_byte_enable_ptr(nullptr);
            trans.set_dmi_allowed(false);
            tlm_phase phase = BEGIN_REQ;

            data_initiator->nb_transport_fw(trans, phase, delay);

            if (trans.get_response_status() != TLM_OK_RESPONSE)
            {
                SC_REPORT_WARNING("DMA", "mem2device: read failed");
                break;
            }
            read_packet_fifo.write(move(buf));
            break;
        }
        case DescriptorType::device2mem:
        {
            vector<unsigned char> buf = write_packet_fifo.read();

            if(buf.size() > desc.length)
            {
                SC_REPORT_WARNING("DMA", "device2mem: packet size exceeds descriptor length");
                break;
            }

            tlm_generic_payload trans;
            trans.set_command(TLM_WRITE_COMMAND);
            trans.set_address(desc.dest_addr);
            trans.set_data_ptr(buf.data());
            trans.set_data_length(buf.size());
            trans.set_streaming_width(buf.size());
            trans.set_byte_enable_ptr(nullptr);
            trans.set_dmi_allowed(false);
            tlm_phase phase = BEGIN_REQ;

            data_initiator->nb_transport_fw(trans, phase, delay);

            if (trans.get_response_status() != TLM_OK_RESPONSE)
            {
                SC_REPORT_WARNING("DMA", "device2mem: write failed");
            }
            break;
        }

            wait(delay);
        }
    }
}
}