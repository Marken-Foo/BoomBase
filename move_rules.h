#ifndef I_MOVE_RULES_INCLUDED
#define I_MOVE_RULES_INCLUDED

#include "chess_types.h"
#include "bitboard.h"
#include "move.h"

#include <cstdint>
#include <memory>

class Position;

class IMoveRules {
    // An abstract class for the concrete logic-containing objects.
    public:
    virtual ~IMoveRules() {}
    
    virtual bool isLegal(Move mv, Position& pos) = 0;
    virtual bool isInCheck(Colour co, const Position& pos) = 0;
    virtual Movelist generateLegalMoves(Position& pos) = 0;
    
    protected:
    IMoveRules() {}
    IMoveRules(const IMoveRules&) {}
    IMoveRules& operator=(const IMoveRules&) {return *this;}
    
    // Code common to most chess variants
    Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos);
    Bitboard attacksTo(Square sq, Colour co, const Position& pos);
    bool isAttacked(Square sq, Colour co, const Position& pos);
    
    // Piece moves
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
};

#endif //#I_MOVE_RULES_INCLUDED