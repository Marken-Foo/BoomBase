#include "ortho_move_rules.h"

#include "chess_types.h"
#include "move.h"
#include "bitboard.h"
#include "position.h"


bool OrthoMoveRules::isInCheck(Colour co, const Position& pos) {
    // Test if a side (colour) is in check.
    Bitboard bb {pos.getUnitsBb(co, KING)};
    Square sq {popLsb(bb)}; // assumes exactly one king per side.
    return isAttacked(sq, !co, pos);
}


bool OrthoMoveRules::isLegal(Move mv, Position& pos) {
    /// Test if making a move would leave one's own royalty in check.
    /// Assumes move is valid.
    // For eventual speedup logic can be improved from naive make-unmake-make.
    Colour co {pos.getSideToMove()}; // is this fine? (why not pass as arg?)
    pos.makeMove(mv);
    bool isSuicide {isInCheck(co, pos)};
    pos.unmakeMove(mv);
    return !isSuicide;
}

Movelist OrthoMoveRules::generateLegalMoves(Position& pos) {
    Colour co {pos.getSideToMove()};
    Movelist mvlist {};
    // Start generating valid moves.
    addKingMoves(mvlist, co, pos);
    addKnightMoves(mvlist, co, pos);
    addBishopMoves(mvlist, co, pos);
    addRookMoves(mvlist, co, pos);
    addQueenMoves(mvlist, co, pos);
    addPawnMoves(mvlist, co, pos);
    addEpMoves(mvlist, co, pos);
    addCastlingMoves(mvlist, co, pos);
    // Test for checks.
    for (auto it = mvlist.begin(); it != mvlist.end();) {
        if (isLegal(*it, pos)) {
            ++it;
        } else {
            it = mvlist.erase(it);
        }
    }
    return mvlist;
}