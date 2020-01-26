#include "atomic_move_rules.h"

#include "atomic_capture_masks.h"
#include "bitboard.h"
#include "bitboard_lookup.h"
#include "chess_types.h"
#include "move.h"
#include "position.h"


bool AtomicMoveRules::isLegal(Move mv, Position& pos) {
    /// Tests valid moves for legality. (Assumes move is valid.)
    /// Illegal moves in atomic include exploding one's own king, and leaving
    /// one's king in check while the opponent's is still on the board.
    
    // No moves are legal from a terminated game.
    if (pos.isVariantEnd()) {
        return false;
    }
    // If en passant or castling, just treat them naively.
    if (isEp(mv) || isCastling(mv)) {
        return isLegalNaive(mv, pos);
    }
    const Colour co {pos.getSideToMove()};
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    
    // Always check king moves. Illegal if stepping into check or capturing.
    // Castling not handled by this.
    if (getPieceType(pos.getMailbox(fromSq)) == KING) {
        if (pos.getMailbox(toSq) != NO_PIECE) {
            return false;
        } else {
            pos.ghostKing(co, fromSq);
            const bool isOk = !isCheckAttacked(toSq, !co, pos);
            pos.unghostKing(co, fromSq);
            return isOk;
        }
    }
    // Always check captures. En passant not handled by this.
    if (pos.getMailbox(toSq) != NO_PIECE) {
        return isCaptureLegal(fromSq, toSq, pos);
    }
    // If not a king move and not a capture (and not ep/castling), verify.
    return isLegalNonKingNonCapture(fromSq, toSq, pos);
}

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

Movelist AtomicMoveRules::generateLegalMoves(Position& pos) {
    return generateLegalMovesByType(pos);
}

bool AtomicMoveRules::isLegalNaive(Move mv, Position& pos) {
    /// Naive method of testing move for legality. (Assumes move is valid.)
    /// Uses makeMove/unmakeMove for reliability.
    const Colour co {pos.getSideToMove()};
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

Movelist AtomicMoveRules::generateLegalMovesNaive(Position& pos) {
    /// Generates all legal moves in the position naively: first generates all
    /// valid moves, then checks each for legality.
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

Movelist AtomicMoveRules::generateLegalMovesByType(Position& pos) {
    /// Directly generates all legal moves in the position by piece type.
    ///
    Colour co {pos.getSideToMove()};
    Movelist mvlist {};
    if (pos.isVariantEnd()) {
        return mvlist;
    }
    // Start generating valid moves -- begin with ep and castling.
    addEpMoves(mvlist, co, pos);
    addCastlingMoves(mvlist, co, pos);
    // Test for legality naively for these.
    for (auto it = mvlist.begin(); it != mvlist.end();) {
        if (isLegalNaive(*it, pos)) {
            ++it;
        } else {
            it = mvlist.erase(it);
        }
    }
    // Generating by piece, rather than by capture/quiet/etc, since this will be
    // useful for PGN validation, which gives a known piece type for each move.
    addLegalKingMoves(mvlist, pos);
    addLegalKnightMoves(mvlist, pos);
    addLegalSliderMoves(mvlist, pos, BISHOP);
    addLegalSliderMoves(mvlist, pos, ROOK);
    addLegalSliderMoves(mvlist, pos, QUEEN);
    addLegalPawnMoves(mvlist, pos);
    return mvlist;
}

Movelist& AtomicMoveRules::addLegalKingMoves(Movelist& mvlist, Position& pos) {
    const Colour co {pos.getSideToMove()};
    Bitboard bbFrom = pos.getUnitsBb(co, KING);
    const Bitboard bbAll = pos.getUnitsBb();
    // Verify that king's destination square is not occupied (kings can't
    // capture) and not check-attacked (can be attacked if enemy king is
    // adjacent). Ghost/unghost king for the latter.
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        pos.ghostKing(co, fromSq);
        Bitboard bbTo {kingAttacks[fromSq] & ~bbAll};
        while (bbTo) {
            Square toSq {popLsb(bbTo)};
            if (!isCheckAttacked(toSq, !co, pos)) {
                mvlist.push_back(buildMove(fromSq, toSq));
            }
        }
        pos.unghostKing(co, fromSq);
    }
    
    return mvlist;
}

Movelist& AtomicMoveRules::addLegalSliderMoves(Movelist& mvlist, Position& pos,
                                               PieceType pcty) {
    /// Among normal pieces, the rook, bishop, and queen are sliders.
    ///
    const Colour co {pos.getSideToMove()};
    Bitboard bbFrom {pos.getUnitsBb(co, pcty)};
    const Bitboard bbFriendly {pos.getUnitsBb(co)};
    const Bitboard bbAll {pos.getUnitsBb()};
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        Bitboard bbTo {BB_NONE};
        if (pcty == BISHOP || pcty == QUEEN) {
            bbTo |= findBishopAttacks(fromSq, bbAll);
        }
        if (pcty == ROOK || pcty == QUEEN) {
            bbTo |= findRookAttacks(fromSq, bbAll);
        }
        bbTo &= ~bbFriendly;
        while (bbTo) {
            Square toSq {popLsb(bbTo)};
            if (pos.getMailbox(toSq) != NO_PIECE) {
                if (isCaptureLegal(fromSq, toSq, pos)) {
                    mvlist.push_back(buildMove(fromSq, toSq));
                }
                continue;
            }
            if (isLegalNonKingNonCapture(fromSq, toSq, pos)) {
                mvlist.push_back(buildMove(fromSq, toSq));
            }
        }
    }
    return mvlist;
}

Movelist& AtomicMoveRules::addLegalKnightMoves(Movelist& mvlist,
                                               Position& pos) {
    const Colour co {pos.getSideToMove()};
    Bitboard bbFrom {pos.getUnitsBb(co, KNIGHT)};
    const Bitboard bbFriendly {pos.getUnitsBb(co)};
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        // TODO: separate to 2 bbTo bitboards -- one for captures, one for not.
        Bitboard bbTo {knightAttacks[fromSq] & ~bbFriendly};
        while (bbTo) {
            Square toSq {popLsb(bbTo)};
            if (pos.getMailbox(toSq) != NO_PIECE) {
                if (isCaptureLegal(fromSq, toSq, pos)) {
                    mvlist.push_back(buildMove(fromSq, toSq));
                }
                continue;
            }
            if (isConnectedKings(pos)) {
                mvlist.push_back(buildMove(fromSq, toSq));
                continue;
            }
            const Square kingSq {lsb(pos.getUnitsBb(co, KING))};
            const Bitboard bbCheckers {attacksTo(kingSq, !co, pos)};
            if (bbCheckers) {
                if (isInterpositionLegal(fromSq, toSq, pos)) {
                    mvlist.push_back(buildMove(fromSq, toSq));
                }
                continue;
            }
            const Bitboard bbPinned {IMoveRules::findPinned(co, pos)};
            if (fromSq & bbPinned) {
                // Knights cannot move along a pin ray.
                // (unless there are nightriders...)
                continue;
            }
            mvlist.push_back(buildMove(fromSq, toSq));
        }
    }
    return mvlist;
}

Movelist& AtomicMoveRules::addLegalPawnMoves(Movelist& mvlist, Position& pos) {
    /// Adds legal pawn pushes, captures and promotions. Does not generate en
    /// passant captures.
    addLegalPawnCaptures(mvlist, pos);
    addLegalPawnPushes(mvlist, pos);
    addLegalPawnDoublePushes(mvlist, pos);
    return mvlist;
}

Movelist& AtomicMoveRules::addLegalPawnCaptures(Movelist& mvlist,
                                                Position& pos) {
    /// Adds legal pawn captures (including capture-promotions). Does not
    /// generate en passant captures.
    const Colour co {pos.getSideToMove()};
    Bitboard bbFrom {pos.getUnitsBb(co, PAWN)};
    const Bitboard bbEnemy {pos.getUnitsBb(!co)};
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        Bitboard bbTo {pawnAttacks[co][fromSq] & bbEnemy};
        while (bbTo) {
            Square toSq {popLsb(bbTo)};
            if (isCaptureLegal(fromSq, toSq, pos)) {
                IMoveRules::addPawnMoves(mvlist, co, fromSq, toSq);
            }
        }
    }
    return mvlist;
}

Movelist& AtomicMoveRules::addLegalPawnPushes(Movelist& mvlist, Position& pos) {
    const Colour co {pos.getSideToMove()};
    Bitboard bbFrom {pos.getUnitsBb(co, PAWN)};
    Bitboard bbAll {pos.getUnitsBb()};
    
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        Square toSq {shiftForward(fromSq, co)};
        if (toSq & bbAll) {
            // pawn push is blocked
            continue;
        }
        if (isLegalNonKingNonCapture(fromSq, toSq, pos)) {
            IMoveRules::addPawnMoves(mvlist, co, fromSq, toSq);
        }
    }
    return mvlist;
}

Movelist& AtomicMoveRules::addLegalPawnDoublePushes(Movelist& mvlist,
                                                    Position& pos) {
    const Colour co {pos.getSideToMove()};
    Bitboard bbFrom {pos.getUnitsBb(co, PAWN) & BB_OUR_2[co]};
    Bitboard bbAll {pos.getUnitsBb()};
    
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        Square toSq {shiftForward(shiftForward(fromSq, co), co)};
        if ((shiftForward(fromSq, co) | toSq) & bbAll) {
            // pawn push is blocked
            continue;
        }
        if (isLegalNonKingNonCapture(fromSq, toSq, pos)) {
            mvlist.push_back(buildMove(fromSq, toSq));
        }
    }
    return mvlist;
}

Bitboard AtomicMoveRules::attacksTo(Square sq, Colour co, const Position& pos) {
    /// Returns bitboard of units of a given colour that attack a given square.
    /// In atomic chess, kings cannot capture, so kings do not attack either.
    
    // In chess, most piece types have the property that: if piece PC is on
    // square SQ_A attacking SQ_B, then from SQ_B it would attack SQ_A.
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

bool AtomicMoveRules::isCaptureLegal(Square fromSq, Square toSq,
                                     const Position& pos) {
    /// Assumes is not a king move nor en passant.
    ///
    // When is a capture legal/illegal?
    // Illegal if it explodes one's own king,
    // else legal if it explodes the enemy king,
    // else legal if kings are adjacent
    // else it depends if one's own king is in check thereafter.
    // En passant is not handled by this.
    const Colour co {pos.getSideToMove()};
    const Bitboard bbKing {pos.getUnitsBb(co, KING)};
    const Bitboard bbEnemyKing {pos.getUnitsBb(!co, KING)};
    const Square kingSq {lsb(bbKing)};
    const bool isConnectedKings {bbEnemyKing & atomicMasks[kingSq]};
    // Check if move is atomic suicide, atomic win, or if kings remain adjacent.
    if (bbKing & atomicMasks[toSq]) {
        return false;
    } else if (bbEnemyKing & atomicMasks[toSq]) {
        return true;
    } else if (isConnectedKings) {
        return true;
    }
    // Checks are real only if kings are not connected.
    // Need to see if king is in check after explosion.
    const Bitboard bbCheckers {attacksTo(kingSq, !co, pos)};
    const Bitboard bbAll {pos.getUnitsBb()};
    Bitboard bbExploded = (atomicMasks[toSq]
                           & (bbAll & ~pos.getUnitsBb(PAWN)))
                          | toSq;
    // Since kings are not connected, checks are real.
    // If in check already, explosion must destroy all checkers.
    if ((bbCheckers & bbExploded) != bbCheckers) {
        return false;
    }
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
    if (bb & findRookAttacks(kingSq, bb) &
        (pos.getUnitsBb(!co, ROOK) | pos.getUnitsBb(!co, QUEEN))) {
        return false;
    }
    if (bb & findBishopAttacks(kingSq, bb) &
        (pos.getUnitsBb(!co, BISHOP) | pos.getUnitsBb(!co, QUEEN))) {
        return false;
    }
    return true;
}

bool AtomicMoveRules::isLegalNonKingNonCapture(Square fromSq, Square toSq,
                                               const Position& pos) {
    /// Returns if the (valid) move described by from/to squares is legal.
    /// Assumes the move is not a king move or a capture (also not en passant
    /// nor castling), and additionally that the move described by the from/to
    /// squares is valid.
    const Colour co {pos.getSideToMove()};
    // If kings are connected, then all non-capture non-king moves are fine.
    // Assumes this is not castling and not en passant.
    if (isConnectedKings(pos)) {
        return true;
    } 
    // If kings are not connected, checkers are real; need to address check.
    // Note: checking pieces don't actually give check if kings are connected.
    const Square kingSq {lsb(pos.getUnitsBb(co, KING))};
    const Bitboard bbCheckers {attacksTo(kingSq, !co, pos)};
    // Captures and king moves already handled, so if in check, just need to
    // look for legal interpositions.
    if (bbCheckers) {
        return isInterpositionLegal(fromSq, toSq, pos);
    }
    const Bitboard bbPinned {IMoveRules::findPinned(co, pos)};
    // Finally check for pinned pieces, which can only move along the pin line.
    if (fromSq & bbPinned) {
        return (lineBetween[fromSq][kingSq] & toSq) ||
               (lineBetween[toSq][kingSq] & fromSq);
    }
    // Any move that avoids all the above criteria is legal.
    return true;
}

bool AtomicMoveRules::isConnectedKings(const Position& pos) {
    // Assumes there is exactly one king per side.
    return pos.getUnitsBb(WHITE, KING) &
           atomicMasks[lsb(pos.getUnitsBb(BLACK, KING))];
}

bool AtomicMoveRules::isInterpositionLegal(Square fromSq, Square toSq,
                                           const Position& pos) {
    /// True iff the from/to squares describe a legal interposition move.
    /// Assumes the side to move is in check.
    const Colour co {pos.getSideToMove()};
    const Square kingSq {lsb(pos.getUnitsBb(co, KING))};
    const Bitboard bbCheckers {attacksTo(kingSq, !co, pos)}; // assumed nonempty
    const Bitboard bbPinned {IMoveRules::findPinned(co, pos)};
    
    if (!isSingle(bbCheckers)) {
        return false;
    } else {
        Square checkerSq {lsb(bbCheckers)};
        PieceType checkerType {getPieceType(pos.getMailbox(checkerSq))};
        // Cannot interpose a contact check.
        if (checkerType == PAWN || checkerType == KNIGHT) {
            return false;
        } else {
            // Verify interposition for line pieces.
            return ((fromSq & bbPinned) == BB_NONE) &&
                   ((toSq & lineBetween[checkerSq][kingSq]) != BB_NONE);
        }
    }
}