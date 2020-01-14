#ifndef ORTHO_MOVE_RULES_INCLUDED
#define ORTHO_MOVE_RULES_INCLUDED

#include "chess_types.h"
#include "move.h"

class IMoveRules;
class pos;

class OrthoMoveRules : IMoveRules {
    // The rules of regular chess, or orthochess.
    public:
    bool isLegal(Move mv, Position& pos);
    bool isInCheck(Colour co, Position& pos);
    Movelist generateLegalMoves(Position& pos);
};
#endif //#ifndef ORTHO_MOVE_RULES_INCLUDED