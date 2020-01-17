#ifndef ORTHO_MOVE_RULES_INCLUDED
#define ORTHO_MOVE_RULES_INCLUDED

#include "chess_types.h"
#include "bitboard.h"
#include "move_rules.h"
#include "move.h"

class Position;

class OrthoMoveRules : public IMoveRules { 
    // The rules of regular chess, or orthochess.
    public:
    OrthoMoveRules() = default;
    
    bool isLegal(Move mv, Position& pos) override;
    bool isInCheck(Colour co, const Position& pos) override;
    Movelist generateLegalMoves(Position& pos) override;
    
    protected:
    bool isAttacked(Square sq, Colour co, const Position& pos) override;
    
    Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos);
    Bitboard attacksTo(Square sq, Colour co, const Position& pos);
};

#endif //#ifndef ORTHO_MOVE_RULES_INCLUDED