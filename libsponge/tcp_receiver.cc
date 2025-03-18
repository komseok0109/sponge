#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &tcp_header = seg.header();

    // If this is the first SYN segment, mark SYN as received and store the Initial Sequence Number.
    if (tcp_header.syn && !_syn_received) {
        _syn_received = true;
        _isn = tcp_header.seqno;
    }

    // If SYN has been received, process the incoming data segment
    if (_syn_received) {
        // checkpoint = last byte reassembled = bytes_written() in absolute sequence number space
        // index = unwrap(seqno + syn, isn, checkpoint) - 1 (adjusted to stream index space)
        const uint64_t index = unwrap(tcp_header.seqno + tcp_header.syn, _isn, stream_out().bytes_written()) - 1;
        _reassembler.push_substring(seg.payload().copy(), index, tcp_header.fin);
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_received)
        return nullopt;
    // map 'first unassembled' to absolute sequence nubmer space (bytes_written() + SYN + FIN)
    // wrap mapped absolute sequence number
    return wrap(stream_out().bytes_written() + 1 + stream_out().input_ended(), _isn);
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
