#ifndef ETH_SWITCH_HPP
#define ETH_SWITCH_HPP

#include "systemc"
#include "tlm"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

#include "network_port.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

namespace netsim
{

struct MacAddrTableEntry
{
    string mac_addr;
    int port_id;
    sc_time time_stamp;
    bool valid = false;
};

class MacAddrTable
{
public:
    MacAddrTable(int size);

    int insert(const string &mac_addr, int &port_id);

    void insert(const string &mac_addr, int &port_id, int &pos);

    void remove(int pos);

    int MacAddrTable::find_oldest_entry();

    int lookup(const string &mac_addr);

private:
    vector<MacAddrTableEntry> table;
};

struct ProcFifoEntry
{
    int port_id;
    Packet packet;
};

class EthSwitch : public sc_module
{
public:
    sc_vector<simple_initiator_socket<EthSwitch>> tx_ports_initiators;
    sc_vector<simple_target_socket<EthSwitch>> rx_ports_targets;

    // Register SystemC thread
    SC_HAS_PROCESS(EthSwitch);

    // Constructor
    EthSwitch(sc_module_name name, const int &num_tx_port = 2, const int &num_rx_port = 2,
              const int &fifo_size = 200, const int &mac_addr_table_size = 100);

private:
    sc_vector<TxPort> tx_ports;
    sc_vector<RxPort> rx_ports;

    multi_passthrough_initiator_socket<EthSwitch> forward_initiator;
    multi_passthrough_initiator_socket<EthSwitch> fetch_initiator;

    MacAddrTable mac_addr_table;

    sc_fifo<ProcFifoEntry> process_fifo;

    int mac_addr_table_size;

    int used_mac_addr_entries_num = 0;

    void learn(const string &mac_addr, int &port_id);

    void forward(int &port_id, Packet &packet);

    void schedule();

    void process();
    
};

}

#endif