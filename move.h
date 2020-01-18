#ifndef MOVE_INCLUDED
#define MOVE_INCLUDED

#include "chess_types.h"

#include <string>
#include <vector>

// === move.h ===
// Contains the internal representation of a chess move and associated methods.
// Uses a plain 16-bit integer type (uint16_t).
// Uses Stockfish move encoding: 16 bits to store a move. From least to most
// significant bits:
//
// -from- --to-- sp pr
// 100000 100000 10 10
//
// 6 bits each for the from/to squares (since 2^6 = 64), 2 bits for special move
// flags, 2 bits for promotion piece type flag.
// Special flag: promotion = 1 (01), castling = 2 (10), en passant = 3 (11)
// Promotion type flag: KNIGHT = 00, BISHOP = 01, ROOK = 10, QUEEN = 11.
//
// If extending for variants, promotion type flag can hold extra information (if
// the special flag is not set as promotion, then the bits can be repurposed.)

typedef uint16_t Move;
typedef std::vector<Move> Movelist; // in case needed

// Enum of "special" flags for readability.
enum MoveSpecial {
    MV_NORMAL, MV_PROMOTION, MV_CASTLING, MV_EP
};
inline Square getFromSq(Move mv) {return square(mv & 0x3f);}
inline Square getToSq(Move mv) {return square((mv >> 6) & 0x3f);}
inline int getSpecial(Move mv) {return (mv >> 12) & 0x3;}

inline bool isPromotion(Move mv) {return ((mv >> 12) & 0x3) == MV_PROMOTION;}
inline bool isCastling(Move mv) {return ((mv >> 12) & 0x3) == MV_CASTLING;}
inline bool isEp(Move mv) {return ((mv >> 12) & 0x3) == MV_EP;}

inline PieceType getPromotionType(Move mv) {
    return pieceType(((mv >> 14) & 0x3) + 1); // here N=0 but PieceType N=1.
}

// === "constructors" for Move ===
inline Move buildMove(Square fromSq, Square toSq) {
    Move mv {0};
    mv |= fromSq;
    mv |= (toSq << 6);
    return mv;
};

inline Move buildPromotion(Square fromSq, Square toSq, PieceType pcty) {
    Move mv {0};
    mv |= fromSq;
    mv |= (toSq << 6);
    mv |= (MV_PROMOTION << 12);
    mv |= ((pcty - 1) << 14);
    return mv;
}

inline Move buildCastling(Square fromSq, Square toSq) {
    /// For move encoding, fromSq/toSq are the king's/rook's initial squares.
    ///
    Move mv {0};
    mv |= fromSq;
    mv |= (toSq << 6);
    mv |= (MV_CASTLING << 12);
    return mv;
}

inline Move buildEp(Square fromSq, Square toSq) {
    Move mv {0};
    mv |= fromSq;
    mv |= (toSq << 6);
    mv |= (MV_EP << 12);
    return mv;
}

inline std::string toString(Move mv) {
    /// Conversion to string, useful for debugging
    ///
    if (mv==0) {return "-----  ";}
    std::string outStr;
    outStr.push_back('a' + getFileIdx(getFromSq(mv)));
    outStr += std::to_string(1+getRankIdx(getFromSq(mv))) + "-";
    outStr.push_back('a' + getFileIdx(getToSq(mv)));
    outStr += std::to_string(1+getRankIdx(getToSq(mv)));
    int isp = getSpecial(mv);
    switch (isp) {
        case MV_PROMOTION: {
            int ipt = getPromotionType(mv);
            outStr += "=";
            outStr.push_back(PIECE_CHARS[ipt]);
            break;
        }
        case MV_CASTLING: outStr += "cs"; break;
        case MV_EP: outStr += "ep"; break;
        default: outStr += "  "; break;
    }
    outStr += " ";
    return outStr;
}


#endif //#ifndef MOVE_INCLUDED