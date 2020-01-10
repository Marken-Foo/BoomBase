#ifndef ATOMIC_POSITION_INCLUDED
#define ATOMIC_POSITION_INCLUDED

#include "position.h"
#include "chess_types.h"

#include <array>

struct AtomicStateInfo : StateInfo {
    AtomicStateInfo(Piece pc, CastlingRights cr, Square sq, int num,
                    std::array<Bitboard, NUM_COLOURS> bbs,
                    std::array<Bitboard, NUM_PIECE_TYPES> bbs2)
        : StateInfo{pc, cr, sq, num}
        , bbExplodedByColour{bbs}
        , bbExplodedByType{bbs2}
            { }
    std::array<Bitboard, NUM_COLOURS> bbExplodedByColour {};
    std::array<Bitboard, NUM_PIECE_TYPES> bbExplodedByType {};
};

class AtomicPosition : public Position {
    public:
    void makeMove(Move mv);
    void unmakeMove(Move mv);
};

#endif //#ifndef ATOMIC_POSITION_INCLUDED