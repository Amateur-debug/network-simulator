#include "packet.hpp"

namespace netsim
{

using namespace std;

using namespace sc_core;
using namespace sc_dt;

uint64_t Packet::size() const
{
    uint64_t total_bits = 0;

    total_bits += eth_ii.dest_mac_addr.length();
    total_bits += eth_ii.src_mac_addr.length();
    total_bits += eth_ii.type.length();
    total_bits += eth_ii.fcs.length();

    total_bits += ipv4_header.version.length();
    total_bits += ipv4_header.ihl.length();
    total_bits += ipv4_header.tos.length();
    total_bits += ipv4_header.total_length.length();
    total_bits += ipv4_header.id.length();
    total_bits += ipv4_header.flags.length();
    total_bits += ipv4_header.fragment_offset.length();
    total_bits += ipv4_header.ttl.length();
    total_bits += ipv4_header.protocol.length();
    total_bits += ipv4_header.checksum.length();
    total_bits += ipv4_header.src_ip.length();
    total_bits += ipv4_header.dest_ip.length();
    total_bits += ipv4_header.options.size() * 8;

    if (static_cast<Protocol>(ipv4_header.protocol.to_uint()) == Protocol::UDP)
    {
        total_bits += udp_header.src_port.length();
        total_bits += udp_header.dest_port.length();
        total_bits += udp_header.length.length();
        total_bits += udp_header.checksum.length();
    }
    else if (static_cast<Protocol>(ipv4_header.protocol.to_uint()) == Protocol::TCP)
    {
        total_bits += tcp_header.src_port.length();
        total_bits += tcp_header.dest_port.length();
        total_bits += tcp_header.seq_num.length();
        total_bits += tcp_header.ack_num.length();
        total_bits += tcp_header.data_offset.length();
        total_bits += tcp_header.reserved.length();
        total_bits += tcp_header.flags.length();
        total_bits += tcp_header.window_size.length();
        total_bits += tcp_header.checksum.length();
        total_bits += tcp_header.urgent_pointer.length();
        total_bits += tcp_header.options.size() * 8;
    }

    total_bits += payload.size() * 8;

    return total_bits;
}

std::vector<unsigned char> Packet::serialize() const
{
    uint64_t total_bits = this->size();
    sc_unsigned data(total_bits);

    uint64_t current_bit = 0;

    data.range(current_bit + eth_ii.dest_mac_addr.length() - 1, current_bit) = eth_ii.dest_mac_addr;
    current_bit += eth_ii.dest_mac_addr.length();

    data.range(current_bit + eth_ii.src_mac_addr.length() - 1, current_bit) = eth_ii.src_mac_addr;
    current_bit += eth_ii.src_mac_addr.length();

    data.range(current_bit + eth_ii.type.length() - 1, current_bit) = eth_ii.type;
    current_bit += eth_ii.type.length();

    if (eth_ii.type.to_uint() == static_cast<uint16_t>(EthernetIIType::IPv4))
    {
        data.range(current_bit + ipv4_header.version.length() - 1, current_bit) = ipv4_header.version;
        current_bit += ipv4_header.version.length();
        data.range(current_bit + ipv4_header.ihl.length() - 1, current_bit) = ipv4_header.ihl;
        current_bit += ipv4_header.ihl.length();
        data.range(current_bit + ipv4_header.tos.length() - 1, current_bit) = ipv4_header.tos;
        current_bit += ipv4_header.tos.length();
        data.range(current_bit + ipv4_header.total_length.length() - 1, current_bit) = ipv4_header.total_length;
        current_bit += ipv4_header.total_length.length();
        data.range(current_bit + ipv4_header.id.length() - 1, current_bit) = ipv4_header.id;
        current_bit += ipv4_header.id.length();
        data.range(current_bit + ipv4_header.flags.length() - 1, current_bit) = ipv4_header.flags;
        current_bit += ipv4_header.flags.length();
        data.range(current_bit + ipv4_header.fragment_offset.length() - 1, current_bit) = ipv4_header.fragment_offset;
        current_bit += ipv4_header.fragment_offset.length();
        data.range(current_bit + ipv4_header.ttl.length() - 1, current_bit) = ipv4_header.ttl;
        current_bit += ipv4_header.ttl.length();
        data.range(current_bit + ipv4_header.protocol.length() - 1, current_bit) = ipv4_header.protocol;
        current_bit += ipv4_header.protocol.length();
        data.range(current_bit + ipv4_header.checksum.length() - 1, current_bit) = ipv4_header.checksum;
        current_bit += ipv4_header.checksum.length();
        data.range(current_bit + ipv4_header.src_ip.length() - 1, current_bit) = ipv4_header.src_ip;
        current_bit += ipv4_header.src_ip.length();
        data.range(current_bit + ipv4_header.dest_ip.length() - 1, current_bit) = ipv4_header.dest_ip;
        current_bit += ipv4_header.dest_ip.length();
        for (const auto &opt_byte : ipv4_header.options)
        {
            data.range(current_bit + opt_byte.length() - 1, current_bit) = opt_byte;
            current_bit += opt_byte.length();
        }
    }
    // TODO: Add serialization for IPv6 if needed

    if (ipv4_header.protocol.to_uint() == static_cast<uint8_t>(Protocol::UDP))
    {
        data.range(current_bit + udp_header.src_port.length() - 1, current_bit) = udp_header.src_port;
        current_bit += udp_header.src_port.length();
        data.range(current_bit + udp_header.dest_port.length() - 1, current_bit) = udp_header.dest_port;
        current_bit += udp_header.dest_port.length();
        data.range(current_bit + udp_header.length.length() - 1, current_bit) = udp_header.length;
        current_bit += udp_header.length.length();
        data.range(current_bit + udp_header.checksum.length() - 1, current_bit) = udp_header.checksum;
        current_bit += udp_header.checksum.length();
    }
    else if (ipv4_header.protocol.to_uint() == static_cast<uint8_t>(Protocol::TCP))
    {
        data.range(current_bit + tcp_header.src_port.length() - 1, current_bit) = tcp_header.src_port;
        current_bit += tcp_header.src_port.length();
        data.range(current_bit + tcp_header.dest_port.length() - 1, current_bit) = tcp_header.dest_port;
        current_bit += tcp_header.dest_port.length();
        data.range(current_bit + tcp_header.seq_num.length() - 1, current_bit) = tcp_header.seq_num;
        current_bit += tcp_header.seq_num.length();
        data.range(current_bit + tcp_header.ack_num.length() - 1, current_bit) = tcp_header.ack_num;
        current_bit += tcp_header.ack_num.length();
        data.range(current_bit + tcp_header.data_offset.length() - 1, current_bit) = tcp_header.data_offset;
        current_bit += tcp_header.data_offset.length();
        data.range(current_bit + tcp_header.reserved.length() - 1, current_bit) = tcp_header.reserved;
        current_bit += tcp_header.reserved.length();
        data.range(current_bit + tcp_header.flags.length() - 1, current_bit) = tcp_header.flags;
        current_bit += tcp_header.flags.length();
        data.range(current_bit + tcp_header.window_size.length() - 1, current_bit) = tcp_header.window_size;
        current_bit += tcp_header.window_size.length();
        data.range(current_bit + tcp_header.checksum.length() - 1, current_bit) = tcp_header.checksum;
        current_bit += tcp_header.checksum.length();
        data.range(current_bit + tcp_header.urgent_pointer.length() - 1, current_bit) = tcp_header.urgent_pointer;
        current_bit += tcp_header.urgent_pointer.length();
        for (const auto &opt_byte : tcp_header.options)
        {
            data.range(current_bit + opt_byte.length() - 1, current_bit) = opt_byte;
            current_bit += opt_byte.length();
        }
    }

    for (const auto &payload_byte : payload)
    {
        data.range(current_bit + payload_byte.length() - 1, current_bit) = payload_byte;
        current_bit += payload_byte.length();
    }

    data.range(current_bit + eth_ii.fcs.length() - 1, current_bit) = eth_ii.fcs;
    current_bit += eth_ii.fcs.length();

    vector<unsigned char> data_vec;
    data_vec.reserve(total_bits / 8);
    for (uint64_t bit = 0; bit < total_bits; bit += 8)
    {
        auto byte_unsigned = data.range(bit + 7, bit).to_uint();
        data_vec.push_back(static_cast<char>(byte_unsigned));
    }

    return data_vec;
}

void Packet::deserialize(const std::vector<unsigned char> &data_vec)
{
    uint64_t total_bits = data_vec.size() * 8;

    sc_unsigned data(total_bits);
    for (uint64_t i = 0; i < data_vec.size(); i++)
    {
        unsigned char byte = static_cast<unsigned char>(data_vec[i]);
        data.range(i * 8 + 7, i * 8) = byte;
    }

    uint64_t current_bit = 0;

    eth_ii.dest_mac_addr = data.range(current_bit + eth_ii.dest_mac_addr.length() - 1, current_bit);
    current_bit += eth_ii.dest_mac_addr.length();

    eth_ii.src_mac_addr = data.range(current_bit + eth_ii.src_mac_addr.length() - 1, current_bit);
    current_bit += eth_ii.src_mac_addr.length();

    eth_ii.type = data.range(current_bit + eth_ii.type.length() - 1, current_bit);
    current_bit += eth_ii.type.length();

    if (eth_ii.type.to_uint() == static_cast<uint16_t>(EthernetIIType::IPv4))
    {
        ipv4_header.version = data.range(current_bit + ipv4_header.version.length() - 1, current_bit);
        current_bit += ipv4_header.version.length();
        ipv4_header.ihl = data.range(current_bit + ipv4_header.ihl.length() - 1, current_bit);
        current_bit += ipv4_header.ihl.length();
        ipv4_header.tos = data.range(current_bit + ipv4_header.tos.length() - 1, current_bit);
        current_bit += ipv4_header.tos.length();
        ipv4_header.total_length = data.range(current_bit + ipv4_header.total_length.length() - 1, current_bit);
        current_bit += ipv4_header.total_length.length();
        ipv4_header.id = data.range(current_bit + ipv4_header.id.length() - 1, current_bit);
        current_bit += ipv4_header.id.length();
        ipv4_header.flags = data.range(current_bit + ipv4_header.flags.length() - 1, current_bit);
        current_bit += ipv4_header.flags.length();
        ipv4_header.fragment_offset = data.range(current_bit + ipv4_header.fragment_offset.length() - 1, current_bit);
        current_bit += ipv4_header.fragment_offset.length();
        ipv4_header.ttl = data.range(current_bit + ipv4_header.ttl.length() - 1, current_bit);
        current_bit += ipv4_header.ttl.length();
        ipv4_header.protocol = data.range(current_bit + ipv4_header.protocol.length() - 1, current_bit);
        current_bit += ipv4_header.protocol.length();
        ipv4_header.checksum = data.range(current_bit + ipv4_header.checksum.length() - 1, current_bit);
        current_bit += ipv4_header.checksum.length();
        ipv4_header.src_ip = data.range(current_bit + ipv4_header.src_ip.length() - 1, current_bit);
        current_bit += ipv4_header.src_ip.length();
        ipv4_header.dest_ip = data.range(current_bit + ipv4_header.dest_ip.length() - 1, current_bit);
        current_bit += ipv4_header.dest_ip.length();

        uint64_t ipv4_header_bytes = ipv4_header.ihl.to_uint() * 4;
        if (ipv4_header_bytes > 20)
        {
            uint64_t option_bytes = ipv4_header_bytes - 20;
            ipv4_header.options.clear();
            ipv4_header.options.resize(option_bytes);
            for (uint64_t i = 0; i < option_bytes; ++i)
            {
                ipv4_header.options[i] = data.range(current_bit + 7, current_bit);
                current_bit += 8;
            }
        }
        else
        {
            ipv4_header.options.clear();
        }
    }
    // TODO: IPv6 可在此扩展

    if (ipv4_header.protocol.to_uint() == static_cast<uint8_t>(Protocol::UDP))
    {
        udp_header.src_port = data.range(current_bit + udp_header.src_port.length() - 1, current_bit);
        current_bit += udp_header.src_port.length();
        udp_header.dest_port = data.range(current_bit + udp_header.dest_port.length() - 1, current_bit);
        current_bit += udp_header.dest_port.length();
        udp_header.length = data.range(current_bit + udp_header.length.length() - 1, current_bit);
        current_bit += udp_header.length.length();
        udp_header.checksum = data.range(current_bit + udp_header.checksum.length() - 1, current_bit);
        current_bit += udp_header.checksum.length();
        tcp_header.options.clear();
    }
    else if (ipv4_header.protocol.to_uint() == static_cast<uint8_t>(Protocol::TCP))
    {
        tcp_header.src_port = data.range(current_bit + tcp_header.src_port.length() - 1, current_bit);
        current_bit += tcp_header.src_port.length();
        tcp_header.dest_port = data.range(current_bit + tcp_header.dest_port.length() - 1, current_bit);
        current_bit += tcp_header.dest_port.length();
        tcp_header.seq_num = data.range(current_bit + tcp_header.seq_num.length() - 1, current_bit);
        current_bit += tcp_header.seq_num.length();
        tcp_header.ack_num = data.range(current_bit + tcp_header.ack_num.length() - 1, current_bit);
        current_bit += tcp_header.ack_num.length();
        tcp_header.data_offset = data.range(current_bit + tcp_header.data_offset.length() - 1, current_bit);
        current_bit += tcp_header.data_offset.length();
        tcp_header.reserved = data.range(current_bit + tcp_header.reserved.length() - 1, current_bit);
        current_bit += tcp_header.reserved.length();
        tcp_header.flags = data.range(current_bit + tcp_header.flags.length() - 1, current_bit);
        current_bit += tcp_header.flags.length();
        tcp_header.window_size = data.range(current_bit + tcp_header.window_size.length() - 1, current_bit);
        current_bit += tcp_header.window_size.length();
        tcp_header.checksum = data.range(current_bit + tcp_header.checksum.length() - 1, current_bit);
        current_bit += tcp_header.checksum.length();
        tcp_header.urgent_pointer = data.range(current_bit + tcp_header.urgent_pointer.length() - 1, current_bit);
        current_bit += tcp_header.urgent_pointer.length();

        uint64_t tcp_header_bytes = tcp_header.data_offset.to_uint() * 4;
        if (tcp_header_bytes > 20)
        {
            uint64_t option_bytes = tcp_header_bytes - 20;
            tcp_header.options.clear();
            tcp_header.options.resize(option_bytes);
            for (uint64_t i = 0; i < option_bytes; ++i)
            {
                tcp_header.options[i] = data.range(current_bit + 7, current_bit);
                current_bit += 8;
            }
        }
        else
        {
            tcp_header.options.clear();
        }
    }
    else
    {
        // 其他协议：清空 L4 结构
        udp_header = {};
        tcp_header = {};
    }

    // 计算 payload 大小：剩余位减去 FCS
    if (total_bits < current_bit + eth_ii.fcs.length())
    {
        payload.clear();
        eth_ii.fcs = 0;
        return;
    }

    uint64_t remaining_bits = total_bits - current_bit;
    if (remaining_bits >= eth_ii.fcs.length())
    {
        uint64_t payload_bits = remaining_bits - eth_ii.fcs.length();
        uint64_t payload_bytes = payload_bits / 8;
        payload.clear();
        payload.resize(payload_bytes);
        for (uint64_t i = 0; i < payload_bytes; ++i)
        {
            payload[i] = data.range(current_bit + 7, current_bit);
            current_bit += 8;
        }
    }
    else
    {
        payload.clear();
    }

    // FCS
    eth_ii.fcs = data.range(current_bit + eth_ii.fcs.length() - 1, current_bit);
    current_bit += eth_ii.fcs.length();
}

}
