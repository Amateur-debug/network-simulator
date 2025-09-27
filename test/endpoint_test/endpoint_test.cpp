#include <arpa/inet.h>
#include <sys/socket.h>

#include "endpoint.hpp"
#include "eth_switch.hpp"
#include "endpoint_test.hpp"

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

Client::Client(sc_module_name name)
    : sc_module(name),
      send_initiator("send_initiator"),
      udp_client("udp_client", 1, 1)
{   
    udp_client.tx_initiators[0].bind(send_initiator);
    receive_target.bind(udp_client.rx_targets[0]);

    SC_THREAD(send);
}

void Client::send()
{
    SC_REPORT_INFO("Client", "Starting send process...");

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

    Packet pkt(udp_header, ipv4_header, eth_ii, payload);

    wait(1, SC_NS);

    uint64_t count = 0;

    while (true)
    {
        stringstream ss;
        ss << "Sending packet number " << count;
        SC_REPORT_INFO("Client", ss.str().c_str());

        udp_client.send(pkt);

        ss.str(""); 
        ss.clear();
        ss << "Sending packet number " << count << " completed";
        SC_REPORT_INFO("Client", ss.str().c_str());

        count++;

        wait(10, SC_NS);
    }
}

Server::Server(sc_module_name name)
    : sc_module(name),
      receive_target("receive_target"),
      udp_server("udp_server", 1, 1)
{   
    udp_server.tx_initiators[0].bind(send_initiator);
    receive_target.bind(udp_server.rx_targets[0]);
    SC_THREAD(receive);
}

void Server::receive()
{   
    SC_REPORT_INFO("Server", "Starting receive process...");
    
    uint64_t count = 0;

    while (true)
    {
        stringstream ss;
        ss << "Receiving packet number " << count;
        SC_REPORT_INFO("Server", ss.str().c_str());

        udp_server.receive();

        ss.str(""); 
        ss.clear();
        ss << "Receiving packet number " << count << " completed";
        SC_REPORT_INFO("Server", ss.str().c_str());

        count++;

        wait(10, SC_NS);
    }
}

}

using namespace netsim;

int sc_main(int argc, char *argv[])
{
    Server server("server");
    Client client("client");

    EthSwitch eth_switch("eth_switch", 2, 2);

    eth_switch.set_mac_addr_table(SERVER_MAC_ADDR, 1);

    client.send_initiator.bind(eth_switch.rx_targets[0]);
    server.send_initiator.bind(eth_switch.rx_targets[1]);
    eth_switch.tx_initiators[0].bind(client.receive_target);
    eth_switch.tx_initiators[1].bind(server.receive_target);
    
    sc_start(50, SC_NS);
    return 0;
}
