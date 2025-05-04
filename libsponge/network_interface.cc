#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    if(_arp_cache.find(next_hop_ip) != _arp_cache.end()){
        EthernetFrame frame_to_send;
        frame_to_send.header() = {_arp_cache[next_hop_ip].first, _ethernet_address, EthernetHeader::TYPE_IPv4};
        frame_to_send.payload() = dgram.serialize();
        _frames_out.push(frame_to_send);
    } 
    else {
        if(_arp_request_elapsed.find(next_hop_ip) == _arp_request_elapsed.end() || _arp_request_elapsed[next_hop_ip] >= 5000) {
            ARPMessage arp_request;
            arp_request.opcode = ARPMessage::OPCODE_REQUEST;
            arp_request.sender_ethernet_address = _ethernet_address;
            arp_request.sender_ip_address = _ip_address.ipv4_numeric();
            arp_request.target_ethernet_address = {};
            arp_request.target_ip_address = next_hop_ip;
            
            EthernetFrame frame_to_send;
            frame_to_send.header() = {ETHERNET_BROADCAST, _ethernet_address, EthernetHeader::TYPE_ARP};
            frame_to_send.payload() = arp_request.serialize();
            _frames_out.push(frame_to_send);

            _arp_request_elapsed[next_hop_ip] = 0;
        }
        _ip_datagrams_waiting_for_arp_reply[next_hop_ip].push(dgram);
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST) {
        return nullopt;
    }
    if (frame.header().type == EthernetHeader::TYPE_IPv4){
        InternetDatagram dgram_to_return;
        if (dgram_to_return.parse(frame.payload()) == ParseResult::NoError)
            return dgram_to_return;
    }
    if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage received_arp;
        if (received_arp.parse(frame.payload()) == ParseResult::NoError) {
            _arp_cache[received_arp.sender_ip_address] = {received_arp.sender_ethernet_address, 30000};
            if (received_arp.opcode == ARPMessage::OPCODE_REQUEST && received_arp.target_ip_address == _ip_address.ipv4_numeric()) {
                ARPMessage arp_reply;
                arp_reply.opcode = ARPMessage::OPCODE_REPLY;
                arp_reply.sender_ethernet_address = _ethernet_address;
                arp_reply.sender_ip_address = _ip_address.ipv4_numeric();
                arp_reply.target_ethernet_address = received_arp.sender_ethernet_address;
                arp_reply.target_ip_address = received_arp.sender_ip_address;

                EthernetFrame frame_to_send;
                frame_to_send.header() = {received_arp.sender_ethernet_address, _ethernet_address, EthernetHeader::TYPE_ARP};
                frame_to_send.payload() = arp_reply.serialize();
                _frames_out.push(frame_to_send); 
            }
            if (_ip_datagrams_waiting_for_arp_reply.find(received_arp.sender_ip_address) != _ip_datagrams_waiting_for_arp_reply.end()){
                queue<InternetDatagram> &ip_datagrams_to_send = _ip_datagrams_waiting_for_arp_reply[received_arp.sender_ip_address];
                while (!ip_datagrams_to_send.empty()){
                    EthernetFrame frame_to_send;
                    frame_to_send.header() = {received_arp.sender_ethernet_address, _ethernet_address, EthernetHeader::TYPE_IPv4};
                    frame_to_send.payload() = ip_datagrams_to_send.front().serialize();
                    _frames_out.push(frame_to_send);
                    ip_datagrams_to_send.pop();
                }
                _ip_datagrams_waiting_for_arp_reply.erase(received_arp.sender_ip_address);
                _arp_request_elapsed.erase(received_arp.sender_ip_address);
            }
        }
    }
    return nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) { 
    for (auto it = _arp_cache.begin(); it != _arp_cache.end(); ) {
        it->second.second -= ms_since_last_tick;
        if (it->second.second <= 0)
            it = _arp_cache.erase(it);
        else
            it++;
    }
    for (auto it = _arp_request_elapsed.begin(); it != _arp_request_elapsed.end(); it++) 
        it->second += ms_since_last_tick;
}
