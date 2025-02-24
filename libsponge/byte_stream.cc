#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    const size_t bytes_to_write = min(data.size(), remaining_capacity());
    bytestream.insert(bytestream.end(), data.begin(), data.begin() + bytes_to_write);
    _bytes_written += bytes_to_write;
    return bytes_to_write;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    const size_t bytes_to_peek = min(len, buffer_size());
    return string(bytestream.begin(), bytestream.begin() + bytes_to_peek);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    const size_t bytes_to_pop = min(len, buffer_size());
    bytestream.erase(bytestream.begin(), bytestream.begin() + bytes_to_pop);
    _bytes_read += bytes_to_pop;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    const string data = peek_output(len);
    pop_output(len);
    return data;
}

void ByteStream::end_input() { _input_ended = true; }

bool ByteStream::input_ended() const { return _input_ended; }

size_t ByteStream::buffer_size() const { return _bytes_written - _bytes_read; }

bool ByteStream::buffer_empty() const { return buffer_size() == 0; }

bool ByteStream::eof() const { return _input_ended && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }
