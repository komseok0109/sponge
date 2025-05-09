#include "router.hh"

#include <iostream>

using namespace std;

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    _routing_table.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    const uint32_t dst_address = dgram.header().dst;

    optional<RoutingTableEntry> longest_prefix_match;
    for (const auto &entry: _routing_table){
        if (entry.prefix_length == 0 && !longest_prefix_match.has_value())
            longest_prefix_match = entry;
        else if (entry.prefix_length != 0 && dst_address >> (32 - entry.prefix_length) == entry.route_prefix >> (32-entry.prefix_length)){
            if (!longest_prefix_match.has_value() || entry.prefix_length > longest_prefix_match.value().prefix_length)
                longest_prefix_match = entry;
        }
    }
    
    if (longest_prefix_match.has_value() && dgram.header().ttl > 1){
        const optional<Address> next_hop_opt = longest_prefix_match.value().next_hop;
        const Address next_hop = next_hop_opt.has_value() ? next_hop_opt.value() : Address::from_ipv4_numeric(dst_address);
        dgram.header().ttl--;
        interface(longest_prefix_match.value().interface_num).send_datagram(dgram, next_hop);
    }
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
