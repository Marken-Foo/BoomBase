#include "bitboard_lookup.h"

#include "bitboard.h"
#include "chess_types.h"

#include <array>

// Arrays of first-rank/file attacks, for slider move generation.
// Indexed by the 8 possible slider locations, and 2^(8 - 2) = 64 non-edge
// occupancy states.
std::array<std::array<Bitboard, 64>, 8> firstRankAttacks{};
std::array<std::array<Bitboard, 64>, 8> firstFileAttacks{};

// Defining lookup arrays exposed in .h
std::array<Bitboard, NUM_SQUARES> knightAttacks{};
std::array<Bitboard, NUM_SQUARES> kingAttacks{};
std::array<std::array<Bitboard, NUM_SQUARES>, NUM_COLOURS> pawnAttacks{};
std::array<Bitboard, NUM_SQUARES> diagMasks{};
std::array<Bitboard, NUM_SQUARES> antidiagMasks{};
std::array<std::array<Bitboard, NUM_SQUARES>, NUM_SQUARES> lineBetween;

// Declaring auxiliary functions not exposed in .h
void initialiseAllDiagMasks();
void initialiseFirstFileAttacks();
void initialiseFirstRankAttacks();
void initialiseKingAttacks();
void initialiseKnightAttacks();
void initialisePawnAttacks();
void initialiseLineBetween();

// === Lookup table initialiser ===
void initialiseBbLookup() {
    initialiseAllDiagMasks();
    initialiseFirstRankAttacks();
    initialiseFirstFileAttacks();
    initialiseKingAttacks();
    initialiseKnightAttacks();
    initialisePawnAttacks();
    // Must be initialised after rank, file, diag attacks available!
    initialiseLineBetween();
    return;
}

// === Sliding attack getters ===
// bbPos contains all pieces of position.
Bitboard findRankAttacks(Square sq, Bitboard bbPos) {
    int irank {getRankIdx(sq)};
    int ifile {getFileIdx(sq)};
    Bitboard oc { bbPos & (BB_1 << (8*irank)) }; // extract just desired rank
    // b-file multiplication puts desired bits on 8th rank to extract.
    int ioc { static_cast<int>((oc * BB_B) >> (64 - 6)) };
    return (BB_1 << (8*irank)) & firstRankAttacks[ifile][ioc];
}

Bitboard findDiagAttacks(Square sq, Bitboard bbPos) {
    int ifile {getFileIdx(sq)};
    Bitboard oc { bbPos & (diagMasks[sq]) }; // extract just desired diagonal
    // b-file multiplication puts desired bits on 8th rank.
    int ioc = { static_cast<int>((oc * BB_B) >> (64 - 6)) };
    return diagMasks[sq] & firstRankAttacks[ifile][ioc];
}

Bitboard findAntidiagAttacks(Square sq, Bitboard bbPos) {
    int ifile {getFileIdx(sq)};
    Bitboard oc { bbPos & (antidiagMasks[sq]) };
    // b-file multiplication puts desired bits on 8th rank.
    int ioc = { static_cast<int>((oc * BB_B) >> (64-6)) };
    return antidiagMasks[sq] & firstRankAttacks[ifile][ioc];
}

Bitboard findFileAttacks(Square sq, Bitboard bbPos) {
    int irank {getRankIdx(sq)};
    int ifile {getFileIdx(sq)};
    Bitboard oc { (bbPos >> ifile) & BB_A }; // send desired file bits to a-file
    // multiply by c2-h7 diagonal; flip multiplication extracts lookup index
    int ioc { static_cast<int>(oc * 0x0080402010080400ULL >> (64-6)) };
    return firstFileAttacks[irank][ioc] & (BB_A << ifile);
}

Bitboard findRookAttacks(Square sq, Bitboard bbPos) {
    return findRankAttacks(sq, bbPos) | findFileAttacks(sq, bbPos);
}

Bitboard findBishopAttacks(Square sq, Bitboard bbPos) {
    return findDiagAttacks(sq, bbPos) | findAntidiagAttacks(sq, bbPos);
}

// === Auxiliary methods ===
    
// --- diagonal masks ---

void initialiseAllDiagMasks() {
    // Generates both diagMasks and antidiagMasks, indexed by Square.
    // first define the S and W (anti: S and E) edges of the table
    diagMasks[0] = BB_LONG_DIAG;
    antidiagMasks[7] = BB_LONG_ANTIDIAG;
    antidiagMasks[56] = BB_LONG_ANTIDIAG;
    
    for (int i = 1; i < 8; ++i) {
        diagMasks[i] = shiftS(diagMasks[i - 1]);
        diagMasks[8*i] = shiftN(diagMasks[8*i - 8]);
        antidiagMasks[7 - i] = shiftS(antidiagMasks[8 - i]);
        antidiagMasks[7 + 8*i] = shiftN(antidiagMasks[8*i - 1]);
    }
    // then propagate and fill in the rest of the table.    
    for (int x = 1; x < 8; ++x) {
        for (int y = 1; y < 8; ++y) {
            diagMasks[x+8*y] = diagMasks[(x-1) + 8*(y-1)];
            antidiagMasks[7-x+8*y] = antidiagMasks[(8-x) + 8*(y-1)];
        }
    }
    return;
}

// --- 1st-rank and 1st-file attacks ---
void initialiseFirstRankAttacks() {
    for (int ioc = 0; ioc < 64; ++ioc) {
         // +129 sets the end bits of rank to 1 (cannot attack past board edge)
        Bitboard oc = (ioc << 1) + 129;
        
        // If slider at end of rank, that end must be handled differently.
        // Therefore use two separate if conditions to isolate them.
        for (int idx_r = 0; idx_r < 8; ++idx_r) {
            int smallLimit {0}; // Westmost square attacked by the slider.
            int bigLimit {7}; // Eastmost square attacked by the slider.
            Bitboard r {bbFromSq(square(idx_r))};
            if (idx_r != 0) {
                // zero high bits, take highest set bit (lowest above slider).
                smallLimit = gsb(oc & (r - 1));
            } 
            if (idx_r != 7) {
                // zero low bits, take lowest set bit (highest below slider).
                bigLimit = lsb(oc & ~((r << 1) - 1));
            }
            // Get bitboard of all bits between limits, inclusive.
            Bitboard bb = ((bbFromSq(square(bigLimit)) << 1) -
                           bbFromSq(square(smallLimit)));
            bb ^= r; // slider does not attack itself
            bb *= BB_A; // north-fill multiplication
            firstRankAttacks[idx_r][ioc] = bb;
        }
    }
    return;
}


void initialiseFirstFileAttacks() {
    for (int ioc = 0; ioc < 64; ++ioc) {
        // NOTE: Index 0 here is 8th rank of the real file!
        // In first-rank terms, 1st rank is mapped to h1, 2nd rank to g1, etc.
        // First we generate the attacks using a rank, taking into account the
        // changed indices, then flip it later such that it is correct.
        
        // +129 sets the end bits of rank to 1 (cannot attack past board edge)
        Bitboard oc = (ioc << 1) + 129;
        
        // If slider at end of rank, that end must be handled differently.
        // Therefore use two separate if conditions to isolate them.
        for (int idx_r = 0; idx_r < 8; ++idx_r) {
            int smallLimit {0};
            int bigLimit {7};
            Bitboard r {bbFromSq(square(7 - idx_r))};
            if (idx_r != 7) {
                // zero high bits, take highest set bit (lowest above slider).
                smallLimit = gsb(oc & (r - 1));
            } 
            if (idx_r != 0) {
                // zero low bits, take lowest set bit (highest below slider).
                bigLimit = lsb(oc & ~((r << 1) - 1));
            }
            // Get bitboard of all bits between limits, inclusive.
            Bitboard bb = ((bbFromSq(square(bigLimit)) << 1) -
                           bbFromSq(square(smallLimit)));
            bb ^= r; // slider does not attack itself
            bb = (bb * BB_LONG_DIAG) & BB_H; // rotate attacks to the h-file.
            // Now fill left.
            bb |= bb >> 1;
            bb |= bb >> 2;
            bb |= bb >> 4;
            firstFileAttacks[idx_r][ioc] = bb;
        }
    }
    return;
}

// --- Simple piece attacks ---
void initialiseKnightAttacks() {
    for (int isq = 0; isq < NUM_SQUARES; ++isq) {
        Bitboard bb {bbFromSq(square(isq))};
        bb = ( shiftN(shiftNW(bb)) | shiftN(shiftNE(bb)) |
               shiftE(shiftNE(bb)) | shiftE(shiftSE(bb)) |
               shiftS(shiftSE(bb)) | shiftS(shiftSW(bb)) |
               shiftW(shiftSW(bb)) | shiftW(shiftNW(bb)) );
        knightAttacks[isq] = bb;
    }
    return;
}

void initialiseKingAttacks() {
    for (int isq = 0; isq < NUM_SQUARES; ++isq) {
        Bitboard bb {bbFromSq(square(isq))};
        bb = ( shiftN(bb) | shiftNE(bb) | shiftE(bb) | shiftSE(bb) |
               shiftS(bb) | shiftSW(bb) | shiftW(bb) | shiftNW(bb) );
        kingAttacks[isq] = bb;
    }
    return;
}

void initialisePawnAttacks() {
    // Will generate legal moves for illegal pawn positions too (1st/8th rank)
    for (int isq = 0; isq < NUM_SQUARES; ++isq) {
        Bitboard bb {bbFromSq(square(isq))};
        pawnAttacks[WHITE][isq] = shiftNE(bb) | shiftNW(bb);
        pawnAttacks[BLACK][isq] = shiftSE(bb) | shiftSW(bb);
    }
    return;
}

void initialiseLineBetween() {
    // Must be intialised after rank, file and diag attacks are ready!
    for (int isq = 0; isq < NUM_SQUARES; ++isq) {
        Square sq1 = square(isq);
        Bitboard bb1 = bbFromSq(sq1);
        Bitboard bbOrtho {findRookAttacks(sq1, bb1)};
        Bitboard bbDiag {findBishopAttacks(sq1, bb1)};
        for (int jsq = 0; jsq <= isq; ++jsq) {
            Square sq2 = square(jsq);
            if (sq2 & bbOrtho) {
                Bitboard bb = {findRookAttacks(sq2, bbFromSq(sq2))};
                lineBetween[isq][jsq] = bbOrtho & bb;
                lineBetween[jsq][isq] = bbOrtho & bb;
            } else if (sq2 & bbDiag) {
                Bitboard bb = {findBishopAttacks(sq2, bbFromSq(sq2))};
                lineBetween[isq][jsq] = bbDiag & bb;
                lineBetween[jsq][isq] = bbDiag & bb;
            } else {
                lineBetween[isq][jsq] = BB_NONE;
                lineBetween[jsq][isq] = BB_NONE;
            }
        }
    }
    return;
}
