Assignment 6 Writeup
=============

My name: Ko Minseok

My POVIS ID: koms0109

My student ID (numeric): 20220615

This assignment took me about 2 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

## Program Structure and Design of the Router:
### Program Structure
- `struct RoutingTableEntry`: Represents a single entry in the router's routing table. Each entry defines a forwarding rule consisting of:
  - `uint32_t route_prefix`
  - `uint8_t prefix_length`
  - `std::optional<Address> next_hop`
  - `size_t interface_num`
- `std::vector<RoutingTableEntry> _routing_table{}`: This is the internal routing table. It stores all RoutingTableEntry objects, which define the router's forwarding behavior. The entries are searched linearly when processing each datagram to find the best match.
### How to find longest prefix match
During iteration over `_routing_table`, we apply the following logic to find the best route:
- A prefix length of 0 means 
```c
if (entry.prefix_length == 0 && !longest_prefix_match.has_value())
```
the route matches any IP. We only update if no other router has been matched yet.
- If prefix length is positive, find match by comparing most significant `prefix_length` bits of destination IP and route prefix.
```c
else if (entry.prefix_length != 0 &&
         (dst_address >> (32 - entry.prefix_length)) ==
         (entry.route_prefix >> (32 - entry.prefix_length)))
```
- We keep the one with longest prefix:
```c
if (!longest_prefix_match.has_value() ||
    entry.prefix_length > longest_prefix_match->prefix_length)
```
## Implementation Challenges:
[]

## Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
