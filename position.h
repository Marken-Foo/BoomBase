#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include "bitboard.h"
#include "chess_types.h"
#include "move.h"

#include <array>
#include <deque>
#include <string>

// === position.h ===
// An abstract class defining the internal representation of a "physical" chess
// position, and able to make/unmake moves supplied by an external source.
//
// It knows the:
// - Piece location, in bitboard and mailbox form
// - Side to move
// - Castling rights
// - En passant rights
// - Fifty move counter
// - Halfmove counter (halfmoves elapsed since start of game).
//
// In addition, it can make/unmake Moves given to it, changing its state
// accordingly.
// Ensure state is updated correctly to maintain a valid Position!

class Position {
    // Making/unmaking moves and resetting all member variables (including
    // subclass-specific ones) are not part of the physical position, and depend
    // on the variant.
    public:
    virtual void makeMove(Move mv) = 0;
    virtual void unmakeMove(Move mv) = 0;
    virtual void reset() = 0;
    
    public:
    Position() {
        mailbox.fill(NO_PIECE);
    }
    
    Position& fromFen(const std::string& fenStr);
    
    // --- Getters ---        
    Bitboard getUnitsBb(Colour co, PieceType pcty) const {
        return bbByColour[co] & bbByType[pcty];
    }
    Bitboard getUnitsBb(Colour co) const {
        return bbByColour[co];
    }
    Bitboard getUnitsBb(PieceType pcty) const {
        return bbByType[pcty];
    }
    Bitboard getUnitsBb() const {
        Bitboard bb {0};
        for (int ico = 0; ico < NUM_COLOURS; ++ico) {bb |= bbByColour[ico];}
        return bb;
    }
    std::array<Piece, NUM_SQUARES> getMailbox() const {
        return mailbox;
    }
    Piece getMailbox(Square sq) const {
        return mailbox[sq];
    }
    
    Colour getSideToMove() const {
        return sideToMove;
    }
    CastlingRights getCastlingRights() const {
        return castlingRights;
    }
    Square getEpSq() const {
        return epRights;
    }
    bool isVariantEnd() const {
        return variantEnd;
    }
    
    // Exposed for convenience for legal move checking.
    // Intentionally restricted to king to prevent temptation to overuse method.
    // DOES NOT MAINTAIN POSITION VALIDITY UNLESS USED TOGETHER.
    void ghostKing(Colour co, Square sq) {
        removePiece(co, KING, sq);
        return;
    }
    void unghostKing(Colour co, Square sq) {
        addPiece(co, KING, sq);
        return;
    }
    
    // getters for info to execute castling
    // **only to be called with "basic" castling rights K, Q, k, or q!**
    Bitboard getCastlingRookMask(CastlingRights cr) const {
        return castlingRookMasks[toIndex(cr)];
    }
    Bitboard getCastlingKingMask(CastlingRights cr) const {
        return castlingKingMasks[toIndex(cr)];
    }
    Square getOrigRookSq(CastlingRights cr) const {
        return originalRookSquares[toIndex(cr)];
    }
    Square getOrigKingSq(CastlingRights cr) const {
        return originalKingSquares[toIndex(cr)];
    }
    
    // Turn position to printable string
    std::string pretty() const;
    
    
    protected:
    struct StateInfo;
    // --- Class data members ---
    std::array<Bitboard, NUM_COLOURS> bbByColour {};
    std::array<Bitboard, NUM_PIECE_TYPES> bbByType {};
    std::array<Piece, NUM_SQUARES> mailbox {};
    // Game state information
    Colour sideToMove {WHITE};
    CastlingRights castlingRights {NO_CASTLE};
    Square epRights {NO_SQ};
    int fiftyMoveNum {0};
    int halfmoveNum {0};
    // for variant use
    bool variantEnd {false};
    // Stack of unrestorable information for unmaking moves.
    std::deque<StateInfo> undoStack {};
    
    // --- Castling information ---
    // Information to help with validating/making castling moves.
    // Indexed in order KQkq like FEN.
    // (Rewrite code if 960!)
    std::array<Square, NUM_CASTLES> originalRookSquares {
        SQ_H1, SQ_A1, SQ_H8, SQ_A8
    };
    std::array<Square, NUM_CASTLES> originalKingSquares {
        SQ_E1, SQ_E1, SQ_E8, SQ_E8
    };
    // Squares the rook passes through, inclusive.
    std::array<Bitboard, NUM_CASTLES> castlingRookMasks {
        BB_NONE | SQ_F1 | SQ_G1 | SQ_H1,
        BB_NONE | SQ_A1 | SQ_B1 | SQ_C1 | SQ_D1,
        BB_NONE | SQ_F8 | SQ_G8 | SQ_H8,
        BB_NONE | SQ_A8 | SQ_B8 | SQ_C8 | SQ_D8
    };
    // Squares the king passes through, inclusive.
    std::array<Bitboard, NUM_CASTLES> castlingKingMasks {
        BB_NONE | SQ_E1 | SQ_F1 | SQ_G1,
        BB_NONE | SQ_C1 | SQ_D1 | SQ_E1,
        BB_NONE | SQ_E8 | SQ_F8 | SQ_G8,
        BB_NONE | SQ_C8 | SQ_D8 | SQ_E8
    };
    
    
    // --- Helper methods ---
    void addPiece(Piece pc, Square sq);
    void addPiece(Colour co, PieceType pcty, Square sq);
    void addPiece(int ico, int ipcty, Square sq);
    void removePiece(Colour co, PieceType pcty, Square sq);
    void makeCastlingMove(Move mv);
    void unmakeCastlingMove(Move mv);
    
    // A struct for irreversible info about the position, for unmaking moves.
    struct StateInfo {
        StateInfo(Piece pc, CastlingRights cr, Square sq, int num50)
            : capturedPiece{pc}
            , castlingRights{cr}
            , epRights{sq}
            , fiftyMoveNum{num50}
                { }
        
        Piece capturedPiece {NO_PIECE};
        CastlingRights castlingRights {NO_CASTLE};
        Square epRights {NO_SQ};
        int fiftyMoveNum {0};
    };
};

inline bool operator==(const Position& lhs, const Position& rhs) {
    /// Default operator override.
    /// Two Positions are the same if they are the same "chess position". This
    /// means the piece locations (mailbox and bitboards) are identical, the
    /// side to move, castling rights, and en passant rights are identical.
    /// 
    /// Note: Positions differing by an en passant capture which is pseudolegal
    /// but not legal due to e.g. a pin, are considered different here but
    /// identical under FIDE.
    /// Note: Positions of which are "physically" identical but of different
    /// variants are considered identical.
    
    if (lhs.getMailbox() != rhs.getMailbox()) {
        return false;
    }
    if (lhs.getUnitsBb(WHITE) != rhs.getUnitsBb(WHITE) ||
        lhs.getUnitsBb(BLACK) != rhs.getUnitsBb(BLACK)) {
        return false;
    }
    if (lhs.getUnitsBb(PAWN) != rhs.getUnitsBb(PAWN) ||
        lhs.getUnitsBb(KNIGHT) != rhs.getUnitsBb(KNIGHT) ||
        lhs.getUnitsBb(BISHOP) != rhs.getUnitsBb(BISHOP) ||
        lhs.getUnitsBb(ROOK) != rhs.getUnitsBb(ROOK) ||
        lhs.getUnitsBb(QUEEN) != rhs.getUnitsBb(QUEEN) ||
        lhs.getUnitsBb(KING) != rhs.getUnitsBb(KING)) {
        return false;
    }
    if (lhs.getSideToMove() != rhs.getSideToMove() ||
        lhs.getCastlingRights() != rhs.getCastlingRights() ||
        lhs.getEpSq() != rhs.getEpSq()) {
        return false;
    }
    return true;
}
inline bool operator!=(const Position& lhs, const Position& rhs) {
    return !(lhs == rhs);
}

#endif //#ifndef POSITION_INCLUDED