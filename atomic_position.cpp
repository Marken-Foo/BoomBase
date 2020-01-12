#include "atomic_position.h"
#include "atomic_capture_masks.h"

#include <array>
#include <memory>

void AtomicPosition::makeMove(Move mv) {
    /// Makes a move by changing the state of AtomicPosition.
    /// Assumes the move is valid (not necessarily legal).
    /// Must maintain validity of the Position!
    
    // Stores explosion information
    std::array<Bitboard, NUM_COLOURS> explosionByColour {};
    std::array<Bitboard, NUM_PIECE_TYPES> explosionByType {};
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    const Piece pc {mailbox[fromSq]};
    
    // Castling is handled with parent method (same as orthochess).
    if (isCastling(mv)) {
        makeCastlingMove(mv);
        // push explosion information for consistency
        explosionStack.emplace_back(pc, explosionByColour, explosionByType);
        return;
    }
    
    const Colour co {sideToMove}; // assert sideToMove == getPieceColour(pc);
    const PieceType pcty {getPieceType(pc)};
    const Piece pcDest {mailbox[toSq]};
    const bool isCapture {pcDest != NO_PIECE};
    const bool isEp {::isEp(mv)};
    const Bitboard mask = atomicMasks[toSq];
    
    // (Not castling) begin updating position; remove moved piece.
    removePiece(co, pcty, fromSq);
    
    if (isCapture || isEp) {
        // Could be capture, promotion capture, or en passant.
        // Record, then remove all units exploded.
        // (Manipulate bitboards directly; mailbox is handled shortly after.)
        for (int i = 1; i < NUM_PIECE_TYPES; ++i) {
            // Adjacent pawns are *not* exploded (note index of for loop!).
            explosionByType[i] ^= bbByType[i] & mask;
            bbByType[i] &= ~explosionByType[i];
        }
        for (int j = 0; j < NUM_COLOURS; ++j) {
            // Adjacent pawns are *not* exploded.
            explosionByColour[j] ^= bbByColour[j] & mask & ~bbByType[PAWN];
            bbByColour[j] &= ~explosionByColour[j];
        }
        // But directly captured or en passant'd pawn, *is* exploded.
        if (getPieceType(pcDest) == PAWN && pcDest != NO_PIECE) {
            explosionByColour[!co] ^= toSq;
            explosionByType[PAWN] ^= toSq;
            bbByColour[!co] ^= toSq;
            bbByType[PAWN] ^= toSq;
        }
        if (isEp) {
            Square sqEpCap {(co == WHITE) ? shiftS(toSq) : shiftN(toSq)};
            explosionByColour[!co] ^= sqEpCap;
            explosionByType[PAWN] ^= sqEpCap;
            bbByColour[!co] ^= sqEpCap;
            bbByType[PAWN] ^= sqEpCap;
        }
        // (Now it is convenient to update the mailbox.)
        Bitboard bbExplosion {explosionByColour[WHITE] | explosionByColour[BLACK]};
        while (bbExplosion) {
            Square sq = popLsb(bbExplosion);
            mailbox[sq] = NO_PIECE;
        }
        
    } else {
        // is not a capture, could be move or normal promotion
        if (isPromotion(mv)) {
            PieceType pctyPromo {getPromotionType(mv)};
            addPiece(co, pctyPromo, toSq);
        } else {
            addPiece(co, pcty, toSq);
        }
    }
    
    // Save irreversible state information in struct, *before* altering them.
    undoStack.emplace_back(pcDest, castlingRights, epRights, fiftyMoveNum);
    explosionStack.emplace_back(pc, explosionByColour, explosionByType);
    
    // Update ep rights.
    if ((pcty == PAWN) && (fromSq & BB_OUR_2[co]) && (toSq & BB_OUR_4[co])) {
        epRights = square((fromSq + toSq) / 2); // average gives middle square
    } else {
        epRights = NO_SQ;
    }
    
    // Update castling rights.
    // Castling rights are lost if the king moves.
    if ((pcty == KING) &&
        (fromSq == originalKingSquares[co * NUM_CASTLES / NUM_COLOURS])
       ) {
        castlingRights &= (co == WHITE) ? ~CASTLE_WHITE : ~CASTLE_BLACK;
    } else if (pcty == ROOK) {
        // Castling rights are lost on one side if that rook is moved.
        if (fromSq == originalRookSquares[toIndex(CASTLE_WSHORT)]) {
            castlingRights &= ~CASTLE_WSHORT;
        } else if (fromSq == originalRookSquares[toIndex(CASTLE_WLONG)]) {
            castlingRights &= ~CASTLE_WLONG;
        } else if (fromSq == originalRookSquares[toIndex(CASTLE_BSHORT)]) {
            castlingRights &= ~CASTLE_BSHORT;
        } else if (fromSq == originalRookSquares[toIndex(CASTLE_BLONG)]) {
            castlingRights &= ~CASTLE_BLONG;
        }
    }
    // Practically, castling rights are lost if the relevant rook is removed.
    // Reading the FIDE Laws (2018-01-01) strictly, this is NOT true!
    // (Relevant just for certain classes of fairy chess like Circe).
    // For compatibility with most other chess programs, castling rights are 
    // lost if the relevant rook is removed (to wit: captured or exploded).
    if (isCapture) {
        // Castling rights lost on one side if that rook is removed.
        if (mask & originalRookSquares[toIndex(CASTLE_WSHORT)]) {
            castlingRights &= ~CASTLE_WSHORT;
        } else if (mask & originalRookSquares[toIndex(CASTLE_WLONG)]) {
            castlingRights &= ~CASTLE_WLONG;
        } else if (mask & originalRookSquares[toIndex(CASTLE_BSHORT)]) {
            castlingRights &= ~CASTLE_BSHORT;
        } else if (mask & originalRookSquares[toIndex(CASTLE_BLONG)]) {
            castlingRights &= ~CASTLE_BLONG;
        }
    }
    // Change side to move, and update fifty-move and halfmove counts.
    sideToMove = !sideToMove;
    if (isCapture || (pcty == PAWN)) {
        fiftyMoveNum = 0;
    } else {
        ++fiftyMoveNum;
    }
    ++halfmoveNum;
    return;
}


void AtomicPosition::unmakeMove(Move mv) {
    /// Unmakes (retracts) a move by changing the state of AtomicPosition.
    /// Assumes the move is valid (not necessarily legal).
    /// Must maintain validity of the Position!
    
    // Castling is handled with parent method (same as orthochess).
    if (isCastling(mv)) {
        unmakeCastlingMove(mv);
        // pop explosion stack for consistency
        explosionStack.pop_back();
        return;
    }
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    const Colour co {!sideToMove}; // the side without the move retracts.
    // if move was an atomic capture, then toSq is empty and pc is NO_PIECE.
    const Piece pc {mailbox[toSq]};
    const bool isCapture {pc == NO_PIECE};
    const bool isEp {::isEp(mv)};
    PieceType pcty {NO_PCTY};
    if (!isCapture) {
        pcty = getPieceType(pc);
    }
    // Grab undo information off the stack. Assumes it matches the move called.
    StateInfo undoState {undoStack.back()};
    undoStack.pop_back();
    ExplosionInfo explosion {explosionStack.back()};
    explosionStack.pop_back();
    
    // Revert side to move, castling and ep rights, fifty- and half-move counts.
    sideToMove = !sideToMove;
    castlingRights = undoState.castlingRights;
    epRights = undoState.epRights;
    fiftyMoveNum = undoState.fiftyMoveNum;
    --halfmoveNum;
    
    // Restore all captured and exploded units.
    if (isCapture || isEp) {
        const std::array<Bitboard, NUM_COLOURS> explosionByColour {
            explosion.bbExplosionByColour
        };
        const std::array<Bitboard, NUM_PIECE_TYPES> explosionByType {
            explosion.bbExplosionByType
        };
        
        for (int xco = 0; xco < NUM_COLOURS; ++xco) {
            for (int xpcty = 0; xpcty < NUM_PIECE_TYPES; ++xpcty) {
                Bitboard bbPiece {explosionByColour[xco] & explosionByType[xpcty]};
                while (bbPiece) {
                    Square sq {popLsb(bbPiece)};
                    addPiece(xco, xpcty, sq);
                }
            }
        }
        addPiece(explosion.movedPiece, fromSq);
        
    // If not a capture, just put unit back on original square.
    } else if (isPromotion(mv)) {
        addPiece(co, PAWN, fromSq);
        removePiece(co, pcty, toSq);
    } else {
        addPiece(co, pcty, fromSq);
        removePiece(co, pcty, toSq);
    }
    return;
}