#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include "chess_types.h"
#include "bitboard.h"
#include "move.h"

#include <string>
#include <array>
#include <deque>

// === position.h ===
// Defines the internal representation of a chess position.


// === StateInfo ===
// A struct for irreversible info about the position, for unmaking moves.
struct StateInfo {
    StateInfo() = default;
    
    Piece capturedPiece {NO_PIECE};
    CastlingRights castlingRights {NO_CASTLE};
    Square epRights {NO_SQ};
    int fiftyMoveNum {0};
};

// === Position class ===
// It knows the:
// - Piece location, in bitboard and mailbox form
// - Side to move
// - Castling rights
// - En passant rights
// - Fifty move counter
// - Halfmove counter (halfmoves elapsed since start of game).
//
// In addition, it can make/unmake Moves that are given to it, changing its
// state accordingly.

class Position {
    public:
        Position() = default;
        void reset();
        // --- Initialise from FEN string ---
        Position& fromFen(const std::string& fenStr);
        
        // --- Getters ---        
        Bitboard getUnitsBb(Colour co, PieceType pcty) const {
            return bbByColour[co] & bbByType[pcty];
        }
        Bitboard getUnitsBb(Colour co) const {return bbByColour[co];}
        Bitboard getUnitsBb(PieceType pcty) const {return bbByType[pcty];}
        Bitboard getUnitsBb() const {
            Bitboard bb {0};
            for (int ico = 0; ico < NUM_COLOURS; ++ico) {bb |= bbByColour[ico];}
            return bb;
        }
        std::array<Piece, NUM_SQUARES> getMailbox() const {return mailbox;}
        
        Colour getSideToMove() const {return sideToMove;}
        CastlingRights getCastlingRights() const {return castlingRights;}
        Square getEpSq() const {return epRights;}
        
        // getters for info to execute castling
        // only to be called with "basic" castling rights K, Q, k, or q.
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
        
        // --- Move making/unmaking ---
        void makeMove(Move mv);
        void unmakeMove(Move mv);
        
        // --- Other ---
        // Turn position to printable string
        std::string pretty() const;
        
        
    private:
        // --- Class data members ---
        // Ensure state is updated correctly to maintain a valid Position!
        std::array<Bitboard, NUM_COLOURS> bbByColour {};
        std::array<Bitboard, NUM_PIECE_TYPES> bbByType {};
        std::array<Piece, NUM_SQUARES> mailbox {};
        // Game state information
        Colour sideToMove {WHITE};
        CastlingRights castlingRights {NO_CASTLE};
        Square epRights {NO_SQ};
        int fiftyMoveNum {0};
        int halfmoveNum {0};
        
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
        void makeCastlingMove(Move mv);
        void unmakeCastlingMove(Move mv);
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
    
    // TODO: replace with zobrist hash or something
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