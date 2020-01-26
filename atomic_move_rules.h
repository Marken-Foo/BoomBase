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
    
    bool isLegalNaive(Move mv, Position& pos);
    
    protected:
    bool isAttacked(Square sq, Colour co, const Position& pos) override;
    bool isCheckAttacked(Square sq, Colour co, const Position& pos) override;
    
    protected:
    Bitboard attacksTo(Square sq, Colour co, const Position& pos);
    
    private:
    Movelist generateLegalMovesNaive(Position& pos);
    
    Movelist generateLegalMovesByType(Position& pos);
    Movelist& addLegalKingMoves(Movelist& mvlist, Position& pos);
    Movelist& addLegalKnightMoves(Movelist& mvlist, Position& pos);
    Movelist& addLegalSliderMoves(Movelist& mvlist, Position& pos,
                                  PieceType pcty);
    Movelist& addLegalPawnMoves(Movelist& mvlist, Position& pos);
    Movelist& addLegalPawnCaptures(Movelist& mvlist, Position& pos);
    Movelist& addLegalPawnPushes(Movelist& mvlist, Position& pos);
    Movelist& addLegalPawnDoublePushes(Movelist& mvlist, Position& pos);
    
    bool isCaptureLegal(Square fromSq, Square toSq, const Position& pos);
    bool isLegalNonKingNonCapture(Square fromSq, Square toSq,
                                  const Position& pos);
    bool isConnectedKings(const Position& pos);
    bool isInterpositionLegal(Square fromSq, Square toSq, const Position& pos);
};

#endif //#ifndef ATOMIC_MOVE_RULES_INCLUDED