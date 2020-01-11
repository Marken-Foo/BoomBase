#include "atomic_position.h"
#include "atomic_capture_masks.h"

#include <array>
#include <memory>

void AtomicPosition::makeMove(Move mv) {
    /// Makes a move by changing the state of AtomicPosition.
    /// Assumes the move is valid (not necessarily legal).
    /// Must maintain validity of the Position!
    
    // Castling is handled in its own method (same as orthochess).
    if (isCastling(mv)) {
        makeCastlingMove(mv);
        return;
    }
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    const Piece pc {mailbox[fromSq]};
    const Colour co {sideToMove}; // assert sideToMove == getPieceColour(pc);
    const PieceType pcty {getPieceType(pc)};
    const Piece pcDest {mailbox[toSq]};
    const bool isCapture {pcDest != NO_PIECE};
    const bool isEp {::isEp(mv)};
    const Bitboard mask = atomicMasks[toSq];
    // Stores explosion information
    std::array<Bitboard, NUM_COLOURS> explodedByColour {};
    std::array<Bitboard, NUM_PIECE_TYPES> explodedByType {};
    
    // (Not castling) begin updating position; remove moved piece.
    removePiece(co, pcty, fromSq);
    
    if (isCapture || isEp) {
        // Could be capture, promotion capture, or en passant.
        // Record, then remove all units exploded.
        // (Manipulate bitboards directly; mailbox is handled shortly after.)
        for (int i = 1; i < NUM_PIECE_TYPES; ++i) {
            // Adjacent pawns are *not* exploded (note index of for loop!).
            explodedByType[i] ^= bbByType[i] & mask;
            bbByType[i] &= ~explodedByType[i];
        }
        for (int j = 0; j < NUM_COLOURS; ++j) {
            // Adjacent pawns are *not* exploded.
            explodedByColour[j] ^= bbByColour[j] & mask & ~bbByType[PAWN];
            bbByColour[j] &= ~explodedByColour[j];
        }
        // But directly captured or en passant'd pawn, *is* exploded.
        if (getPieceType(pcDest) == PAWN && pcDest != NO_PIECE) {
            explodedByColour[!co] ^= toSq;
            explodedByType[PAWN] ^= toSq;
            bbByColour[!co] ^= toSq;
            bbByType[PAWN] ^= toSq;
        }
        if (isEp) {
            Square sqEpCap {(co == WHITE) ? shiftS(toSq) : shiftN(toSq)};
            explodedByColour[!co] ^= sqEpCap;
            explodedByType[PAWN] ^= sqEpCap;
            bbByColour[!co] ^= sqEpCap;
            bbByType[PAWN] ^= sqEpCap;
        }
        // (Now it is convenient to update the mailbox.)
        Bitboard bbExploded {explodedByColour[WHITE] | explodedByColour[BLACK]};
        while (bbExploded) {
            Square sq = popLsb(bbExploded);
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
    // TODO: REFACTOR OUT THIS PART (AND IN UNMAKE) HERE AND IN POSITION.CPP
    // Since castling still puts in/takes out a regular StateInfo undoState
    
    AtomicStateInfo undoState {pcDest, castlingRights, epRights, fiftyMoveNum,
                               pc, explodedByColour, explodedByType};
    undoStack.push_back(undoState);
    
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
    // considered lost if the relevant rook is captured.
    // (For atomic chess: or if there is an adjacent explosion.)
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
    
    // if isCapture, set explodedByColour and explodedByType (excluding radial pawns, including central captured pawn)
    // (if isEp, need to included targeted pawn.)
    
    // else is not a capture.
    // then do normal move stuff, including promo (noncap).
    
    // then store stateinfo in struct.
    // so storing in struct is zero bitboards if not capture, and correct if is capture.
    // for unmake, just need to XOR the exploded bitboards back on.
}


void AtomicPosition::unmakeMove(Move mv) {
    /// Unmakes (retracts) a move by changing the state of AtomicPosition.
    /// Assumes the move is valid (not necessarily legal).
    /// Must maintain validity of the Position!
    
    // Castling is handled separately (same as orthochess).
    if (isCastling(mv)) {
        unmakeCastlingMove(mv);
        return;
    }
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    const Colour co {!sideToMove}; // retractions are by the side without the move.
    const Piece pc {mailbox[toSq]}; // if move was an atomic capture, then toSq is empty, i.e. this is NO_PIECE.
    const bool isCapture {pc == NO_PIECE};
    const bool isEp {::isEp(mv)};
    PieceType pcty {NO_PCTY};
    if (!isCapture) {
        pcty = getPieceType(pc);
    }
    // Grab undo information off the stack. Assumes it matches the move called.
    AtomicStateInfo undoState {undoStack.back()};
    undoStack.pop_back();
    // std::unique_ptr<AtomicStateInfo> ptrUndoState = std::make_unique<AtomicStateInfo>(undoStack.back());
    // undoStack.pop_back();
    // AtomicStateInfo undoState {*ptrUndoState};
    
    // Revert side to move, castling and ep rights, fifty- and half-move counts.
    sideToMove = !sideToMove;
    castlingRights = undoState.castlingRights;
    epRights = undoState.epRights;
    fiftyMoveNum = undoState.fiftyMoveNum;
    --halfmoveNum;
    
    
    // Restore all captured and exploded units.
    if (isCapture || isEp) {
        const std::array<Bitboard, NUM_COLOURS> explodedByColour {undoState.bbExplodedByColour};
        const std::array<Bitboard, NUM_PIECE_TYPES> explodedByType {undoState.bbExplodedByType};
        
        for (int xco = 0; xco < NUM_COLOURS; ++xco) {
            for (int xpcty = 0; xpcty < NUM_PIECE_TYPES; ++xpcty) {
                Bitboard bbPiece {explodedByColour[xco] & explodedByType[xpcty]};
                while (bbPiece) {
                    Square sq {popLsb(bbPiece)};
                    addPiece(xco, xpcty, sq);
                }
            }
        }
        addPiece(undoState.movedPiece, fromSq);
        
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