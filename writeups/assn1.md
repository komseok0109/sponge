Assignment 1 Writeup
=============

My name: Ko Minseok

My POVIS ID: koms0109

My student ID (numeric): 20220615

This assignment took me about [3] hours to do (including the time on studying, designing, and writing the code).

## Program Structure and Design of the StreamReassembler:
The core function of the **StreamReassembler** is to receive substrings and write them into an output stream. Because these 
substrings may arrive in any order, they cannot be immediately written to the output stream as they are received. Instead, the 
reassembler must arrange the bytes in the correct order before writing them. This requires a data structure to hold out-of-order 
bytes.

### Data Structures
Either a deque or a vector with a fixed size equal to the capacity is used. Based on the tests, a deque performs faster, so deque is used. Two deques are employed:
- One deque stores the bytes. (`_bytes_to_reassemble`)
- The other is a boolean deque that records whether each position in the byte deque contains a valid received byte or is still 
empty. Since substrings can arrive out of order, gaps (empty spaces) may exist between the valid bytes. (`_indicators`)
```
first unread|<---------------------capacity---------------------->|
+-------------------------------------------------------------------------------+
|###########|#############|____####___#############___##___###____|             |
                          |FFFFTTTTFFFTTTTTTTTTTTTTFFFTTFFFTTTFFFF|             |
+-------------------------------------------------------------------------------+
0        first unassembled|<--remaining_capacity----------------->|first unacceptable    
```
### `push_substring` function flow: how StreamReassembler accepts substring
[1] Calculate the range boundaries: first_unassembled, first_unacceptable

[2] If `eof` is true and within acceptable range, update _eof

[3] If index + data overlaps with acceptable range:
 - Calculate the start and end indices of `data` within the acceptable range w.r.t whole stream
- Extract the acceptable bytes

[4] For each acceptable byte:
- If byte hasn't been placed yet, insert into `_bytes_reassemble`
- Mark byte as valid and increment `_unassembled_bytes`

[5] Calculate bytes to assemble by checking indicators
- Write the assembled bytes to output stream

[6] Update buffers (remove assembled bytes, resize)

[7] If eof is true and no unassembled bytes, end input

## Implementation Challenges:
Initially, I tried using the `count` method to calculate `_unassembled_bytes` by counting the number of true values in
`_indicators`. The logic was the same as the current code, and the test passed, but the performance was not good.

## Remaining Bugs:
Although not a bug, as stated in the FAQ, inconsistent substrings are assumed not to be input. That is, the byte stored at a 
specific index is not expected to change. Handling such cases is not implemented.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
