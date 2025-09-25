#ifndef ETH_SWITCH_HPP
#define ETH_SWITCH_HPP

#include "systemc"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

#include "network_port.hpp"
#include "stream_out.hpp"

namespace netsim
{

struct MacAddrTableEntry
{
    sc_dt::sc_uint<48> mac_addr;
    int port_id;
    sc_core::sc_time time_stamp;
    bool valid = false;
};

class MacAddrTable
{
public:
    MacAddrTable(int size);

    int insert(const sc_dt::sc_uint<48> &mac_addr, const int &port_id);

    void insert(const sc_dt::sc_uint<48> &mac_addr, const int &port_id, const int &pos);

    void remove(int pos);

    int find_oldest_entry();

    int lookup(const sc_dt::sc_uint<48> &mac_addr);

private:
    std::vector<MacAddrTableEntry> table;
};

struct ProcFifoEntry
{
    int port_id;
    std::vector<unsigned char> pkt;

    friend std::ostream &operator<<(std::ostream &os, const ProcFifoEntry &entry);
};

class EthSwitch : public sc_core::sc_module
{
public:
    sc_core::sc_vector<tlm_utils::multi_passthrough_initiator_socket<EthSwitch>> tx_initiators;
    sc_core::sc_vector<tlm_utils::multi_passthrough_target_socket<EthSwitch>> rx_targets;
    
    void set_mac_addr_table(const sc_dt::sc_uint<48> &mac_addr, const int &port_id);

    SC_HAS_PROCESS(EthSwitch);

    EthSwitch(sc_core::sc_module_name name,
              const int &num_tx_port,
              const int &num_rx_port,
              const int &fifo_size = 200,
              const int &mac_addr_table_size = 100);

private:
    sc_core::sc_vector<TxPort> tx_ports;
    sc_core::sc_vector<RxPort> rx_ports;

    MacAddrTable mac_addr_table;
    sc_core::sc_fifo<ProcFifoEntry> process_fifo;
    int mac_addr_table_size;
    int used_mac_addr_entries_num;

    void learn(const sc_dt::sc_uint<48> &mac_addr, int &port_id);

    void send(int port_id, std::vector<unsigned char> &pkt);

    void forward();

    void schedule();
};

}

#endif
