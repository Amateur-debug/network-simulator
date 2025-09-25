#include <arpa/inet.h>
#include <sys/socket.h>

#include "packet.hpp"
#include "network_port.hpp"

#define CLIENT_PORT 22222
#define CLIENT_IP_ADDR "192.168.0.1"
#define CLIENT_MAC_ADDR 0x123456789ABC
#define SERVER_PORT 22222
#define SERVER_IP_ADDR "192.168.0.2"
#define SERVER_MAC_ADDR 0xCBA987654321
#define PAYLOAD "Hello, World!"

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

using namespace netsim;

int sc_main(int argc, char *argv[])
{
    TxPort tx_port("tx_port");
    RxPort rx_port("rx_port");

    tx_port.tx_initiator.bind(rx_port.rx_target);

    UDPHeader udp_header;
    IPv4Header ipv4_header;
    EthernetII eth_ii;
    vector<sc_uint<8>> payload;

    string payload_str = PAYLOAD;
    for (char c : payload_str) {
        payload.push_back(sc_uint<8>(static_cast<unsigned char>(c)));
    }

    struct in_addr src_ip{};
    struct in_addr dest_ip{};
    inet_pton(AF_INET, CLIENT_IP_ADDR, &src_ip);
    inet_pton(AF_INET, SERVER_IP_ADDR, &dest_ip);

    udp_header.src_port = CLIENT_PORT;
    udp_header.dest_port = SERVER_PORT;
    ipv4_header.src_ip = ntohl(src_ip.s_addr);
    ipv4_header.dest_ip = ntohl(dest_ip.s_addr);
    eth_ii.src_mac_addr = CLIENT_MAC_ADDR;
    eth_ii.dest_mac_addr = SERVER_MAC_ADDR;

    Packet send_pkt(udp_header, ipv4_header, eth_ii, payload);
    vector<unsigned char> send_pkt_serialized = send_pkt.serialize();

    tx_port.packet_fifo.write(send_pkt_serialized);

    sc_start(50, SC_NS);

    vector<unsigned char> receive_pkt_serialized = rx_port.packet_fifo.read();
    Packet receive_pkt;
    receive_pkt.deserialize(receive_pkt_serialized);

    string result;
    for (const auto &bv : receive_pkt.payload)
    {
        result += static_cast<char>(bv.to_uint());
    }

    std::stringstream ss;
    ss << "Received packet: " << result;
    SC_REPORT_INFO("EndPoint", ss.str().c_str());

    return 0;
}
