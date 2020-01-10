#ifndef ATOMIC_CAPTURE_MASKS
#define ATOMIC_CAPTURE_MASKS

#include "chess_types.h"
#include "bitboard.h"

#include <array>

extern std::array<Bitboard, NUM_SQUARES> atomicMasks;

void initialiseAtomicMasks();

#endif //#ifndef ATOMIC_CAPTURE_MASKS