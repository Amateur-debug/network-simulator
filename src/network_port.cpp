#include "network_port.hpp"
#include <cstring>

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

NetworkPort::NetworkPort(sc_module_name name, const int &fifo_size)
    : sc_module(name),
      packet_fifo("packet_fifo", fifo_size) {}

TxPort::TxPort(sc_module_name name, const int &fifo_size)
    : NetworkPort(name, fifo_size),
      tx_initiator("tx_initiator")
{
    SC_THREAD(send);
}

void TxPort::send()
{
    while (true)
    {
        SC_REPORT_INFO("TxPort", "Starting send process...");

        vector<unsigned char> pkt = packet_fifo.read();

        tlm_generic_payload trans;
        tlm_phase phase = BEGIN_REQ;
        sc_time delay = sc_time(0, SC_NS);

        trans.set_command(TLM_READ_COMMAND);
        trans.set_data_ptr(pkt.data());
        trans.set_data_length(pkt.size());
        trans.set_streaming_width(pkt.size());
        trans.set_byte_enable_ptr(nullptr);
        trans.set_dmi_allowed(false);

        tx_initiator->nb_transport_fw(trans, phase, delay);

        if (trans.get_response_status() != TLM_OK_RESPONSE)
        {
            SC_REPORT_WARNING("TxPort", "send failed");
        }

        wait(delay);
    }
}

RxPort::RxPort(sc_module_name name, const int &fifo_size)
    : NetworkPort(name, fifo_size),
      rx_target("rx_target")
{
    rx_target.register_nb_transport_fw(this, &RxPort::receive);
}

tlm_sync_enum RxPort::receive(int id, tlm_generic_payload &trans, tlm_phase &phase, sc_time &delay)
{
    unsigned char *pkt_ptr = trans.get_data_ptr();
    unsigned int pkt_len = trans.get_data_length();
    unsigned int sw = trans.get_streaming_width();

    if (pkt_len == 0)
    {
        trans.set_response_status(TLM_OK_RESPONSE);
        return TLM_COMPLETED;
    }

    std::vector<unsigned char> pkt;
    pkt.resize(pkt_len);
    std::memcpy(pkt.data(), pkt_ptr, pkt_len);

    packet_fifo.write(std::move(pkt));

    trans.set_response_status(TLM_OK_RESPONSE);
    phase = END_RESP;

    unsigned int seg = (sw == 0 || sw > pkt_len) ? pkt_len : sw;
    unsigned int num_segs = (pkt_len + seg - 1) / seg;
    delay += num_segs * sc_time(1, SC_NS);

    return TLM_COMPLETED;
}

}