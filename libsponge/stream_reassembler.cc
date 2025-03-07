#include "stream_reassembler.hh"
#include <algorithm>

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity), _bytes_to_reassemble(capacity, '\0'), _indicators(capacity, false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    const size_t first_unassembled = _output.bytes_written();
    const size_t first_unacceptable = first_unassembled + _output.remaining_capacity();
    if (eof && index + data.size() <= first_unacceptable) 
        _eof = eof;
    if (index < first_unacceptable && index + data.size() > first_unassembled) {
        const size_t start = max(index, first_unassembled);
        const size_t end = min(index + data.size(), first_unacceptable);
        const string acceptable_bytes = data.substr(start - index, end - start);       
        for (size_t i = 0; i < acceptable_bytes.size(); i++) {
            if (!_indicators[start - first_unassembled + i]){
                _bytes_to_reassemble[start - first_unassembled + i] = acceptable_bytes[i];
                _indicators[start - first_unassembled + i] = true;
                _unassembled_bytes++;
            }
        } 
        const size_t bytes_to_assemble = distance(_indicators.begin(), find(_indicators.begin(), _indicators.end(), false));
        _output.write(string(_bytes_to_reassemble.begin(), _bytes_to_reassemble.begin() + bytes_to_assemble));
        _bytes_to_reassemble.erase(_bytes_to_reassemble.begin(), _bytes_to_reassemble.begin() + bytes_to_assemble);
        _bytes_to_reassemble.resize(_capacity, '\0');
        _indicators.erase(_indicators.begin(), _indicators.begin() + bytes_to_assemble);
        _indicators.resize(_capacity, false);
        _unassembled_bytes -= bytes_to_assemble;
    }
    if (_eof && _unassembled_bytes == 0) 
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
