#include "tcp_connection.hh"

#include <iostream>
#include <algorithm>
#include <limits>        
#include <cstdint> 

using namespace std;

void TCPConnection::_tcp_send_segment() {
    while(!_sender.segments_out().empty()){
        TCPSegment seg_to_send = _sender.segments_out().front();
        if (const bool ack = _receiver.ackno().has_value()){
            seg_to_send.header().win = min(_receiver.window_size(), static_cast<size_t>(numeric_limits<uint16_t>::max()));
            seg_to_send.header().ack = ack;
            seg_to_send.header().ackno = _receiver.ackno().value();
        }
        _segments_out.push(seg_to_send);
        _sender.segments_out().pop();
    }
}

void TCPConnection::_deal_with_rst (const bool received) {
    if (!received) {
        _sender.send_empty_segment();
        TCPSegment rst_seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        rst_seg.header().rst = true;
        _segments_out.push(rst_seg);
    }
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _linger_after_streams_finish = false;
    _active = false;
}

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _time_since_last_segment_received = 0;
    if (seg.header().rst)
        _deal_with_rst(true);
    else if (seg.header().ack && _sender.next_seqno_absolute() == 0) {}
    else {
        _receiver.segment_received(seg);
        if (seg.header().ack)
            _sender.ack_received(seg.header().ackno, seg.header().win);
        if (seg.length_in_sequence_space() > 0){
            _sender.fill_window();
            if (_sender.segments_out().empty())
                _sender.send_empty_segment();
        }
        if (_receiver.ackno().has_value() && seg.length_in_sequence_space() == 0
            && seg.header().seqno == _receiver.ackno().value() - 1)  
            _sender.send_empty_segment(); 
        _tcp_send_segment();
        if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof())
            _linger_after_streams_finish = false;
    }
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    const size_t bytes_written = _sender.stream_in().write(data);
    _sender.fill_window();
    _tcp_send_segment();
    return bytes_written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) 
        _deal_with_rst(false);
    else {
        _tcp_send_segment();
        if (_receiver.stream_out().input_ended() && _sender.stream_in().eof() && _sender.bytes_in_flight() == 0) {
            if (!_linger_after_streams_finish || _time_since_last_segment_received >= 10 * _cfg.rt_timeout)
                _active = false;
        }
    }
}       

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    _tcp_send_segment();
}

void TCPConnection::connect() {
    _sender.fill_window();
    _tcp_send_segment();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            _deal_with_rst(false);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
