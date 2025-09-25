#include <arpa/inet.h>
#include <sys/socket.h>

#include "packet.hpp"
#include "network_port.hpp"
#include "eth_switch.hpp"

#define PORT0_PORT 22222
#define PORT0_IP_ADDR "192.168.0.1"
#define PORT0_MAC_ADDR 0x123456789ABC
#define PORT0_PAYLOAD "Hello, World from Port0!"
#define PORT1_PORT 22222
#define PORT1_IP_ADDR "192.168.0.2"
#define PORT1_MAC_ADDR 0xCBA987654321
#define PORT1_PAYLOAD "Hello, World from Port1!"

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

using namespace netsim;

int sc_main(int argc, char *argv[])
{
    sc_vector<TxPort> tx_ports("tx_ports", 2);
    sc_vector<RxPort> rx_ports("rx_ports", 2);

    EthSwitch eth_switch("eth_switch", 2, 2);

    tx_ports[0].tx_initiator.bind(eth_switch.rx_targets[0]);
    tx_ports[1].tx_initiator.bind(eth_switch.rx_targets[1]);
    eth_switch.tx_initiators[0].bind(rx_ports[0].rx_target);
    eth_switch.tx_initiators[1].bind(rx_ports[1].rx_target);

    UDPHeader udp_header;
    IPv4Header ipv4_header;
    EthernetII eth_ii;
    
    vector<sc_uint<8>> payload0;

    string payload0_str = PORT0_PAYLOAD;
    for (char c : payload0_str) {
        payload0.push_back(sc_uint<8>(static_cast<unsigned char>(c)));
    }

    vector<sc_uint<8>> payload1;

    string payload1_str = PORT1_PAYLOAD;
    for (char c : payload1_str) {
        payload1.push_back(sc_uint<8>(static_cast<unsigned char>(c)));
    }

    struct in_addr port0_ip{};
    struct in_addr port1_ip{};
    inet_pton(AF_INET, PORT0_IP_ADDR, &port0_ip);
    inet_pton(AF_INET, PORT1_IP_ADDR, &port1_ip);

    udp_header.src_port = PORT0_PORT;
    udp_header.dest_port = PORT1_PORT;
    ipv4_header.src_ip = ntohl(port0_ip.s_addr);
    ipv4_header.dest_ip = ntohl(port1_ip.s_addr);
    eth_ii.src_mac_addr = PORT0_MAC_ADDR;
    eth_ii.dest_mac_addr = PORT1_MAC_ADDR;

    Packet send_pkt0(udp_header, ipv4_header, eth_ii, payload0);
    vector<unsigned char> send_pkt0_serialized = send_pkt0.serialize();

    udp_header.src_port = PORT1_PORT;
    udp_header.dest_port = PORT0_PORT;
    ipv4_header.src_ip = ntohl(port1_ip.s_addr);
    ipv4_header.dest_ip = ntohl(port0_ip.s_addr);
    eth_ii.src_mac_addr = PORT1_MAC_ADDR;
    eth_ii.dest_mac_addr = PORT0_MAC_ADDR;

    Packet send_pkt1(udp_header, ipv4_header, eth_ii, payload1);
    vector<unsigned char> send_pkt1_serialized = send_pkt1.serialize();

    tx_ports[0].packet_fifo.write(send_pkt0_serialized);
    tx_ports[1].packet_fifo.write(send_pkt1_serialized);

    sc_start(100, SC_NS);

    vector<unsigned char> receive_pkt0_serialized = rx_ports[0].packet_fifo.read();
    Packet receive_pkt0;
    receive_pkt0.deserialize(receive_pkt0_serialized);

    vector<unsigned char> receive_pkt1_serialized = rx_ports[1].packet_fifo.read();
    Packet receive_pkt1;
    receive_pkt1.deserialize(receive_pkt1_serialized);

    string result0;
    for (const auto &bv : receive_pkt0.payload)
    {
        result0 += static_cast<char>(bv.to_uint());
    }

    std::stringstream ss0;
    ss0 << "Received packet: " << result0;
    SC_REPORT_INFO("EndPoint", ss0.str().c_str());

    string result1;
    for (const auto &bv : receive_pkt1.payload)
    {
        result1 += static_cast<char>(bv.to_uint());
    }

    std::stringstream ss1;
    ss1 << "Received packet: " << result1;
    SC_REPORT_INFO("EndPoint", ss1.str().c_str());

    return 0;
}
