#ifndef ATOMIC_MOVE_RULES_INCLUDED
#define ATOMIC_MOVE_RULES_INCLUDED

#include "bitboard.h"
#include "chess_types.h"
#include "move.h"
#include "move_rules.h"

class Position;

class AtomicMoveRules : public IMoveRules {
    // Knowledge of the rules of atomic chess.
    public:
    AtomicMoveRules() = default;
    
    bool isLegal(Move mv, Position& pos) override;
    bool isInCheck(Colour co, const Position& pos) override;
    Movelist generateLegalMoves(Position& pos) override;
    
    protected:
    bool isAttacked(Square sq, Colour co, const Position& pos) override;
    bool isCheckAttacked(Square sq, Colour co, const Position& pos) override;
    
    Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos);
    Bitboard attacksTo(Square sq, Colour co, const Position& pos);
};

#endif //#ifndef ATOMIC_MOVE_RULES_INCLUDED