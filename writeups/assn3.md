Assignment 3 Writeup
=============

My name: Ko Minseok

My POVIS ID: koms0109

My student ID (numeric): 20220615

This assignment took me about 4 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

## Program Structure and Design of the TCPSender:
### Retransmission Timer
- `start_timer` (set elapsed time to 0 and start timer)
- `stop_timer` (stop timer)
- `timeout` (check whether timeout has occured. If timeout has occured and window size is nonzero, double the RTO)
- `restart_timer_with_initial_rto` (Reset RTO to initla RTO and start timer)

### `fill_window`
- send as much as possible: `while(LastByteSent - LastByteAcked <= rwnd) <=> while(NextByteToSend - ACK (next byte expected to receive) <= rwnd)`
- each segment size: `min(TCPConfig::MAX_PAYLOAD_SIZE, _rwnd - _next_seqno + _ackno)`
- Set the values of SYN, FIN, sequence number, and payload in the segment, then push it to `_segments_out`
- When new segment sent, set `_consecutive_retransmissions = 0`
### `ack_received`
- When the received window size is 0, set `_rwnd = 1` and `_rwnd_zero = true`. The `_rwnd_zero` flag indicates whether the received window size was zero, allowing other functions to handle this case separately
- Treat each segment as fully outstanding until it’s been fully acknowledged: 
```c
     if (absolute_seqno + segment_length > _ackno)
        break;
```
- When valid ACK received, set RTO to initial value by invoking `restart_timer_with_initial_rto`. 
- After that, stop timer if all outstandings have been acknowledged.
### `tick`
- If timeout happens, retransmit the oldest outstanding segment. `fill_window` pushes outstanding segment in-order, so the oldest one is `_oustanding_segments.front()`
### `send_empty_segment`
- just send empty segment with sequence number. We don't have to retransmit it.

## Implementation Challenges:
In the initial implementation, the while loop in `fill_window` did not check for the empty segment condition. This caused an infinite loop when no data and no flags had been sent.

## Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
