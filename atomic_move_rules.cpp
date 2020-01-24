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
    // In atomic, one side can have no king (variant game ending) -- lsb() is
    // undefined in that case. We need to check for this.
    if (bb == BB_NONE) {
        return false;
    }
    Square sq {lsb(bb)}; // assumes exactly one king per side.
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
bool AtomicMoveRules::isLegalNaive(Move mv, Position& pos) {
    /// Naive method of testing move for legality. (Assumes move is valid.)
    /// Uses makeMove/unmakeMove for reliability at the cost of performance.
    Colour co {pos.getSideToMove()};
    bool isOk {false};
    
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

bool AtomicMoveRules::isLegal(Move mv, Position& pos) {
    /// Tests valid moves for legality. (Assumes move is valid.)
    /// Illegal moves in atomic include exploding one's own king, and leaving
    /// one's king in check while the opponent's is still on the board.
    
    // A move can be illegal only if it is a king move, capture/ep, or move of a
    // pinned piece while kings are not connected.
    // (In atomic, captures can explode multiple pieces on a line to open up a
    // checking ray.)
    // So if kings are connected, only king moves/captures can be illegal.
    // If kings are not connected, we need to check for pins too.
    const Colour co {pos.getSideToMove()};
    bool isOk {false};
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    const Bitboard bbKing {pos.getUnitsBb(co, KING)};
    const Bitboard bbEnemyKing {pos.getUnitsBb(!co, KING)};
    const Bitboard bbAll {pos.getUnitsBb()};
    const Square kingSq {lsb(bbKing)};
    // Note: checking pieces don't actually give check if kings are connected.
    const Bitboard bbCheckers {attacksTo(kingSq, !co, pos)};
    const bool isConnectedKings {bbEnemyKing & atomicMasks[kingSq]};
    
    // No moves are legal from a terminated game.
    if (pos.isVariantEnd()) {
        return false;
    }
    // If en passant or castling, just treat them naively.
    if (isEp(mv) || isCastling(mv)) {
        return isLegalNaive(mv, pos);
    }
    
    // Always check king moves. Illegal if stepping into check or capturing.
    // Castling not handled by this.
    if (getPieceType(pos.getMailbox(fromSq)) == KING) {
        if (pos.getMailbox(toSq) != NO_PIECE) {
            return false;
        } else {
            pos.ghostKing(co, fromSq);
            isOk = !isCheckAttacked(toSq, !co, pos);
            pos.unghostKing(co, fromSq);
            return isOk;
        }
    }
    // Always check captures. Illegal if explodes own king, else legal if
    // explodes enemy king, else legal if kings are adjacent, else see if king
    // is in check after. En passant not handled by this.
    if (pos.getMailbox(toSq) != NO_PIECE) {
        if (bbKing & atomicMasks[toSq]) {
            return false;
        } else if (bbEnemyKing & atomicMasks[toSq]) {
            return true;
        } else {
            if (isConnectedKings) {
                return true;
            }
            Bitboard bbExploded = atomicMasks[toSq]
                                  & (bbAll & ~pos.getUnitsBb(PAWN));
            bbExploded |= toSq;
            Bitboard bb = bbAll & ~(bbExploded | fromSq);
            // bb now represents the occupancy bitboard if the move were made.
            // Assumes this not a king move.
            if (bb & knightAttacks[kingSq] & pos.getUnitsBb(!co, KNIGHT)) {
                return false;
            }
            // kingSq is attacked by an enemy pawn if an own pawn on kingSq
            // would attack that enemy pawn.
            if (bb & pawnAttacks[co][kingSq] & pos.getUnitsBb(!co, PAWN)) {
                return false;
            }
            // Checks line pieces for check.
            Bitboard bbR {findRookAttacks(kingSq, bb)};
            if (bb & bbR & (pos.getUnitsBb(!co, ROOK) |
                            pos.getUnitsBb(!co, QUEEN))) {
                return false;
            }
            Bitboard bbB {findBishopAttacks(kingSq, bb)};
            if (bb & bbB & (pos.getUnitsBb(!co, BISHOP) |
                            pos.getUnitsBb(!co, QUEEN))) {
                return false;
            }
            return true;
        }
    }
    // If kings are connected, then all non-capture non-king moves are fine.
    // Assumes this is not castling and not en passant.
    if (isConnectedKings) {
        return true;
    }
    // If kings are not connected, then check if piece is pinned.
    // Checkers are then also real checkers.
    Bitboard bbOrthoPinners {findRookAttacks(kingSq, bbKing) & (pos.getUnitsBb(!co, ROOK) | pos.getUnitsBb(!co, QUEEN))};
    Bitboard bbDiagPinners {findBishopAttacks(kingSq, bbKing) & (pos.getUnitsBb(!co, BISHOP) | pos.getUnitsBb(!co, QUEEN))};
    Bitboard bbPinners {bbOrthoPinners | bbDiagPinners};
    Bitboard bbPinned {BB_NONE};
    while (bbPinners) {
        Square pinner {popLsb(bbPinners)};
        Bitboard ray {lineBetween[pinner][kingSq]};
        // if ray has only one entry of my colour, that is pinned piece.
        if (isSingle(ray & bbAll)) {
            bbPinned |= (ray & bbAll);
        }
    }
    if (fromSq & bbPinned) {
        if (lineBetween[fromSq][toSq] & kingSq || lineBetween[fromSq][kingSq] & toSq || lineBetween[toSq][kingSq] & fromSq) {
            if (bbCheckers) {
                return false;
            } else {
                return true;
            }
        } else {
            return false;
        }
    }
    
    // Fallback: use the reliable naive method.
    return isLegalNaive(mv, pos);
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
        bbAttacked = findBishopAttacks(sq, bbAll);
        break;
    case ROOK:
        bbAttacked = findRookAttacks(sq, bbAll);
        break;
    case QUEEN:
        bbAttacked = findRookAttacks(sq, bbAll) | findBishopAttacks(sq, bbAll);
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
    bbAttackers |= findBishopAttacks(sq, pos.getUnitsBb())
                   & (pos.getUnitsBb(co, BISHOP) | pos.getUnitsBb(co, QUEEN));
    bbAttackers |= findRookAttacks(sq, pos.getUnitsBb())
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