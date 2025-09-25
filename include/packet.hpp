#ifndef PACKET_HPP
#define PACKET_HPP

#include "systemc"

namespace netsim
{

// TCP头结构
struct TCPHeader
{
    sc_dt::sc_uint<16> src_port = 0;        // 源端口
    sc_dt::sc_uint<16> dest_port = 0;       // 目标端口
    sc_dt::sc_uint<32> seq_num = 0;         // 序列号 (sequence number)
    sc_dt::sc_uint<32> ack_num = 0;         // 确认号 (acknowledgment number)
    sc_dt::sc_uint<4> data_offset = 0;      // 数据偏移（TCP头部长度）
    sc_dt::sc_uint<6> reserved = 0;         // 保留位
    sc_dt::sc_uint<6> flags = 0;            // 控制位（如URG, ACK, PSH, RST, SYN, FIN）
    sc_dt::sc_uint<16> window_size = 0;     // 窗口大小
    sc_dt::sc_uint<16> checksum = 0;        // 校验和
    sc_dt::sc_uint<16> urgent_pointer = 0;  // 紧急指针（如果设置了URG标志）
    std::vector<sc_dt::sc_uint<8>> options; // 选项字段（可选, 0 - 40Byte,需用 0 补齐至 32 位边界）
};

// UDP头结构
struct UDPHeader
{
    sc_dt::sc_uint<16> src_port = 0;  // 源端口
    sc_dt::sc_uint<16> dest_port = 0; // 目标端口
    sc_dt::sc_uint<16> length = 0;    // UDP长度（头部+数据）
    sc_dt::sc_uint<16> checksum = 0;  // 校验和
};

// IPv4头结构
struct IPv4Header
{
    sc_dt::sc_uint<4> version = 4;          // 协议版本号
    sc_dt::sc_uint<4> ihl = 0;              // 头部长度（IHL - Internet Header Length）
    sc_dt::sc_uint<8> tos = 0;              // 服务类型（Type of Service）
    sc_dt::sc_uint<16> total_length = 0;    // 总长度（包括头部和负载）
    sc_dt::sc_uint<16> id = 0;              // 标识符 (Identification)
    sc_dt::sc_uint<3> flags = 0;            // 标志
    sc_dt::sc_uint<13> fragment_offset = 0; // 分片偏移
    sc_dt::sc_uint<8> ttl = 0;              // 生存时间（Time to Live）
    sc_dt::sc_uint<8> protocol = 0;         // 上层协议类型（如TCP, UDP等）
    sc_dt::sc_uint<16> checksum = 0;        // 头部校验和
    sc_dt::sc_uint<32> src_ip = 0;          // 源IP地址
    sc_dt::sc_uint<32> dest_ip = 0;         // 目标IP地址
    std::vector<sc_dt::sc_uint<8>> options; // 选项字段（可选, 0 - 40Byte，需用 0 补齐至 32 位边界）
};

enum class Protocol : uint8_t
{
    TCP = 0x06, // TCP协议
    UDP = 0x11, // UDP协议
};

// EthernetII结构
struct EthernetII
{
    sc_dt::sc_uint<48> dest_mac_addr = 0; // 目标mac
    sc_dt::sc_uint<48> src_mac_addr = 0;  // 源mac
    sc_dt::sc_uint<16> type = 0;          // 上层协议类型
    sc_dt::sc_uint<32> fcs = 0;           // 帧校验序列（Frame Check Sequence)
};

enum class EthernetIIType : uint16_t
{
    IPv4 = 0x0800, // IPv4协议
    IPv6 = 0x86DD, // IPv6协议
};

class Packet
{
public:
    std::vector<sc_dt::sc_uint<8>> payload; // 数据载荷
    IPv4Header ipv4_header;                 // IPv4头部
    TCPHeader tcp_header;                   // TCP头部
    UDPHeader udp_header;                   // UDP头部
    EthernetII eth_ii;                      // Ethernet II头部和帧校验序列

    Packet() = default;

    Packet(TCPHeader tcp_header, IPv4Header ipv4_header, EthernetII eth_ii, std::vector<sc_dt::sc_uint<8>> payload)
        : tcp_header(tcp_header),
          ipv4_header(ipv4_header),
          eth_ii(eth_ii),
          payload(payload) {}

    Packet(UDPHeader udp_header, IPv4Header ipv4_header, EthernetII eth_ii, std::vector<sc_dt::sc_uint<8>> payload)
        : udp_header(udp_header),
          ipv4_header(ipv4_header),
          eth_ii(eth_ii),
          payload(payload) {}

    std::uint64_t size() const;

    std::vector<unsigned char> serialize() const;
    void deserialize(const std::vector<unsigned char>& data_vec);

    friend std::ostream &operator<<(std::ostream &os, const Packet &packet)
    {
        os << "Packet[payload_size=" << packet.payload.size()
           << ", protocol=" << packet.ipv4_header.protocol.to_uint() << "]";
        return os;
    }
};

}

#endif
