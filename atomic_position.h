#ifndef ATOMIC_POSITION_INCLUDED
#define ATOMIC_POSITION_INCLUDED

#include "position.h"
#include "chess_types.h"
#include "move.h"

#include <array>
#include <deque>

struct AtomicStateInfo : StateInfo {
    AtomicStateInfo(Piece pcDest, CastlingRights cr, Square sq, int num,
                    Piece pc,
                    std::array<Bitboard, NUM_COLOURS> bbExCo,
                    std::array<Bitboard, NUM_PIECE_TYPES> bbExPcty)
        : StateInfo{pcDest, cr, sq, num}
        , movedPiece{pc}
        , bbExplodedByColour{bbExCo}
        , bbExplodedByType{bbExPcty}
    { }
    
    Piece movedPiece {NO_PIECE};
    std::array<Bitboard, NUM_COLOURS> bbExplodedByColour {};
    std::array<Bitboard, NUM_PIECE_TYPES> bbExplodedByType {};
};

class AtomicPosition : public Position {
    public:
    void makeMove(Move mv);
    void unmakeMove(Move mv);
    
    protected:
    std::deque<AtomicStateInfo> undoStack {};
};

#endif //#ifndef ATOMIC_POSITION_INCLUDED