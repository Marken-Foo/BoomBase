#include "movegen.h"

#include "chess_types.h"
#include "move.h"
#include "bitboard.h"
#include "bitboard_lookup.h"
#include "position.h"

#include <cstdint>
#include <iostream>

Movelist generateLegalMoves(Position& pos) {
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


bool isInCheck(Colour co, const Position& pos) {
    // Test if a side (colour) is in check.
    Bitboard bb {pos.getUnitsBb(co, KING)};
    Square sq {popLsb(bb)}; // assumes exactly one king per side.
    return isAttacked(sq, !co, pos);
}


bool isLegal(Move mv, Position& pos) {
    // Test if making a move would leave one's own royalty in check.
    // Assumes move is valid.
    // For eventual speedup logic can be improved from naive make-unmake-make.
    Colour co {pos.getSideToMove()}; // is this fine? (why not pass as arg?)
    pos.makeMove(mv);
    bool isSuicide {isInCheck(co, pos)};
    pos.unmakeMove(mv);
    return !isSuicide;
}


uint64_t perft(int depth, Position& pos) {
    // Recursive function to count all legal moves (nodes) at depth n.
    uint64_t nodes = 0;
    // Terminating condition
    if (depth == 0) {return 1;}
    
    Movelist mvlist = generateLegalMoves(pos);
    int sz = mvlist.size();
    // Recurse.
    for (int i = 0; i < sz; ++i) {
        pos.makeMove(mvlist[i]);
        int childN = perft(depth-1, pos);
        nodes += childN;
        pos.unmakeMove(mvlist[i]);
    }
    return nodes;
}


// === Functions to generate valid moves of a particular type ===
// Functions take in a Movelist and append to it the valid moves generated.
Movelist& addKingMoves(Movelist& mvlist, Colour co, const Position& pos) {
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

Movelist& addKnightMoves(Movelist& mvlist, Colour co, const Position& pos) {
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

Movelist& addBishopMoves(Movelist& mvlist, Colour co, const Position& pos) {
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

Movelist& addRookMoves(Movelist& mvlist, Colour co, const Position& pos) {
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

Movelist& addQueenMoves(Movelist& mvlist, Colour co, const Position& pos) {
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

Movelist& addPawnAttacks(Movelist& mvlist, Colour co, const Position& pos) {
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

Movelist& addPawnMoves(Movelist& mvlist, Colour co, const Position& pos) {
    // Generates moves, captures, double moves, promotions (and captures).
    // Does not generate en passant moves.
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

Movelist& addEpMoves(Movelist& mvlist, Colour co, const Position& pos) {
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

bool isCastlingValid(CastlingRights cr, const Position& pos) {
    // Helper function to test if a particular castling is valid.
    // Takes [CastlingRights cr] corresponding to a single castling.
    // Tests if king or rook has moved, if their paths are clear, and if the
    // king passes through any attacked squares. Ignores side to move.
    //
    // Subtlety 1: the attacked squares test looks at the diagram "as-is",
    // including the involved king and rook.
    // Subtlety 2: because of subtlety 1, there needs to be an additional test
    // for checks after the move has been *made*. (Not in regular chess, but in
    // 960, or with certain fairy pieces, it is *necessary*.)
    
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

Movelist& addCastlingMoves(Movelist& mvlist, Colour co, const Position& pos) {
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


Bitboard attacksFrom(Square sq, Colour co, PieceType pcty,
                     const Position& pos) {
    // Returns bitboard of squares attacked by a given piece type placed on a
    // given square.
    Bitboard bbAttacked {0};
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
        bbAttacked = kingAttacks[sq];
        break;
    }
    return bbAttacked;
}


Bitboard attacksTo(Square sq, Colour co, const Position& pos) {
    // Returns bitboard of units of a given colour that attack a given square.
    // In chess, most piece types have the following property: if piece PC is on
    // square SQ_A attacking SQ_B, then from SQ_B it would attack SQ_A.
    Bitboard bbAttackers {0};
    bbAttackers = kingAttacks[sq] & pos.getUnitsBb(co, KING);
    bbAttackers |= knightAttacks[sq] & pos.getUnitsBb(co, KNIGHT);
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

bool isAttacked(Square sq, Colour co, const Position& pos) {
    // Returns if a square is attacked by pieces of a particular colour.
    return !(attacksTo(sq, co, pos) == BB_NONE);
}