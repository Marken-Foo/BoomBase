#ifndef ATOMIC_POSITION_INCLUDED
#define ATOMIC_POSITION_INCLUDED

#include "chess_types.h"
#include "move.h"
#include "position.h"

#include <array>
#include <deque>

/// Position class for atomic chess.
///
class AtomicPosition : public Position {
    public:
    void makeMove(Move mv) override;
    void unmakeMove(Move mv) override;
    void reset() override;
    
    protected:
    struct ExplosionInfo;
    std::deque<ExplosionInfo> explosionStack {};
    
    struct ExplosionInfo {
        ExplosionInfo(Piece pc,
                      std::array<Bitboard, NUM_COLOURS> bbExByCo,
                      std::array<Bitboard, NUM_PIECE_TYPES> bbExByPcty)
            : movedPiece{pc}
            , bbExplosionByColour{bbExByCo}
            , bbExplosionByType{bbExByPcty}
        { }
        
        Piece movedPiece {NO_PIECE};
        std::array<Bitboard, NUM_COLOURS> bbExplosionByColour {};
        std::array<Bitboard, NUM_PIECE_TYPES> bbExplosionByType {};
    };
};

#endif //#ifndef ATOMIC_POSITION_INCLUDED