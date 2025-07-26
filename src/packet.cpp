#include "packet.hpp"

namespace netsim
{

Packet::Packet(TCPHeader tcp_header, IPv4Header ipv4_header, EthernetII eth_ii, vector<uint8_t> payload)
    : tcp_header(tcp_header),
      ipv4_header(ipv4_header),
      eth_ii(eth_ii),
      payload(payload)
{
}

Packet::Packet(UDPHeader udp_header, IPv4Header ipv4_header, EthernetII eth_ii, vector<uint8_t> payload)
    : udp_header(udp_header),
      ipv4_header(ipv4_header),
      eth_ii(eth_ii),
      payload(payload)
{
}

ostream &operator<<(ostream &os, const Packet &packet)
{
    os << "Packet[payload_size=" << packet.payload.size()
       << ", protocol=" << static_cast<int>(packet.ipv4_header.protocol) << "]";
    return os;
}

string EthernetII::mac_addr_to_string(uint64_t mac_addr)
{
    string mac_addr_string;

    for (int i = 0; i < 6; i++)
    {
        uint8_t byte = (mac_addr >> (8 * (5 - i)));
        mac_addr_string += to_string(byte);
    }
    return mac_addr_string;
}

}
