#include "packet.hpp"

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

ostream& operator<<(ostream& os, const Packet& packet) {
    os << "Packet[payload_size=" << packet.payload.size() 
       << ", protocol=" << static_cast<int>(packet.ipv4_header.protocol) << "]";
    return os;
}