#include "atomic_capture_masks.h"
#include "chess_types.h"
#include "bitboard.h"

#include <array>

std::array<Bitboard, NUM_SQUARES> atomicMasks;

void initialiseAtomicMasks() {
    for (int isq = 0; isq < NUM_SQUARES; ++isq) {
        Bitboard bb {bbFromSq(square(isq))};
        bb |= ( shiftN(bb) | shiftNE(bb) | shiftE(bb) | shiftSE(bb) |
               shiftS(bb) | shiftSW(bb) | shiftW(bb) | shiftNW(bb) );
        atomicMasks[isq] = bb;
    }
    return;
}