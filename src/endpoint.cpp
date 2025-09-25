#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include "endpoint.hpp"

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

EndPoint::EndPoint(sc_module_name name, const int &num_tx_port, const int &num_rx_port)
    : sc_module(name),
      tx_initiators("tx_initiators", num_tx_port),
      rx_targets("rx_targets", num_rx_port),
      cpu_data_initiator("cpu_data_initiator"),
      cpu_desc_initiator("cpu_desc_initiator"),
      nic("nic", num_tx_port, num_rx_port, 200, 200, 200),
      mem("mem", 16ULL * 1024 * 1024 * 1024),
      xbar("xbar", 2, 1)
{
    for (int i = 0; i < nic.tx_initiators.size(); i++)
    {
        nic.tx_initiators[i].bind(tx_initiators[i]);
    }
    for (int i = 0; i < nic.rx_targets.size(); i++)
    {
        rx_targets[i].bind(nic.rx_targets[i]);
    }

    cpu_data_initiator.bind(xbar.s_targets[0]);
    cpu_desc_initiator.bind(nic.desc_target);

    nic.data_initiator.bind(xbar.s_targets[1]);
    xbar.m_initiators[0](mem.mem_target);
}

UDPClient::UDPClient(sc_module_name name, const int &num_tx_port, const int &num_rx_port)
    : EndPoint(name, num_tx_port, num_rx_port) {}

void UDPClient::send(Packet pkt)
{
    tlm_generic_payload data_trans;
    tlm_phase data_phase = BEGIN_REQ;
    sc_time data_delay = sc_time(0, SC_NS);

    vector<unsigned char> serialized_pkt = pkt.serialize();

    data_trans.set_command(TLM_WRITE_COMMAND);
    data_trans.set_address(0);
    data_trans.set_data_ptr(serialized_pkt.data());
    data_trans.set_data_length(serialized_pkt.size());
    data_trans.set_streaming_width(serialized_pkt.size());
    data_trans.set_byte_enable_ptr(nullptr);
    data_trans.set_dmi_allowed(false);

    cpu_data_initiator->nb_transport_fw(data_trans, data_phase, data_delay);

    if (data_trans.get_response_status() != TLM_OK_RESPONSE)
    {
        SC_REPORT_WARNING("UDPClient", "send failed");
    }

    wait(data_delay);

    Descriptor desc;
    desc.type = DescriptorType::mem2device;
    desc.src_addr = 0;
    desc.length = sizeof(pkt);

    tlm_generic_payload desc_trans;
    tlm_phase desc_phase = BEGIN_REQ;
    sc_time desc_delay = sc_time(0, SC_NS);

    desc_trans.set_command(TLM_WRITE_COMMAND);
    desc_trans.set_address(0);
    desc_trans.set_data_ptr(reinterpret_cast<unsigned char *>(&desc));
    desc_trans.set_data_length(static_cast<unsigned int>(sizeof(desc)));
    desc_trans.set_streaming_width(static_cast<unsigned int>(sizeof(desc)));
    desc_trans.set_byte_enable_ptr(nullptr);
    desc_trans.set_dmi_allowed(false);

    cpu_desc_initiator->nb_transport_fw(desc_trans, desc_phase, desc_delay);

    wait(desc_delay);
}

UDPServer::UDPServer(sc_module_name name, const int &num_tx_port, const int &num_rx_port)
    : EndPoint(name, num_tx_port, num_rx_port) {}

Packet UDPServer::receive()
{
    Descriptor desc;
    desc.type = DescriptorType::device2mem;
    desc.dest_addr = 0x80000000;
    desc.length = 1500;

    tlm_generic_payload desc_trans;
    tlm_phase desc_phase = BEGIN_REQ;
    sc_time desc_delay = sc_time(0, SC_NS);

    desc_trans.set_command(TLM_WRITE_COMMAND);
    desc_trans.set_address(0);
    desc_trans.set_data_ptr(reinterpret_cast<unsigned char *>(&desc));
    desc_trans.set_data_length(static_cast<unsigned int>(sizeof(desc)));
    desc_trans.set_streaming_width(static_cast<unsigned int>(sizeof(desc)));
    desc_trans.set_byte_enable_ptr(nullptr);
    desc_trans.set_dmi_allowed(false);

    cpu_desc_initiator->nb_transport_fw(desc_trans, desc_phase, desc_delay);

    wait(desc_delay);

    tlm_generic_payload mem_trans;
    tlm_phase mem_phase = BEGIN_REQ;
    sc_time mem_delay = sc_time(0, SC_NS);

    vector<unsigned char> buffer;
    buffer.resize(desc.length);

    mem_trans.set_command(TLM_READ_COMMAND);
    mem_trans.set_address(0x80000000);
    mem_trans.set_data_ptr(buffer.data());
    mem_trans.set_data_length(desc.length);
    mem_trans.set_streaming_width(desc.length);
    mem_trans.set_byte_enable_ptr(nullptr);
    mem_trans.set_dmi_allowed(false);

    cpu_data_initiator->nb_transport_fw(mem_trans, mem_phase, mem_delay);

    Packet pkt;
    pkt.deserialize(buffer);

    string result;
    for (const auto &bv : pkt.payload)
    {
        result += static_cast<char>(bv.to_uint());
    }

    std::stringstream ss;
    ss << "Received packet: " << result;
    SC_REPORT_INFO("EndPoint", ss.str().c_str());

    return pkt;
}

}
