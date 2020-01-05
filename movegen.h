#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "chessTypes.h"
#include "bitboard.h"
#include "move.h"

#include <cstdint>

// === movegen.h ===
// Contains functions pertaining to move generation from a given position.
//
// Definitions of relevant terms:
//
// "Legal moves" are those which follow all the rules of chess.
// "Valid moves" meet most of the requirements of legality, except that one
// can leave one's own royalty (kings, for normal chess) under attack.
// "(Valid) attacks" are valid moves with the additional relaxation that the
// target square may be occupied by a friendly piece.
// "Invalid moves" are all other moves (e.g. moved piece doesn't exist, movement
// makes no sense, attempting to move an enemy piece, castling without meeting
// all the criteria, promotion to enemy knight...)

class Position;

Movelist generateLegalMoves(Position& pos);
bool isInCheck(Colour co, const Position& pos);
bool isLegal(Move mv, Position& pos);

uint64_t perft(int depth, Position& pos);

// === Functions to generate particular types of valid moves ===
Movelist& addKingMoves(Movelist& mvlist, Colour co, const Position& pos);
Movelist& addKnightMoves(Movelist& mvlist, Colour co, const Position& pos);
Movelist& addBishopMoves(Movelist& mvlist, Colour co, const Position& pos);
Movelist& addRookMoves(Movelist& mvlist, Colour co, const Position& pos);
Movelist& addQueenMoves(Movelist& mvlist, Colour co, const Position& pos);

Movelist& addPawnAttacks(Movelist& mvlist, Colour co, const Position& pos);
Movelist& addPawnMoves(Movelist& mvlist, Colour co, const Position& pos);
Movelist& addEpMoves(Movelist& mvlist, Colour co, const Position& pos);

bool isCastlingValid(CastlingRights cr, const Position& pos);
Movelist& addCastlingMoves(Movelist& mvlist, Colour co, const Position& pos);

// === Useful auxiliary functions ===
Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos);
Bitboard attacksTo(Square sq, Colour co, const Position& pos);
bool isAttacked(Square sq, Colour co, const Position& pos);

#endif //#ifndef MOVEGEN_INCLUDED