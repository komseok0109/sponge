#include "wrapping_integers.hh"

using namespace std;

constexpr uint64_t ALIGN = 1ULL << 32;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return isn + n;  // (isn + n) mod 2^32
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    // Calculate offset and align the checkpoint to the smaller neares multiple of ALIGN
    const uint64_t offset = static_cast<uint32_t>(n - isn);
    const uint64_t aligned_checkpoint = (checkpoint / ALIGN) << 32;

    //<! The "middle" candidate absolute seqno based on the aligned checkpoint and the offset
    const uint64_t mid = aligned_checkpoint + offset;
    if (mid < checkpoint) {
        //<! The "high" candidate absolute seqno by adding ALIGN to the middle value
        const uint64_t high = aligned_checkpoint + offset + ALIGN;

        // Check if the "high" is closer to the checkpoint, or if "mid" should be chosen
        return checkpoint < (ALIGN - 1) * ALIGN && high - checkpoint <= checkpoint - mid ? high : mid;
    } else {
        //<! The "low" candidate absolute seqno by adding ALIGN to the middle value
        const uint64_t low = aligned_checkpoint + offset - ALIGN;

        // Check if the "low" is closer to the checkpoint, or if "mid" should be chosen
        return checkpoint >= ALIGN && mid - checkpoint >= checkpoint - low ? low : mid;
    }
}
