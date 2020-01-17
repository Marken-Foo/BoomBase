#ifndef MOVE_VALIDATOR_INCLUDED
#define MOVE_VALIDATOR_INCLUDED

#include "chess_types.h"
#include "move.h"
#include "move_rules.h"

#include <cstdint>
#include <memory>

// Contains functions pertaining to move generation from a given position.
//
// Definitions of relevant terms:
//
// "Legal moves" are those which follow all the rules of chess.
// "Valid moves" meet most of the requirements of legality, except that one
// can leave one's own royalty (kings, for normal chess) under attack.
// "(Valid) attacks" are valid moves with the additional relaxation that the
// target square may be occupied by a friendly piece.
// "Invalid moves" are all other moves (e.g. moved piece doesn't exist, movement
// makes no sense, attempting to move an enemy piece, castling without meeting
// all the criteria, promotion to enemy knight...)

class Position;

enum Variant {
    ORTHO, ATOMIC
};

class MoveValidator {
    // A class that contains static methods to validate a move, given a Move and
    // a Position. Delegates actual checking to specialised classes.
    public:
    MoveValidator() {
        setVariant(ORTHO);
    }
    
    MoveValidator(Variant var) {
        setVariant(var);
    }
    
    void setVariant(Variant var);
    Variant getVariant() {return currentVariant;}
    
    bool isLegal(Move mv, Position& pos) {
        return rules->isLegal(mv, pos);
    }
    bool isInCheck(Colour co, Position& pos) {
        return rules->isInCheck(co, pos);
    }
    Movelist generateLegalMoves(Position& pos) {
        return rules->generateLegalMoves(pos);
    }
    
    uint64_t perft(int depth, Position& pos);
    
    protected:
    Variant currentVariant;
    std::unique_ptr<IMoveRules> rules;
};

#endif //#ifndef MOVE_VALIDATOR_INCLUDED