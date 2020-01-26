#ifndef I_MOVE_RULES_INCLUDED
#define I_MOVE_RULES_INCLUDED

#include "atomic_capture_masks.h"
#include "bitboard.h"
#include "chess_types.h"
#include "move.h"

#include <cstdint>
#include <memory>

class Position;

class IMoveRules {
    /// An abstract class for the concrete logic-containing rules objects.
    /// These objects will be able to judge move legality of a Position.
    public:
    virtual ~IMoveRules() {}
    
    virtual bool isLegal(Move mv, Position& pos) = 0;
    virtual bool isInCheck(Colour co, const Position& pos) = 0;
    virtual Movelist generateLegalMoves(Position& pos) = 0;
    
    protected:
    IMoveRules() {}
    IMoveRules(const IMoveRules&) {}
    IMoveRules& operator=(const IMoveRules&) {return *this;}
    
    // "Attacks" depend on the variant.
    virtual bool isAttacked(Square sq, Colour co, const Position& pos) = 0;
    // Whether an enemy king placed on that square would be in check. (In
    // atomic, this is different from plain attacks!)
    virtual bool isCheckAttacked(Square sq, Colour co, const Position& pos) = 0;
    
    // Code common to most chess variants
    // (Regular) piece moves are independent of variant
    Movelist& addKingMoves(Movelist& mvlist, Colour co, const Position& pos);
    Movelist& addKnightMoves(Movelist& mvlist, Colour co, const Position& pos);
    Movelist& addBishopMoves(Movelist& mvlist, Colour co, const Position& pos);
    Movelist& addRookMoves(Movelist& mvlist, Colour co, const Position& pos);
    Movelist& addQueenMoves(Movelist& mvlist, Colour co, const Position& pos);
    Movelist& addPawnMoves(Movelist& mvlist, Colour co, const Position& pos);
    Movelist& addEpMoves(Movelist& mvlist, Colour co, const Position& pos);
    
    // Helper method to write pawn moves to movelist.
    Movelist& addPawnMoves(Movelist& mvlist, Colour co,
                           Square fromSq, Square toSq);
    
    // Castling validation needs to know which squares are attacked.
    bool isCastlingValid(CastlingRights cr, const Position& pos);
    Movelist& addCastlingMoves(Movelist& mvlist, Colour co,
                               const Position& pos);
    
    // For efficient legal move generation.
    Bitboard findPinned(Colour co, const Position& pos);
};

#endif //#I_MOVE_RULES_INCLUDED