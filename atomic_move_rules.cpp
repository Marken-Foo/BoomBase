#include "atomic_move_rules.h"

#include "atomic_capture_masks.h"
#include "bitboard.h"
#include "bitboard_lookup.h"
#include "chess_types.h"
#include "move.h"
#include "position.h"


bool AtomicMoveRules::isInCheck(Colour co, const Position& pos) {
    /// Test if a side (colour) is in check.
    /// A king is in check if it is attacked by any enemy piece except the king.
    /// Additionally, it is *never* check if the two kings are adjacent.
    Bitboard bb {pos.getUnitsBb(co, KING)};
    // In atomic, one side can have no king (variant game ending) -- popLsb() is
    // undefined in that case. We need to check for this.
    if (bb == BB_NONE) {
        return false;
    }
    Square sq {popLsb(bb)}; // assumes exactly one king per side.
    if (isAttacked(sq, !co, pos)) { // king is attacked.
        if (atomicMasks[sq] & pos.getUnitsBb(!co, KING)) {
            // connected kings, no check.
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}


bool AtomicMoveRules::isLegal(Move mv, Position& pos) {
    /// Tests valid moves for illegality. (Assumes move is valid.)
    /// Illegal moves in atomic include exploding one's own king, and leaving
    /// one's king in check while the opponent's is still on the board.
    
    // To optimise by not checking every move with make/unmake, but by screening
    // moves that are possibly illegal and only checking those.
    // A move can be illegal only if it is a king move, capture/ep, or move of a
    // pinned piece while kings are not connected.
    // (In atomic, captures can explode multiple pieces on a line to open up a
    // checking ray.)
    // So if kings are connected, only king moves/captures can be illegal.
    // If kings are not connected, we need to check for pins too.
    
    Colour co {pos.getSideToMove()};
    bool isOk {false};
    Square fromSq {getFromSq(mv)};
    Square toSq {getToSq(mv)};
    
    // Check if position is in an end state already. (SHOULD MOVE TO POSITION.CPP)
    if (pos.getUnitsBb(co, KING) == BB_NONE || pos.getUnitsBb(!co, KING) == BB_NONE) {
        return false;
    }
    
    // Always check captures. Illegal if explodes own king, else legal if
    // explodes enemy king.
    if (pos.getMailbox(toSq) != NO_PIECE) {
        if (pos.getUnitsBb(co, KING) & atomicMasks[toSq]) {
            return false;
        } else if (pos.getUnitsBb(!co, KING) & atomicMasks[toSq]) {
            return true;
        }
    }
    
    // Always check king moves. Illegal if stepping into check or capturing.
    if (getPieceType(pos.getMailbox(fromSq)) == KING && !isCastling(mv)) {
        if (pos.getMailbox(toSq) != NO_PIECE) {
            return false;
        } else {
            pos.ghostKing(co, fromSq);
            isOk = !isCheckAttacked(toSq, !co, pos);
            pos.unghostKing(co, fromSq);
            return isOk;
        }
    }
    
    pos.makeMove(mv);
    if (pos.getUnitsBb(co, KING) == BB_NONE) {
        // exploded own king
        isOk = false;
    } else if (pos.getUnitsBb(!co, KING) == BB_NONE) {
        // exploded enemy king
        isOk = true;
    } else {
        // left own king in check?
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
        bbAttacked = findDiagAttacks(sq, bbAll)
                     | findAntidiagAttacks(sq, bbAll);
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
    // For pawns, a square SQ_A is attacked by a [Colour] pawn on SQ_B,
    // if a [!Colour] pawn on SQ_A would attack SQ_B.
    bbAttackers |= pawnAttacks[!co][sq] & pos.getUnitsBb(co, PAWN);
    return bbAttackers;
}

bool AtomicMoveRules::isAttacked(Square sq, Colour co, const Position& pos) {
    /// Returns if a square is attacked by pieces of a particular colour.
    /// 
    return attacksTo(sq, co, pos) != BB_NONE;
}

bool AtomicMoveRules::isCheckAttacked(Square sq, Colour co,
                                      const Position& pos) {
    /// Returns if an enemy king placed on that square would be in check by co.
    ///
    return !(atomicMasks[sq] & pos.getUnitsBb(co, KING))
           && attacksTo(sq, co, pos);
}