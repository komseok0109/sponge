Assignment 5 Writeup
=============

My name: Ko Minseok

My POVIS ID: koms0109

My student ID (numeric): 20220615

This assignment took me about 3 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

## Program Structure and Design of the NetworkInterface:
### Key Data Structures
- `unordered_map<uint32_t, pair<EthernetAddress, int>> _arp_cache`: It stores resolved IP-to-MAC address mappings with a TTL (time-to-live) in milliseconds.
- `unordered_map<uint32_t, queue<InternetDatagram>> _ip_datagrams_waiting_for_arp_reply` : It buffers outgoing datagrams that are waiting for an ARP reply.
- `unordered_map<uint32_t, size_t> _arp_request_elapsed` : It tracks how much time has passed since the last ARP request was sent to each IP address
### Main Functions
- `send_datagram`
    - If the next hop's MAC address is known (in `_arp_cache`), it sends it immediately.
    - Otherwise, it sends an ARP request (if not recently (5ms) sent), and queues the datagram in `_ip_datagrams_waiting_for_arp_reply`.
- `recv_frame`
    - For IPv4 `EthernetHeader::TYPE_IPv4` : parses and returns the datagram if addressed to this interface.
    - For ARP `EthernetHeader::TYPE_ARP` : updates `_arp_cache`, replies to ARP requests, and flushes any buffered datagrams if the target MAC has been resolved.
- `tick`
    - Decreases TTLs in `_arp_cache`, removing expired entries.
    - Increments elapsed time for each IP in `_arp_request_elapsed` to control ARP request rate.

## Implementation Challenges:
[]

## Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
