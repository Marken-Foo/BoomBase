#ifndef ATOMIC_MOVE_RULES_INCLUDED
#define ATOMIC_MOVE_RULES_INCLUDED

#include "chess_types.h"
#include "bitboard.h"
#include "move_rules.h"
#include "move.h"

class Position;

class AtomicMoveRules : public IMoveRules {
    public:
    AtomicMoveRules() = default;
    
    bool isLegal(Move mv, Position& pos);
    bool isInCheck(Colour co, const Position& pos);
    Movelist generateLegalMoves(Position& pos);
    
    protected:
    // override base rules -- kings don't attack in atomic
    Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos);
    Bitboard attacksTo(Square sq, Colour co, const Position& pos);
    bool isAttacked(Square sq, Colour co, const Position& pos);
};

#endif //#ifndef ATOMIC_MOVE_RULES_INCLUDED