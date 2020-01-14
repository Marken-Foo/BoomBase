#ifndef MOVE_VALIDATOR_INCLUDED
#define MOVE_VALIDATOR_INCLUDED

#include "chess_types.h"
#include "bitboard.h"
#include "move.h"

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

class MoveValidator {
    // A class that contains static methods to validate a move, given a Move and
    // a Position& or uPosition*. Delegates actual checking to specialised classes
    // (Command? Strategy? W/e)
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




/* 
// SPLIT TO ANOTHER FILE
class AtomicMoveRules : IMoveRules{
    // The rules of atomic chess.
    public:
    bool isLegal(Move mv, Position& pos);
    bool isInCheck(Colour co, Position& pos);
    Movelist generateLegalMoves(Position& pos);
    
    protected:
    // in atomic, kings can't capture, so they don't attack anything.
    Bitboard attacksFrom(Square sq, Colour co, PieceType pcty, const Position& pos);
    Bitboard attacksTo(Square sq, Colour co, const Position& pos); 
    bool isAttacked(Square sq, Colour co, const Position& pos);
}; */

#endif //#ifndef MOVE_VALIDATOR_INCLUDED