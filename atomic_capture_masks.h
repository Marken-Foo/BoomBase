#ifndef ATOMIC_CAPTURE_MASKS
#define ATOMIC_CAPTURE_MASKS

#include "bitboard.h"
#include "chess_types.h"

#include <array>

/// Lookup table for atomic capture masks (the "blast radius" of a capture).
/// Should include epicentre of the explosion.

extern std::array<Bitboard, NUM_SQUARES> atomicMasks;

void initialiseAtomicMasks();

#endif //#ifndef ATOMIC_CAPTURE_MASKS