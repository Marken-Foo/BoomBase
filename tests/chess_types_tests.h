#include "chess_types.h"

#include <cxxtest/TestSuite.h>

class TestSuiteColours: public CxxTest::TestSuite {
    public:
    // colour convention
    void testWhite() {
        TS_ASSERT_EQUALS(WHITE, 0);
    }
    void testBlack() {
        TS_ASSERT_EQUALS(BLACK, 1);
    }
    // ! operator to toggle colour
    void testWhiteToBlack() {
        TS_ASSERT_EQUALS((!WHITE), BLACK);
    }
    void testBlackToWhite() {
        TS_ASSERT_EQUALS(WHITE, (!BLACK));
    }
    void testNegateBlackTwice() {
        TS_ASSERT_EQUALS((!!BLACK), BLACK);
    }
};

class TestSuitePieces: public CxxTest::TestSuite {
    public:
    void testMakeKnight() {
        PieceType pcty = pieceType(1);
        TS_ASSERT_EQUALS(pcty, KNIGHT);
    }
    void testMakeBk(void) {
        Piece pc = piece(11);
        TS_ASSERT_EQUALS(pc, BK);
    }
    void testMakeWqFromIdx() {
        Piece pc = piece(0, 4);
        TS_ASSERT_EQUALS(pc, WQ);
    }
    void testMakeWb() {
        Piece pc = piece(WHITE, BISHOP);
        TS_ASSERT_EQUALS(pc, WB);
    }
    void testMakeBp() {
        Piece pc = piece(BLACK, PAWN);
        TS_ASSERT_EQUALS(pc, BP);
    }
    void testGetColourBq() {
        Colour co = getPieceColour(BQ);
        TS_ASSERT_EQUALS(co, BLACK);
    }
    void testGetTypeWn() {
        PieceType pcty = getPieceType(WN);
        TS_ASSERT_EQUALS(pcty, KNIGHT);
    }
};

class TestSuiteSquares: public CxxTest::TestSuite {
    public:
    void testG2Is14() {
        // Confirm square numbering order (a1=0, increasing W to E, S to N.)
        TS_ASSERT_EQUALS(SQ_G2, 14);
    }
    void testSquareFrom14() {
        Square sq = square(14);
        TS_ASSERT_EQUALS(sq, SQ_G2);
    }
    void testE8FromIndices() {
        Square sq = square(4, 7);
        TS_ASSERT_EQUALS(sq, SQ_E8);
    }
    void testRankIdxOfD1() {
        int iy = getRankIdx(SQ_D1);
        TS_ASSERT_EQUALS(iy, 0);
    }
    void testFileIdxOfH6() {
        int ix = getFileIdx(SQ_H6);
        TS_ASSERT_EQUALS(ix, 7);
    }
    void testShiftNC3() {
        Square sq = shiftN(SQ_C3);
        TS_ASSERT_EQUALS(sq, SQ_C4);
    }
    void testShiftSB7() {
        Square sq = shiftS(SQ_B7);
        TS_ASSERT_EQUALS(sq, SQ_B6);
    }
};

class TestSuiteCastlingRights: public CxxTest::TestSuite {
    public:
    void testCrWLongNegation() {
        TS_ASSERT_EQUALS((~CASTLE_WLONG), CASTLE_NOT_WLONG);
    }
    void testCrKingsideNegation() {
        TS_ASSERT_EQUALS((~CASTLE_KINGSIDE), CASTLE_QUEENSIDE);
    }
    void testCrBlackNegation() {
        TS_ASSERT_EQUALS((~CASTLE_BLACK), CASTLE_WHITE);
    }
    void testCrKingsideAndWhite() {
        CastlingRights cr = CASTLE_KINGSIDE;
        cr &= CASTLE_WHITE;
        TS_ASSERT_EQUALS(cr, CASTLE_WSHORT);
    }
    void testCrNotWlongAndBlack() {
        CastlingRights cr = CASTLE_NOT_WLONG;
        cr &= CASTLE_BLACK;
        TS_ASSERT_EQUALS(cr, CASTLE_BLACK);
    }
    void testCrWhiteOrBlack() {
        CastlingRights cr = CASTLE_WHITE;
        cr |= CASTLE_BLACK;
        TS_ASSERT_EQUALS(cr, CASTLE_ALL);
    }
    void testCrWhiteXorKingside() {
        CastlingRights cr = CASTLE_WHITE;
        cr ^= CASTLE_KINGSIDE;
        TS_ASSERT_EQUALS(cr, (CASTLE_WLONG | CASTLE_BSHORT));
    }
};