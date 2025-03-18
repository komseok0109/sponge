Assignment 2 Writeup
=============

My name: Ko Minseok

My POVIS ID: koms0109

My student ID (numeric): 20220615

This assignment took me about [4] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

# Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
## wrap
(n + isn) mod 2^32 = (n mod 2^32 + isn mod 2^32) mod 2^32, so we can implement wrap(n, isn) just by returning isn + n
## unwrap
Let a * ALIGN be an aligned checkpoint by shifting the checkpoint to the nearest multiple
a * ALIGN = (checkpoint / ALIGN) << 32 (ALIGN = 2^32)
(i) x = (n - isn + a * ALIGN) > checkpoint
```
        (a-1) * ALIGN     x - ALIGN          a * ALIGN        checkpoint             x 
+--------------------------------------------------------------------------------------------+
            |                |                   |                 |                 |       |
+--------------------------------------------------------------------------------------------+
```
In this case, we have two candidates x (mid), x - ALIGN (low). Compare which one is closer to checkpoint and return.
(ii) x = (n - isn + a * ALGIN) < checkpoint
```
        a * ALIGN     x            checkpoint      (a+1) * ALIGN         x + ALIGN 
+--------------------------------------------------------------------------------------------+
            |         |                |                 |                 |                 |
+--------------------------------------------------------------------------------------------+
```
In this case, we have two candidates x (mid), x + ALIGN (high). Compare which one is closer to checkpoint and return.
## segment_received
(i) Set the initial sequence number and flag if the first segment with SYN flag set has been received.
(ii) If SYN has received, push data to stream reassembler.
checkpoint = last byte reassembled = bytes_written() in absolute sequence number space
index = unwrap(seqno + syn, isn, checkpoint) - 1 (adjusted to stream index space)
=> push_substring(payload, index, header's FIN flag)
## ackno
ackno = first unassembled byte 
      = bytes_written() in stream index space
      = bytes_written() + SYN + FIN in absolute sequence number space
      = wrap(bytes_written() + SYN + FIN) in sequence number space
# Implementation Challenges:
- Understanding and implementing the relationship between `int32_t`, `uint32_t`, and `uint64_t`.
- Considering various cases in the `unwrap` function.

# Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
