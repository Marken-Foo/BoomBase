#ifndef BITBOARD_LOOKUP_INCLUDED
#define BITBOARD_LOOKUP_INCLUDED

#include "bitboard.h"
#include "chess_types.h"

#include <array>

// === bitboard_lookup.h ===
// Contains Bitboard lookup tables (for move generation) and functions to
// generate them.
// Also contains functions to get slider attacks in particular directions.

// Initialise tables. Must be called at least once before doing any lookup.
void initialiseBbLookup();

Bitboard findRankAttacks(Square sq, Bitboard bbPos);
Bitboard findDiagAttacks(Square sq, Bitboard bbPos);
Bitboard findAntidiagAttacks(Square sq, Bitboard bbPos);
Bitboard findFileAttacks(Square sq, Bitboard bbPos);

Bitboard findRookAttacks(Square sq, Bitboard bbPos);
Bitboard findBishopAttacks(Square sq, Bitboard bbPos);

// === Lookup tables ===
// **Must be generated before doing any lookup!**

// Indexed by square on the chessboard.
extern std::array<Bitboard, NUM_SQUARES> knightAttacks;
extern std::array<Bitboard, NUM_SQUARES> kingAttacks;
// Pawn attacks depend on colour.
extern std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> pawnAttacks;

// Indexed by square on the chessboard. Contains the Bitboard of the
// corresponding (anti)diagonal passing through that square.
extern std::array<Bitboard, NUM_SQUARES> diagMasks;
extern std::array<Bitboard, NUM_SQUARES> antidiagMasks;

// Lookup for bitboard of squares between (exclusive) two endpoint squares.
// Bitboard is zero if squares are not along the same rank, file, or diagonal.
extern std::array<std::array<Bitboard, NUM_SQUARES>, NUM_SQUARES> lineBetween;

#endif //#ifndef BITBOARD_LOOKUP_INCLUDED