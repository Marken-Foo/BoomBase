#ifndef BITBOARD_INCLUDED
#define BITBOARD_INCLUDED

#include "chess_types.h"

#include <cstdint>
#include <string>
#include <array>

// === bitboard.h ===
// Essentially lots of global variables and methods involving bitboards.
// Bitboards are a useful technique to represent a chess position; a 64-bit
// integer (uint64_t) is used here, corresponding nicely to an 8x8 chessboard.

typedef uint64_t Bitboard;

// Returns a string visualisation of a bitboard; useful to print and debug
const std::string pretty(Bitboard bb);

// === Useful constants ===
constexpr Bitboard BB_ALL = ~Bitboard{0};
constexpr Bitboard BB_NONE = Bitboard{0};

constexpr Bitboard BB_A {0x0101010101010101ULL};
constexpr Bitboard BB_B {BB_A << 1};
constexpr Bitboard BB_C {BB_A << 2};
constexpr Bitboard BB_D {BB_A << 3};
constexpr Bitboard BB_E {BB_A << 4};
constexpr Bitboard BB_F {BB_A << 5};
constexpr Bitboard BB_G {BB_A << 6};
constexpr Bitboard BB_H {BB_A << 7};

constexpr Bitboard BB_1 {0xFFULL};
constexpr Bitboard BB_2 {BB_1 << (8*1)};
constexpr Bitboard BB_3 {BB_1 << (8*2)};
constexpr Bitboard BB_4 {BB_1 << (8*3)};
constexpr Bitboard BB_5 {BB_1 << (8*4)};
constexpr Bitboard BB_6 {BB_1 << (8*5)};
constexpr Bitboard BB_7 {BB_1 << (8*6)};
constexpr Bitboard BB_8 {BB_1 << (8*7)};

constexpr Bitboard BB_LONG_DIAG {0x8040201008040201ULL};
constexpr Bitboard BB_LONG_ANTIDIAG {0x0102040810204080ULL};

constexpr std::array<Bitboard, NUM_COLOURS> BB_OUR_2 {BB_2, BB_7};
constexpr std::array<Bitboard, NUM_COLOURS> BB_OUR_4 {BB_4, BB_5};
constexpr std::array<Bitboard, NUM_COLOURS> BB_OUR_8 {BB_8, BB_1};

// Square-to-Bitboard conversion.
inline Bitboard bbFromSq(Square sq) {uint64_t x = 1; return (x << sq);}


// Bitscan operations -- relies on x64 processor instructions
#ifdef __GNUC__ // e.g. GCC compiler
// Clears and returns least significant bit of Bitboard as a Square.
// Undefined if bitboard is zero.
inline Square popLsb(Bitboard& bb) {
    const Square sq {square(__builtin_ctzll(bb))};
    bb &= bb - 1;
    return sq;
}
// Read and return least/greatest significant bits of Bitboard as a Square.
// Undefined if bitboard is zero.
inline Square lsb(Bitboard bb) {return square(__builtin_ctzll(bb));}
inline Square gsb(Bitboard bb) {return square(63 ^ __builtin_clzll(bb));}
#endif //ifdef GCC compiler


// === Bitboard logic ===
inline Bitboard operator&(Bitboard bb, Square sq) {return bb & bbFromSq(sq);}
inline Bitboard operator|(Bitboard bb, Square sq) {return bb | bbFromSq(sq);}
inline Bitboard operator^(Bitboard bb, Square sq) {return bb ^ bbFromSq(sq);}
inline Bitboard operator&(Square sq, Bitboard bb) {return bb & bbFromSq(sq);}
inline Bitboard operator|(Square sq, Bitboard bb) {return bb | bbFromSq(sq);}
inline Bitboard operator^(Square sq, Bitboard bb) {return bb ^ bbFromSq(sq);}
inline Bitboard& operator&=(Bitboard& bb, Square sq) {return bb &= bbFromSq(sq);}
inline Bitboard& operator|=(Bitboard& bb, Square sq) {return bb |= bbFromSq(sq);}
inline Bitboard& operator^=(Bitboard& bb, Square sq) {return bb ^= bbFromSq(sq);}

inline Bitboard operator&(Square sq1, Square sq2) {return bbFromSq(sq1) & bbFromSq(sq2);}
inline Bitboard operator|(Square sq1, Square sq2) {return bbFromSq(sq1) | bbFromSq(sq2);}
inline Bitboard operator^(Square sq1, Square sq2) {return bbFromSq(sq1) ^ bbFromSq(sq2);}


// === Bitboard shifting ===
inline Bitboard shiftN(Bitboard bb) {return bb << 8;}
inline Bitboard shiftS(Bitboard bb) {return bb >> 8;}
inline Bitboard shiftE(Bitboard bb) {return (bb << 1) & ~BB_A;}
inline Bitboard shiftW(Bitboard bb) {return (bb >> 1) & ~BB_H;}
inline Bitboard shiftNE(Bitboard bb) {return (bb << 9) & ~BB_A;}
inline Bitboard shiftNW(Bitboard bb) {return (bb << 7) & ~BB_H;}
inline Bitboard shiftSE(Bitboard bb) {return (bb >> 7) & ~BB_A;}
inline Bitboard shiftSW(Bitboard bb) {return (bb >> 9) & ~BB_H;}

#endif //#ifndef BITBOARD_INCLUDED