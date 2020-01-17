#include "atomic_move_rules.h"

#include "chess_types.h"
#include "move.h"
#include "bitboard.h"
#include "bitboard_lookup.h"
#include "position.h"


bool AtomicMoveRules::isInCheck(Colour co, const Position& pos) {
    /// Test if a side (colour) is in check.
    ///
    Bitboard bb {pos.getUnitsBb(co, KING)};
    // In atomic, one side can have no king (variant game ending) -- popLsb() is
    // undefined in that case. We need to check for this.
    if (bb == BB_NONE) {
        return false;
    }
    Square sq {popLsb(bb)}; // assumes exactly one king per side.
    if (isAttacked(sq, !co, pos)) { // king is attacked.
        if (kingAttacks[sq] & pos.getUnitsBb(!co, KING)) {
            return false; // connected kings, no check.
        } else {
            return true;
        }
    } else {
        return false;
    }
}


bool AtomicMoveRules::isLegal(Move mv, Position& pos) {
    /// Assumes move is valid.
    // Logic: I make a move.
    // If I have no king, illegal. (atomic suicide)
    // Else, if my opponent has no king, legal. (explosion > check)
    // Else, if I am in check, illegal. (suicide)
    // Unmake move.
    Colour co {pos.getSideToMove()};
    bool isOk {false};
    pos.makeMove(mv);
    if (pos.getUnitsBb(co, KING) == BB_NONE) {
        isOk = false;
    } else if (pos.getUnitsBb(!co, KING) == BB_NONE) {
        isOk = true;
    } else {
        isOk = !isInCheck(co, pos);
    }
    pos.unmakeMove(mv);
    return isOk;
}

Movelist AtomicMoveRules::generateLegalMoves(Position& pos) {
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
    // Test for legality.
    for (auto it = mvlist.begin(); it != mvlist.end();) {
        if (isLegal(*it, pos)) {
            ++it;
        } else {
            it = mvlist.erase(it);
        }
    }
    return mvlist;
}

Bitboard AtomicMoveRules::attacksFrom(Square sq, Colour co, PieceType pcty,
                                      const Position& pos) {
    /// Returns bitboard of squares attacked by a given piece type placed on a
    /// given square.
    Bitboard bbAttacked {BB_NONE};
    Bitboard bbAll {pos.getUnitsBb()};
    
    switch (pcty) {
    case PAWN:
        // Note: Does not check that an enemy piece is on the target square!
        bbAttacked = pawnAttacks[co][sq];
        break;
    case KNIGHT:
        bbAttacked = knightAttacks[sq];
        break;
    case BISHOP:
        bbAttacked = findDiagAttacks(sq, bbAll) | findAntidiagAttacks(sq, bbAll);
        break;
    case ROOK:
        bbAttacked = findRankAttacks(sq, bbAll) | findFileAttacks(sq, bbAll);
        break;
    case QUEEN:
        bbAttacked = findRankAttacks(sq, bbAll) | findFileAttacks(sq, bbAll) |
                     findDiagAttacks(sq, bbAll) | findAntidiagAttacks(sq, bbAll);
        break;
    case KING:
        // King does not attack in atomic.
        bbAttacked = BB_NONE;
        break;
    }
    return bbAttacked;
}

Bitboard AtomicMoveRules::attacksTo(Square sq, Colour co, const Position& pos) {
    /// Returns bitboard of units of a given colour that attack a given square.
    /// In chess, most piece types have the property that: if piece PC is on
    /// square SQ_A attacking SQ_B, then from SQ_B it would attack SQ_A.
    /// In atomic chess, kings cannot capture, so kings do not attack either.
    Bitboard bbAttackers {BB_NONE};
    bbAttackers = knightAttacks[sq] & pos.getUnitsBb(co, KNIGHT);
    bbAttackers |= (findDiagAttacks(sq, pos.getUnitsBb()) |
                    findAntidiagAttacks(sq, pos.getUnitsBb()))
                   & (pos.getUnitsBb(co, BISHOP) | pos.getUnitsBb(co, QUEEN));
    bbAttackers |= (findRankAttacks(sq, pos.getUnitsBb()) |
                    findFileAttacks(sq, pos.getUnitsBb()))
                   & (pos.getUnitsBb(co, ROOK) | pos.getUnitsBb(co, QUEEN));
    // But for pawns, a square SQ_A is attacked by a [Colour] pawn on SQ_B,
    // if a [!Colour] pawn on SQ_A would attack SQ_B.
    bbAttackers |= pawnAttacks[!co][sq] & pos.getUnitsBb(co, PAWN);
    return bbAttackers;
}

bool AtomicMoveRules::isAttacked(Square sq, Colour co, const Position& pos) {
    /// Returns if a square is attacked by pieces of a particular colour.
    /// 
    return attacksTo(sq, co, pos) != BB_NONE;
}