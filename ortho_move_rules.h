#ifndef ORTHO_MOVE_RULES_INCLUDED
#define ORTHO_MOVE_RULES_INCLUDED

#include "chess_types.h"
#include "move_validator.h"
#include "move.h"

class Position;

class OrthoMoveRules : public IMoveRules { 
    // The rules of regular chess, or orthochess.
    public:
    OrthoMoveRules() = default;
    
    bool isLegal(Move mv, Position& pos);
    bool isInCheck(Colour co, const Position& pos);
    Movelist generateLegalMoves(Position& pos);
};
#endif //#ifndef ORTHO_MOVE_RULES_INCLUDED