#include "position.h"
#include "chess_types.h"
#include "bitboard.h"

#include <array>
#include <cctype>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

void Position::reset() {
    /// Resets Position to default. (Might want to rewrite explicitly).
    *this = Position(); // cheaty way out; calls default constructor!
    mailbox.fill(NO_PIECE);
    return;
}

Position& Position::fromFen(const std::string& fenStr) {
    /// Reads a FEN string and sets up the Position accordingly.
    // TODO: Make it more robust in terms of accepting input.
    std::istringstream fenSs(fenStr);
    fenSs >> std::noskipws;
    unsigned char token {0};
    int isq {static_cast<int>(SQ_A8)};
    size_t idx {std::string::npos};
    
    // Clear board.
    reset();
    // Read physical position
    while ((fenSs >> token) && !isspace(token)) {
        if (isdigit(token)) {
            isq += static_cast<int>(token - '0'); // char '0' != int 0
        } else if (token == '/') {
            isq -= 8 + 8; // "square 9" of rank wraps up; must go down 2 ranks.
        } else if ((idx = PIECE_CHARS.find(token)) != std::string::npos) {
            addPiece(piece(idx), square(isq));
            ++isq;
        } else {
            throw std::runtime_error("Unknown character in FEN position.");
        }
    }
    // while loop ate 1 whitespace. Assume only 1 whitespace between blocks.
    // Read side to move; assumes only 1 character.
    if (fenSs >> token) {
        switch (std::tolower(token)) {
            case 'w': {sideToMove = WHITE; break;}
            case 'b': {sideToMove = BLACK; break;}
            default: {
                throw std::runtime_error("Unknown side to move in FEN.");
                break;
            }
        }
    }
    fenSs >> token; // Assumes (and eats) 1 whitespace.
    // Read castling rights.
    while ((fenSs >> token) && !isspace(token)) {
        switch (token) {
            case 'K': {castlingRights |= CASTLE_WSHORT; break;}
            case 'Q': {castlingRights |= CASTLE_WLONG; break;}
            case 'k': {castlingRights |= CASTLE_BSHORT; break;}
            case 'q': {castlingRights |= CASTLE_BLONG; break;}
            case '-': {castlingRights = NO_CASTLE; break;}
            default: {
                throw std::runtime_error("Unknown castling rights in FEN.");
            }
        }
    }
    // while loop ate 1 whitespace.
    // Read en passant rights (one square).
    while ((fenSs >> token) && !isspace(token)) {
        if (token == '-') {epRights = NO_SQ; break;}
        else if (('a' <= token) && (token <= 'h')) {
            int x = static_cast<int>(token - 'a');
            if ((fenSs >> token) && ('1' <= token) && (token <= '8')) {
                int y = static_cast<int>(token - '1');
                epRights = square(x, y);
            }
        } else {
            throw std::runtime_error("Unknown en passant rights in FEN.");
        }
    }
    // while loop ate 1 whitespace.
    // Read fifty-move and fullmove counters (assumes each is one integer)
    int fullmoveNum {1};
    fenSs >> std::skipws >> fiftyMoveNum >> fullmoveNum;
    // Converting a fullmove number to halfmove number.
    // Halfmove 0 = Fullmove 1 + white to move.
    halfmoveNum = (sideToMove == WHITE) ? 2 * fullmoveNum - 2: 2 * fullmoveNum - 1;
    
    return *this;
}


void Position::makeMove(Move mv) {
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
    StateInfo undoState {pcDest, castlingRights, epRights, fiftyMoveNum};
    // std::unique_ptr<StateInfo> uS = std::make_unique<StateInfo>(pcDest, castlingRights, epRights, fiftyMoveNum);
    // StateInfo undoState = *uS;
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


void Position::unmakeMove(Move mv) {
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
    const Colour co {!sideToMove}; // retractions are by the side without the move.
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


std::string Position::pretty() const {
    /// Makes a human-readable string of the board represented by Position.
    std::array<Piece, NUM_SQUARES> posArr {};
    posArr.fill(NO_PIECE);
    std::string strOut {"+--------+\n"};
    
    // Reads bitboards into single FEN-ordered array of Pieces.
    // Could also rewrite to use mailbox instead of bitboard fields.
    for (int n = 0; n < NUM_SQUARES; ++n) {
        int idx_fen = 56 - 8*(n/8) + n%8; // mapping: bitboard to FEN ordering
        Square sq = square(n);
        // Read pieces from bitboards.
        for (int ipcty = 0; ipcty < NUM_PIECE_TYPES; ++ipcty) {
            if (bbByType[ipcty] & sq) {
                if (bbByColour[WHITE] & sq) {
                    posArr[idx_fen] = piece(WHITE, ipcty);
                    break;
                } else if (bbByColour[BLACK] & sq) {
                    posArr[idx_fen] = piece(BLACK, ipcty);
                    break;
                } else {
                    // Piecetype says there's a piece, colour says no.
                    throw std::runtime_error("Position bitboards not"
                                             "self-consistent. (bbByType has a"
                                             " piece, but not bbByColour)");
                }
            }
        }
    }
    // loops over FEN-ordered array to print
    for (int idx = 0; idx < NUM_SQUARES; ++idx) {
        if (idx % 8 == 0) {strOut.push_back('|');}
        Piece pc = posArr[idx];
        switch (pc) {
            case WP: {strOut.push_back('P'); break;}
            case WN: {strOut.push_back('N'); break;}
            case WB: {strOut.push_back('B'); break;}
            case WR: {strOut.push_back('R'); break;}
            case WQ: {strOut.push_back('Q'); break;}
            case WK: {strOut.push_back('K'); break;}
            case BP: {strOut.push_back('p'); break;}
            case BN: {strOut.push_back('n'); break;}
            case BB: {strOut.push_back('b'); break;}
            case BR: {strOut.push_back('r'); break;}
            case BQ: {strOut.push_back('q'); break;}
            case BK: {strOut.push_back('k'); break;}
            default: {strOut.push_back('.'); break;}
        }
        if (idx % 8 == 7) {strOut += "|\n";}
    }
    strOut += "+--------+\n";
    // Output state info (useful for debugging)
    strOut += "sideToMove: " + std::to_string(sideToMove) + "\n";
    strOut += "castlingRights: " + std::to_string(castlingRights) + "\n";
    strOut += "epRights: " + std::to_string(epRights) + "\n";
    strOut += "fiftyMoveNum: " + std::to_string(fiftyMoveNum) + "\n";
    strOut += "halfmoveNum: " + std::to_string(halfmoveNum) + "\n";
    return strOut;
}


// === Helper methods (private) ===

void Position::addPiece(Piece pc, Square sq) {
    // Does not maintain position validity. Do not call with NO_PIECE.
    Colour co {getPieceColour(pc)};
    PieceType pcty {getPieceType(pc)};
    bbByColour[co] ^= sq;
    bbByType[pcty] ^= sq;
    mailbox[sq] = pc;
    return;
}

void Position::addPiece(Colour co, PieceType pcty, Square sq) {
    // Does not maintain position validity by itself.
    bbByColour[co] ^= sq;
    bbByType[pcty] ^= sq;
    mailbox[sq] = piece(co, pcty);
}

void Position::addPiece(int ico, int ipcty, Square sq) {
    // Does not maintain position validity by itself.
    bbByColour[ico] ^= sq;
    bbByType[ipcty] ^= sq;
    mailbox[sq] = piece(ico, ipcty);
}

void Position::removePiece(Colour co, PieceType pcty, Square sq) {
    // Does not maintain position validity by itself.
    bbByColour[co] ^= sq;
    bbByType[pcty] ^= sq;
    mailbox[sq] = NO_PIECE;
}

void Position::makeCastlingMove(Move mv) {
    // assert isCastling(mv);
    const Colour co {sideToMove};
    const Square sqKFrom {getFromSq(mv)};
    const Square sqRFrom {getToSq(mv)};
    Square sqKTo{NO_SQ};
    Square sqRTo{NO_SQ};
    // By square encoding, further east = higher number
    if (sqKFrom > sqRFrom) {
        // King east of rook, i.e. west castling.
        if (co == WHITE) {
            sqKTo = SQ_K_TO[toIndex(CASTLE_WLONG)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_WLONG)];
        } else {
            sqKTo = SQ_K_TO[toIndex(CASTLE_BLONG)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_BLONG)];
        }
    } else {
        // King west of rook, i.e. east castling.
        if (co == WHITE) {
            sqKTo = SQ_K_TO[toIndex(CASTLE_WSHORT)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_WSHORT)];
        } else {
            sqKTo = SQ_K_TO[toIndex(CASTLE_BSHORT)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_BSHORT)];
        }
    }
    // Remove king and rook, and place them at their final squares.
    bbByColour[co] ^= (sqKFrom | sqRFrom | sqKTo | sqRTo);
    bbByType[KING] ^= (sqKFrom | sqKTo);
    bbByType[ROOK] ^= (sqRFrom | sqRTo);
    mailbox[sqKFrom] = NO_PIECE;
    mailbox[sqRFrom] = NO_PIECE;
    mailbox[sqKTo] = piece(co, KING);
    mailbox[sqRTo] = piece(co, ROOK);
    
    // Save irreversible information in struct, *before* altering them.
    const StateInfo undoState {NO_PIECE, castlingRights,
                               epRights, fiftyMoveNum};
    undoStack.push_back(undoState);
    // Update ep and castling rights.
    epRights = NO_SQ;
    castlingRights &= (co == WHITE) ? ~CASTLE_WHITE : ~CASTLE_BLACK;
    // Change side to move, and update fifty-move and halfmove counts.
    sideToMove = !sideToMove;
    ++fiftyMoveNum;
    ++halfmoveNum;
    return;
}


void Position::unmakeCastlingMove(Move mv) {
    // assert isCastling(mv);
    // Establish castling start/end squares (where the king and rook were/are).
    const Colour co {!sideToMove}; // retraction is by nonmoving side.
    const Square sqKFrom {getFromSq(mv)};
    const Square sqRFrom {getToSq(mv)};
    Square sqKTo{NO_SQ};
    Square sqRTo{NO_SQ};
    // By square encoding, further east = higher number
    if (sqKFrom > sqRFrom) {
        // King east of rook, i.e. west castling.
        if (co == WHITE) {
            sqKTo = SQ_K_TO[toIndex(CASTLE_WLONG)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_WLONG)];
        } else {
            sqKTo = SQ_K_TO[toIndex(CASTLE_BLONG)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_BLONG)];
        }
    } else {
        // King west of rook, i.e. east castling.
        if (co == WHITE) {
            sqKTo = SQ_K_TO[toIndex(CASTLE_WSHORT)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_WSHORT)];
        } else {
            sqKTo = SQ_K_TO[toIndex(CASTLE_BSHORT)];
            sqRTo = SQ_R_TO[toIndex(CASTLE_BSHORT)];
        }
    }
    // Grab undo information off the stack. Assumes it matches the move called.
    StateInfo undoState {undoStack.back()};
    undoStack.pop_back();
    
    // Revert side to move, castling and ep rights, fifty- and half-move counts.
    sideToMove = !sideToMove;
    castlingRights = undoState.castlingRights;
    epRights = undoState.epRights;
    fiftyMoveNum = undoState.fiftyMoveNum;
    halfmoveNum--;
    
    // Put king and rook back on their original squares.
    bbByColour[co] ^= (sqKFrom | sqRFrom | sqKTo | sqRTo);
    bbByType[KING] ^= (sqKFrom | sqKTo);
    bbByType[ROOK] ^= (sqRFrom | sqRTo);
    mailbox[sqKFrom] = piece(co, KING);
    mailbox[sqRFrom] = piece(co, ROOK);
    mailbox[sqKTo] = NO_PIECE;
    mailbox[sqRTo] = NO_PIECE;
    return;
}
