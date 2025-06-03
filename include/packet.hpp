#ifndef PACKET_HPP
#define PACKET_HPP

#include <cstdint>
#include <vector>
#include <iostream>

using namespace std;

// IPv4头结构
#pragma pack(push, 1) // 确保结构体紧凑, 没有内存对齐
struct IPv4Header
{
    uint8_t version : 4 = 4;           // 协议版本号
    uint8_t ihl : 4 = 0;               // 头部长度（IHL - Internet Header Length）
    uint8_t tos = 0;                   // 服务类型（Type of Service）
    uint16_t total_length = 0;         // 总长度（包括头部和负载）
    uint16_t id = 0;                   // 标识符 (Identification)
    uint16_t flags : 3 = 0;            // 标志
    uint16_t fragment_offset : 13 = 0; // 分片偏移
    uint8_t ttl = 0;                   // 生存时间（Time to Live）
    uint8_t protocol = 0;              // 上层协议类型（如TCP, UDP等）
    uint16_t checksum = 0;             // 头部校验和
    uint32_t src_ip = 0;               // 源IP地址
    uint32_t dest_ip = 0;              // 目标IP地址
    vector<uint8_t> options;           // 选项字段（可选, 0 - 40Byte）
    vector<uint8_t> padding;           // 填充字段（确保头部长度是 ​32 位（4 字节）的整数倍。仅在存在选项且长度不足时添加）
};
#pragma pack(pop)

// TCP头结构
#pragma pack(push, 1) // 确保结构体紧凑, 没有内存对齐
struct TCPHeader
{
    uint16_t src_port = 0;        // 源端口
    uint16_t dest_port = 0;       // 目标端口
    uint32_t seq_num = 0;         // 序列号 (sequence number)
    uint32_t ack_num = 0;         // 确认号 (acknowledgment number)
    uint16_t data_offset : 4 = 0; // 数据偏移（TCP头部长度）
    uint16_t reserved : 6 = 0;    // 保留位
    uint16_t flags : 6 = 0;       // 控制位（如URG, ACK, PSH, RST, SYN, FIN）
    uint16_t window_size = 0;     // 窗口大小
    uint16_t checksum = 0;        // 校验和
    uint16_t urgent_pointer = 0;  // 紧急指针（如果设置了URG标志）
    vector<uint8_t> options;      // 选项字段（可选, 0 - 40Byte,需用 0 补齐至 32 位边界）
};
#pragma pack(pop)

// UDP头结构
#pragma pack(push, 1) // 确保结构体紧凑, 没有内存对齐
struct UDPHeader
{
    uint16_t src_port = 0;  // 源端口
    uint16_t dest_port = 0; // 目标端口
    uint16_t length = 0;    // UDP长度（头部+数据）
    uint16_t checksum = 0;  // 校验和
};
#pragma pack(pop)

// EthernetII结构
#pragma pack(push, 1) // 确保结构体紧凑, 没有内存对齐
struct EthernetII
{
    uint64_t dest_mac : 48 = 0; // 源mac
    uint64_t src_mac : 48 = 0;  // 目标mac
    uint16_t type = 0;          // 上层协议类型
    uint32_t fcs = 0;           // 帧校验序列（Frame Check Sequence)
};
#pragma pack(pop)

class Packet
{
public:
    vector<uint8_t> payload; // 数据载荷
    IPv4Header ipv4_header;  // IPv4头部
    TCPHeader tcp_header;    // TCP头部
    UDPHeader udp_header;    // UDP头部
    EthernetII eth_ii;       // Ethernet II头部和帧校验序列

    Packet() = default;

    Packet(TCPHeader tcp_header, IPv4Header ipv4_header, EthernetII eth_ii, vector<uint8_t> payload);

    Packet(UDPHeader udp_header, IPv4Header ipv4_header, EthernetII eth_ii, vector<uint8_t> payload);

    friend ostream& operator<<(ostream& os, const Packet& packet);
};

#endif // PACKET_HPP
