Assignment 4 Writeup
=============

My name: Ko Minseok

My POVIS ID: koms0109

My student ID (numeric): 20220615

This assignment took me about [6] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.40, 0.38]

## Program Structure and Design of the TCPConnection:
### Helper Functions
- `_tcp_send_segment`:  
  - Sends all queued TCP segments by pushing them to `_segments_out`.  
  - Before sending each segment, it sets the ACK flag and window size if `_receiver.ackno().has_value()` is true.  
  - This function is called whenever the `TCPSender` has potential to send data.
- `_deal_with_rst(const bool received)`:  
  Handles logic for connection reset (RST).  
  - If `received == true`, the function sets both inbound and outbound streams to the error state and deactivates the connection.  
  - If `received == false`, it additionally sends an RST segment to the peer before deactivating the connection.
---
### `segment_received(const TCPSegment &seg)`
- If the segment has the RST flag (`seg.header().rst == true`), terminate the connection by calling `_deal_with_rst(true)`.
- Otherwise, hand the segment over to the receiver: `_receiver.segment_received(seg)`
- If ACK is set (`seg.header().ack == true`), acknowledge it via: `_sender.ack_received`  
- If the segment occupies sequence space (`seg.length_in_sequence_space() > 0`):  
  - Call `_sender.fill_window()` to prepare outgoing segments.  
  - If no segment was actually pushed, send an empty one to reflect updates in ACK and window size:
    ```cpp
    if (_sender.segments_out().empty())
        _sender.send_empty_segment();
    ```
- Respond to keep-alive segment:
    ```cpp
    if (_receiver.ackno().has_value() &&
      seg.length_in_sequence_space() == 0 &&
      seg.header().seqno == _receiver.ackno().value() - 1) {
      _sender.send_empty_segment();
    }
    ```
- Edge case: If an ACK is received but the sender hasn’t sent SYN yet, ignore the segment
- If the inbound stream has ended, but the outbound stream hasn't:
  ```cpp
  if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof())
      _linger_after_streams_finish = false;
  ```
---
### `tick(const size_t ms_since_last_tick)`
- Let the `TCPSender` know time has passed: `_sender.tick(ms_since_last_tick)`
- If `_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS`, invoke `_deal_with_rst(false)`
- Clean shutdown conditions:
  - All of the following must be true:
    1. `_receiver.stream_out().input_ended()`
    2. `_sender.stream_in().eof()`
    3. `_sender.bytes_in_flight() == 0`
  - Then check: `if (!_linger_after_streams_finish or _time_since_last_segment_received >= 10 * _cfg.rt_timeout)`
## Implementation Challenges:
The detailed and well-structured assignment documentation significantly reduced the challenges during the implementation process
## Remaining Bugs:
While the current implementation is functionally correct, there may be room for optimizing throughput. This possibility has not been explored in the current work

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
