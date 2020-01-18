#include "ortho_position.h"

#include "bitboard.h"
#include "chess_types.h"
#include "move.h"
#include "position.h"

void OrthoPosition::makeMove(Move mv) {
    /// Makes a move by changing the state of Position.
    /// Assumes the move is valid (not necessarily legal).
    /// Must maintain validity of the Position!
    
    // Castling is handled in its own method.
    if (isCastling(mv)) {
        makeCastlingMove(mv);
        return;
    }
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    // The piece that moved
    const Piece pc {mailbox[fromSq]};
    const Colour co {sideToMove}; // assert sideToMove == getPieceColour(pc);
    const PieceType pcty {getPieceType(pc)};
    // Anything on the destination square
    const Piece pcDest {mailbox[toSq]};
    const bool isCapture {pcDest != NO_PIECE};
    
    // Remove piece from fromSq
    removePiece(co, pcty, fromSq);
    
    // Handle regular captures and en passant separately
    if (isCapture) {
        // Regular capture is occurring (not ep)
        PieceType pctyCap {getPieceType(pcDest)};
        removePiece(!co, pctyCap, toSq);
    }
    if (isEp(mv)) {
        // ep capture is occurring, erase the captured pawn
        Square sqEpCap {(co == WHITE) ? shiftS(toSq) : shiftN(toSq)};
        removePiece(!co, PAWN, sqEpCap);
        // No need to store captured piece; ep flag in the Move is sufficient.
    }
    
    // Place piece on toSq
    if (isPromotion(mv)) {
        PieceType pctyPromo {getPromotionType(mv)};
        addPiece(co, pctyPromo, toSq);
    } else {
        addPiece(co, pcty, toSq);
    }
    // Save irreversible state information in struct, *before* altering them.
    undoStack.emplace_back(pcDest, castlingRights, epRights, fiftyMoveNum);
    
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
    if (isCapture && (getPieceType(pcDest) == ROOK)) {
        // Castling rights lost on one side if that rook is removed.
        if (toSq == originalRookSquares[toIndex(CASTLE_WSHORT)]) {
            castlingRights &= ~CASTLE_WSHORT;
        } else if (toSq == originalRookSquares[toIndex(CASTLE_WLONG)]) {
            castlingRights &= ~CASTLE_WLONG;
        } else if (toSq == originalRookSquares[toIndex(CASTLE_BSHORT)]) {
            castlingRights &= ~CASTLE_BSHORT;
        } else if (toSq == originalRookSquares[toIndex(CASTLE_BLONG)]) {
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

void OrthoPosition::unmakeMove(Move mv) {
    /// Unmakes (retracts) a move by changing the state of Position.
    /// Assumes the move is valid (not necessarily legal).
    /// Must maintain validity of the Position!
    
    // Castling is handled separately.
    if (isCastling(mv)) {
        unmakeCastlingMove(mv);
        return;
    }
    const Square fromSq {getFromSq(mv)};
    const Square toSq {getToSq(mv)};
    const Piece pc {mailbox[toSq]};
    const Colour co {!sideToMove}; // the side without the move retracts.
    const PieceType pcty {getPieceType(pc)};
    
    // Grab undo information off the stack. Assumes it matches the move called.
    StateInfo undoState {undoStack.back()};
    undoStack.pop_back();
    
    // Revert side to move, castling and ep rights, fifty- and half-move counts.
    sideToMove = !sideToMove;
    castlingRights = undoState.castlingRights;
    epRights = undoState.epRights;
    fiftyMoveNum = undoState.fiftyMoveNum;
    --halfmoveNum;
    
    // Put unit back on original square.
    if (isPromotion(mv)) {
        addPiece(co, PAWN, fromSq);
        removePiece(co, pcty, toSq);
    } else {
        addPiece(co, pcty, fromSq);
        removePiece(co, pcty, toSq);
    }
    
    // Put back captured piece, if any (en passant handled separately.)
    const Piece pcCap = undoState.capturedPiece;
    if (pcCap != NO_PIECE) {
        addPiece(pcCap, toSq);
    }
    
    // replace en passant captured pawn.
    if (isEp(mv)) {
        Square sqEpCap {(co == WHITE) ? shiftS(toSq) : shiftN(toSq)};
        addPiece(!co, PAWN, sqEpCap);
    }
    return;
}

void OrthoPosition::reset() {
    /// Resets Position to default.
    /// 
    bbByColour.fill(BB_NONE);
    bbByType.fill(BB_NONE);
    mailbox.fill(NO_PIECE);
    sideToMove = WHITE;
    castlingRights = NO_CASTLE;
    epRights = NO_SQ;
    fiftyMoveNum = 0;
    halfmoveNum = 0;
    undoStack.clear();
    return;
}