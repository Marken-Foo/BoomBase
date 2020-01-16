#include "move_rules.h"

#include "chess_types.h"
#include "move.h"
#include "bitboard.h"
#include "bitboard_lookup.h"
#include "position.h"

#include <cstdint>
#include <memory>


Movelist& IMoveRules::addKingMoves(Movelist& mvlist, Colour co, const Position& pos) {
    Bitboard bbFrom {pos.getUnitsBb(co, KING)};
    Bitboard bbFriendly {pos.getUnitsBb(co)};
    Square fromSq {NO_SQ};
    Bitboard bbTo {BB_NONE};
    while (bbFrom) {
        fromSq = popLsb(bbFrom);
        bbTo = kingAttacks[fromSq] & ~bbFriendly;
        while (bbTo) {
            mvlist.push_back(buildMove(fromSq, popLsb(bbTo)));
        }
    }
    return mvlist;
}

Movelist& IMoveRules::addKnightMoves(Movelist& mvlist, Colour co, const Position& pos) {
    Bitboard bbFrom {pos.getUnitsBb(co, KNIGHT)};
    Bitboard bbFriendly {pos.getUnitsBb(co)};
    Square fromSq {NO_SQ};
    Bitboard bbTo {BB_NONE};
    while (bbFrom) {
        fromSq = popLsb(bbFrom);
        bbTo = knightAttacks[fromSq] & ~bbFriendly;
        while (bbTo) {
            mvlist.push_back(buildMove(fromSq, popLsb(bbTo)));
        }
    }
    return mvlist;
}

Movelist& IMoveRules::addBishopMoves(Movelist& mvlist, Colour co, const Position& pos) {
    Bitboard bbFrom {pos.getUnitsBb(co, BISHOP)};
    Bitboard bbFriendly {pos.getUnitsBb(co)};
    Bitboard bbAll {pos.getUnitsBb()};
    Square fromSq {NO_SQ};
    Bitboard bbTo {BB_NONE};
    while (bbFrom) {
        fromSq = popLsb(bbFrom);
        bbTo = (findDiagAttacks(fromSq, bbAll) |
                findAntidiagAttacks(fromSq, bbAll))
               & ~bbFriendly;
        while (bbTo) {
            mvlist.push_back(buildMove(fromSq, popLsb(bbTo)));
        }
    }
    return mvlist;
}

Movelist& IMoveRules::addRookMoves(Movelist& mvlist, Colour co, const Position& pos) {
    Bitboard bbFrom {pos.getUnitsBb(co, ROOK)};
    Bitboard bbFriendly {pos.getUnitsBb(co)};
    Bitboard bbAll {pos.getUnitsBb()};
    Square fromSq {NO_SQ};
    Bitboard bbTo {BB_NONE};
    while (bbFrom) {
        fromSq = popLsb(bbFrom);
        bbTo = (findRankAttacks(fromSq, bbAll) |
                findFileAttacks(fromSq, bbAll))
               & ~bbFriendly;
        while (bbTo) {
            mvlist.push_back(buildMove(fromSq, popLsb(bbTo)));
        }
    }
    return mvlist;
}

Movelist& IMoveRules::addQueenMoves(Movelist& mvlist, Colour co, const Position& pos) {
    Bitboard bbFrom {pos.getUnitsBb(co, QUEEN)};
    Bitboard bbFriendly {pos.getUnitsBb(co)};
    Bitboard bbAll {pos.getUnitsBb()};
    Square fromSq {NO_SQ};
    Bitboard bbTo {BB_NONE};
    while (bbFrom) {
        fromSq = popLsb(bbFrom);
        bbTo = (findRankAttacks(fromSq, bbAll) |
                findFileAttacks(fromSq, bbAll) |
                findDiagAttacks(fromSq, bbAll) |
                findAntidiagAttacks(fromSq, bbAll))
               & ~bbFriendly;
        while (bbTo) {
            mvlist.push_back(buildMove(fromSq, popLsb(bbTo)));
        }
    }
    return mvlist;
}

Movelist& IMoveRules::addPawnAttacks(Movelist& mvlist, Colour co, const Position& pos) {
    Bitboard bbFrom {pos.getUnitsBb(co, PAWN)};
    Bitboard bbEnemy {pos.getUnitsBb(!co)};
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        Bitboard bbTo {pawnAttacks[co][fromSq] & bbEnemy};
        while (bbTo) {
            mvlist.push_back(buildMove(fromSq, popLsb(bbTo)));
        }
    }
    return mvlist;
}

Movelist& IMoveRules::addPawnMoves(Movelist& mvlist, Colour co, const Position& pos) {
    /// Generates moves, captures, double moves, promotions (and captures).
    /// Does not generate en passant moves.
    Bitboard bbFrom {pos.getUnitsBb(co, PAWN)};
    Square toSq {NO_SQ};
    
    Bitboard bbEnemy {pos.getUnitsBb(!co)};
    Bitboard bbAll {pos.getUnitsBb()};
    
    while (bbFrom) {
        Square fromSq {popLsb(bbFrom)};
        // Generate captures (and capture promotions).
        Bitboard bbAttacks {pawnAttacks[co][fromSq] & bbEnemy};
        while (bbAttacks) {
            toSq = popLsb(bbAttacks);
            if (toSq & BB_OUR_8[co]) {
                mvlist.push_back(buildPromotion(fromSq, toSq, KNIGHT));
                mvlist.push_back(buildPromotion(fromSq, toSq, BISHOP));
                mvlist.push_back(buildPromotion(fromSq, toSq, ROOK));
                mvlist.push_back(buildPromotion(fromSq, toSq, QUEEN));
            } else {
                mvlist.push_back(buildMove(fromSq, toSq));
            }
        }
        // Generate single (and promotions) and double moves.
        toSq = (co == WHITE) ? shiftN(fromSq) : shiftS(fromSq);
        if (!(toSq & bbAll)) {
            // Single moves (and promtions).
            if (toSq & BB_OUR_8[co]) {
                mvlist.push_back(buildPromotion(fromSq, toSq, KNIGHT));
                mvlist.push_back(buildPromotion(fromSq, toSq, BISHOP));
                mvlist.push_back(buildPromotion(fromSq, toSq, ROOK));
                mvlist.push_back(buildPromotion(fromSq, toSq, QUEEN));
            } else {
                mvlist.push_back(buildMove(fromSq, toSq));
            }
            // Double moves
            if (fromSq & BB_OUR_2[co]) {
                toSq = (co == WHITE)
                    ? shiftN(shiftN(fromSq))
                    : shiftS(shiftS(fromSq));
                if (!(toSq & bbAll)) {
                    mvlist.push_back(buildMove(fromSq, toSq));
                }
            }
        }
    }
    return mvlist;
}

Movelist& IMoveRules::addEpMoves(Movelist& mvlist, Colour co, const Position& pos) {
    Square toSq {pos.getEpSq()}; // only one possible ep square at all times.
    Square fromSq {NO_SQ};
    Bitboard bbEp {bbFromSq(toSq)};
    // each ep square could have 2 pawns moving to it.
    Bitboard bbEpFrom { (co == WHITE)
        ? (shiftSW(bbEp) | shiftSE(bbEp))
        : (shiftNW(bbEp) | shiftNE(bbEp))
    };
    Bitboard bbEpPawns {bbEpFrom & pos.getUnitsBb(co, PAWN)};
    while (bbEpPawns) {
        fromSq = popLsb(bbEpPawns);
        mvlist.push_back(buildEp(fromSq, toSq));
    }
    return mvlist;
}

bool IMoveRules::isCastlingValid(CastlingRights cr, const Position& pos) {
    /// Helper function to test if a particular castling is valid.
    /// Takes [CastlingRights cr] corresponding to a single castling.
    /// Tests if king or rook has moved, if their paths are clear, and if the
    /// king passes through any attacked squares. Ignores side to move.
    ///
    /// Subtlety 1: the attacked squares test looks at the diagram "as-is",
    /// including the involved king and rook.
    /// Subtlety 2: because of subtlety 1, there needs to be an additional test
    /// for checks after the move has been *made*. (Not in regular chess, but in
    /// 960, or with certain fairy pieces, it is *necessary*.)
    
    // Test if king or relevant rook have moved.
    if (!(cr & pos.getCastlingRights())) {
        return false;
    }
    Bitboard rookMask {pos.getCastlingRookMask(cr)};
    Bitboard kingMask {pos.getCastlingKingMask(cr)};
    Bitboard bbOthers {pos.getUnitsBb() ^ pos.getOrigKingSq(cr) ^
                       pos.getOrigRookSq(cr)};
    // Test if king and rook paths are clear of obstruction.
    if ((rookMask | kingMask) & bbOthers) {
        return false;
    }
    // Test if there are attacked squares in the king's path.
    Square sq {NO_SQ};
    while (kingMask) {
        sq = popLsb(kingMask);
        if (isAttacked(sq, !toColour(cr), pos)) {
            return false;
        }
    }
    // Conditions met, castling is valid.
    return true;
}

Movelist& IMoveRules::addCastlingMoves(Movelist& mvlist, Colour co, const Position& pos) {
    if (co == WHITE) {
        if (isCastlingValid(CASTLE_WSHORT, pos)) {
            Move mv {buildCastling(pos.getOrigKingSq(CASTLE_WSHORT), pos.getOrigRookSq(CASTLE_WSHORT))};
            mvlist.push_back(mv);
        }
        if (isCastlingValid(CASTLE_WLONG, pos)) {
            Move mv {buildCastling(pos.getOrigKingSq(CASTLE_WLONG), pos.getOrigRookSq(CASTLE_WLONG))};
            mvlist.push_back(mv);
        }
    } else if (co == BLACK) {
        if (isCastlingValid(CASTLE_BSHORT, pos)) {
            Move mv {buildCastling(pos.getOrigKingSq(CASTLE_BSHORT), pos.getOrigRookSq(CASTLE_BSHORT))};
            mvlist.push_back(mv);
        }
        if (isCastlingValid(CASTLE_BLONG, pos)) {
            Move mv {buildCastling(pos.getOrigKingSq(CASTLE_BLONG), pos.getOrigRookSq(CASTLE_BLONG))};
            mvlist.push_back(mv);
        }
    }
    return mvlist;
}