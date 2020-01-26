#ifndef CHESS_TYPES_INCLUDED
#define CHESS_TYPES_INCLUDED

#include <string>
#include <stdexcept> //helps debugging
#include <array>

// === chess_types.h ===
// Essentially a lot of global constant variables and functions.
// These are types pertaining directly to position representation, and methods
// to convert between them.
//
// If changing/adding things:
// Keep enums continuous to avoid breaking things.
// Null objects last (unless good reasons), provide a count NUM_[TYPE] outside.


// === Colour ===
// An int representing the colour of the players and units.
enum Colour : int {WHITE, BLACK, NO_COLOUR};
constexpr int NUM_COLOURS {2};

// Converts white to black and vice versa.
inline Colour operator!(Colour co) {
    return static_cast<Colour>(static_cast<int>(co) ^ 1);
}

inline Colour colour(int i) {
    if (WHITE <= i && i <= BLACK) {return static_cast<Colour>(i);}
    else {throw std::range_error("Integer not a valid colour(int i).");}
}


// === PieceType ===
// An int representing the type of chess unit. Sorted in ascending material
// order for ease of remembering.
enum PieceType : int {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    NO_PCTY
};
constexpr int NUM_PIECE_TYPES {6};

inline PieceType pieceType(int i) {
    if (PAWN <= i && i <= KING) {return static_cast<PieceType>(i);}
    else {throw std::range_error("Integer not a valid pieceType(int i).");}
}


// === Piece ===
// An int representing a chess unit with colour and type.
// Conceptually, the set of valid Pieces should be the Cartesian product of
// Colour and PieceTypes (except null values).
enum Piece : int {
    WP, WN, WB, WR, WQ, WK,
    BP, BN, BB, BR, BQ, BK,
    NO_PIECE
};
constexpr int NUM_PIECES {12};

// String of FEN symbols for the pieces, for lookup purposes.
const std::string PIECE_CHARS {"PNBRQKpnbrqk"};

inline Piece piece(int i) {
    if (WP <= i && i <= BK) {return static_cast<Piece>(i);}
    else {throw std::range_error("Integer not a valid piece(int i).");}
}
inline Piece piece(int ico, int ipcty) {
    return static_cast<Piece>(ico*NUM_PIECE_TYPES + ipcty);
}
inline Piece piece(Colour co, PieceType pcType) {
    return static_cast<Piece>(
        static_cast<int>(co)*NUM_PIECE_TYPES + static_cast<int>(pcType)
    );
}
// Useful to get the colour or type of a Piece. Can be optimised (cf Stockfish).
// TODO: error on NO_PIECE.
inline Colour getPieceColour(Piece pc) {
    return colour(static_cast<int>(pc)/static_cast<int>(NUM_PIECE_TYPES));
}
inline PieceType getPieceType(Piece pc) {
    return pieceType(static_cast<int>(pc) % static_cast<int>(NUM_PIECE_TYPES));
}


// === Square ===
// An int representing a square on the chessboard. Indexed from 0 to 63,
// starting from a1 (southwest) and going west to east, then south to north.
enum Square : int {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    NO_SQ
};
constexpr int NUM_SQUARES {64};

inline Square square(int isq) {
    if (SQ_A1 <= isq && isq <= NO_SQ) {return static_cast<Square>(isq);}
    else {throw std::range_error("Integer not a valid square(int isq).");}
}

inline Square square(int x, int y) {
    //returns validated Square from x/y algebraic coords.
    if ((0 <= x && x <= 7) && (0 <= y && y <= 7)) {
        return static_cast<Square>(x + 8*y);
    } else {
        throw std::range_error("x/y coordinates not in range of valid "
                                    "square(int x, int y)");
    }
}

inline int getRankIdx(Square sq) {return static_cast<int>(sq) >> 3;}
inline int getFileIdx(Square sq) {return static_cast<int>(sq) & 7;}

inline Square shiftN(Square sq) {return square(static_cast<int>(sq) + 8);}
inline Square shiftS(Square sq) {return square(static_cast<int>(sq) - 8);}
inline Square shiftForward(Square sq, Colour co) {
    return co == WHITE ? shiftN(sq) : shiftS(sq);
}


// === CastlingRights ===
// Four bits representing castling rights KQkq. The least significant bit is K.
enum CastlingRights : int {
    // Four basic types of castling.
    NO_CASTLE = 0,
    CASTLE_WSHORT = 1,
    CASTLE_WLONG = CASTLE_WSHORT << 1,
    CASTLE_BSHORT = CASTLE_WSHORT << 2,
    CASTLE_BLONG = CASTLE_WSHORT << 3,
    
    // The rest are defined for convenience.
    CASTLE_KINGSIDE = CASTLE_WSHORT | CASTLE_BSHORT,
    CASTLE_QUEENSIDE = CASTLE_WLONG | CASTLE_BLONG,
    CASTLE_WHITE = CASTLE_WSHORT | CASTLE_WLONG,
    CASTLE_BLACK = CASTLE_BSHORT | CASTLE_BLONG,
    
    CASTLE_NOT_WSHORT = CASTLE_BLACK | CASTLE_WLONG,
    CASTLE_NOT_WLONG = CASTLE_BLACK | CASTLE_WSHORT,
    CASTLE_NOT_BSHORT = CASTLE_WHITE | CASTLE_BLONG,
    CASTLE_NOT_BLONG = CASTLE_WHITE | CASTLE_BSHORT,
    
    CASTLE_ALL = CASTLE_WHITE | CASTLE_BLACK
};
constexpr int NUM_CASTLES {4}; // KQkq, 4 types of castling move.

// Lookup array to simplify iterating over the 4 basic types of castling.
constexpr std::array<CastlingRights, NUM_CASTLES> CASTLE_LIST {
    CASTLE_WSHORT, CASTLE_WLONG, CASTLE_BSHORT, CASTLE_BLONG
};
// Bitwise operations on CastlingRights.
inline CastlingRights operator~(CastlingRights cr) {
    return static_cast<CastlingRights>(
            ~static_cast<int>(cr) & static_cast<int>(CASTLE_ALL)
           );
}
inline CastlingRights& operator&=(CastlingRights& cr1, CastlingRights cr2) {
    cr1 = static_cast<CastlingRights>(
            static_cast<int>(cr1) & static_cast<int>(cr2)
          );
    return cr1;
}
inline CastlingRights& operator|=(CastlingRights& cr1, CastlingRights cr2) {
    cr1 = static_cast<CastlingRights>(
            static_cast<int>(cr1) | static_cast<int>(cr2)
          );
    return cr1;
}
inline CastlingRights& operator^=(CastlingRights& cr1, CastlingRights cr2) {
    cr1 = static_cast<CastlingRights>(
            static_cast<int>(cr1) ^ static_cast<int>(cr2)
          );
    return cr1;
}

// converts a "basic" CastlingRights back to integer index.
inline int toIndex(CastlingRights cr) {
    if (cr == CASTLE_WSHORT) {return 0;}
    else if (cr == CASTLE_WLONG) {return 1;}
    else if (cr == CASTLE_BSHORT) {return 2;}
    else if (cr == CASTLE_BLONG) {return 3;}
    else {throw std::range_error("Given CastlingRights cannot be converted"
                                      " to an index.");}
}
// Identifies the side that can make a particular basic castling.
inline Colour toColour(CastlingRights cr) {
    if (cr == CASTLE_WSHORT) {return WHITE;}
    else if (cr == CASTLE_WLONG) {return WHITE;}
    else if (cr == CASTLE_BSHORT) {return BLACK;}
    else if (cr == CASTLE_BLONG) {return BLACK;}
    else {throw std::range_error("Given CastlingRights cannot be converted"
                                      " to a Colour.");}
}

// === Castling squares ===
// In chess and in 960, the post-castling squares of the king and rook are
// fixed.
constexpr std::array<Square, NUM_CASTLES> SQ_K_TO {SQ_G1, SQ_C1, SQ_G8, SQ_C8};
constexpr std::array<Square, NUM_CASTLES> SQ_R_TO {SQ_F1, SQ_D1, SQ_F8, SQ_D8};

#endif //#ifndef CHESS_TYPES_INCLUDED