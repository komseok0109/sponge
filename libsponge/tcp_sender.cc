#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <algorithm>
#include <random>

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    // LastByteSent - LastByteAcked <= rwnd <=> NextByteToSend - ACK (next byte expected to receive) <= rwnd
    while (_next_seqno - _ackno <= _rwnd && !_fin_sent) {
        const size_t segment_size = min(TCPConfig::MAX_PAYLOAD_SIZE, _rwnd - _next_seqno + _ackno);
        TCPSegment segment_to_send;
        if (!_syn_sent) {
            segment_to_send.header().syn = true;
            _syn_sent = true;
        }
        segment_to_send.payload() = Buffer(_stream.read(segment_size));
        segment_to_send.header().seqno = wrap(_next_seqno, _isn);
        if (_stream.eof() && _next_seqno + segment_to_send.length_in_sequence_space() + 1 - _ackno <= _rwnd) {
            segment_to_send.header().fin = true;
            _fin_sent = true;
        }
        if (segment_to_send.length_in_sequence_space() == 0) {
            break;
        }
        if (_outstanding_segments.empty())
            _timer.start_timer();
        _segments_out.push(segment_to_send);
        _outstanding_segments.push(segment_to_send);
        _next_seqno += segment_to_send.length_in_sequence_space();
        _bytes_in_flight += segment_to_send.length_in_sequence_space();
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    if (window_size > 0) {
        _rwnd = window_size;
        _rwnd_zero = false;
    } else {  //! If window size is 0, act like window size is 1.
        _rwnd = 1;
        _rwnd_zero = true;
    }
    const uint64_t new_ackno = unwrap(ackno, _isn, _ackno);
    if (new_ackno > _ackno && new_ackno <= _next_seqno) {
        _ackno = new_ackno;
        while (!_outstanding_segments.empty()) {
            const TCPSegment segment_to_retransmit = _outstanding_segments.front();
            const size_t segment_length = segment_to_retransmit.length_in_sequence_space();
            const uint64_t absolute_seqno = unwrap(segment_to_retransmit.header().seqno, _isn, _ackno);

            // Treat each segment as fully outstanding until it’s been fully acknowledged
            if (absolute_seqno + segment_length > _ackno)
                break;
            _outstanding_segments.pop();
            _bytes_in_flight -= segment_length;
            _timer.restart_timer_with_initial_rto(_initial_retransmission_timeout);
        }
        _consecutive_retransmissions = 0;
        fill_window();
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (_timer.timeout(ms_since_last_tick)) {
        if (!_outstanding_segments.empty()) {
            _segments_out.push(_outstanding_segments.front());
            _timer.double_rto(_rwnd_zero);
            _consecutive_retransmissions++;
            _timer.start_timer();
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment empty_segment_to_send;
    empty_segment_to_send.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(empty_segment_to_send);
}
