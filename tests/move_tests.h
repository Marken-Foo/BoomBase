#include "move.h"

#include <cxxtest/TestSuite.h>

class TestSuiteMove: public CxxTest::TestSuite {
    public:
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
    
    // Nf3-g5 (21 to 38)
    // 00 00 100110 010101
    Move f3g5 = 0x995;
    
    // Promotion e7-e8Q (52 to 60)
    // 11 01 111100 110100
    Move e7e8Q = 0xDF34;
    
    // Castling with Ke1 and Rh1
    // 00 10 000111 000100
    Move w00 = 0x21C4;
    
    // En passant Black ...bxa4 ep (33 to 24)
    // 00 11 011000 100001
    Move bxa4ep = 0x3621;
    
    void testBuildF3G5() {
        Move mv = buildMove(SQ_F3, SQ_G5);
        TS_ASSERT_EQUALS(mv, f3g5);
    }
    void testBuildE7E8Q() {
        Move mv = buildPromotion(SQ_E7, SQ_E8, QUEEN);
        TS_ASSERT_EQUALS(mv, e7e8Q);
    }
    void testBuildE1H1Castling() {
        Move mv = buildCastling(SQ_E1, SQ_H1);
        TS_ASSERT_EQUALS(mv, w00);
    }
    void testBuildB5A4Ep() {
        Move mv = buildEp(SQ_B5, SQ_A4);
        TS_ASSERT_EQUALS(mv, bxa4ep);
    }
    void testFromSqf3g5() {
        Square sq = getFromSq(f3g5);
        TS_ASSERT_EQUALS(sq, SQ_F3);
    }
    void testToSqf3g5() {
        Square sq = getToSq(f3g5);
        TS_ASSERT_EQUALS(sq, SQ_G5);
    }
    void testGetSpecialNormal() {
        TS_ASSERT_EQUALS(getSpecial(f3g5), MV_NORMAL);
    }
    void testGetSpecialPromotion() {
        TS_ASSERT_EQUALS(getSpecial(e7e8Q), MV_PROMOTION);
    }
    void testGetSpecialCastling() {
        TS_ASSERT_EQUALS(getSpecial(w00), MV_CASTLING);
    }
    void testGetSpecialEp() {
        TS_ASSERT_EQUALS(getSpecial(bxa4ep), MV_EP);
    }
    
    // testing isPromotion()
    void testNormalIsPromotion() {
        TS_ASSERT(!isPromotion(f3g5));
    }
    void testPromotionIsPromotion() {
        TS_ASSERT(isPromotion(e7e8Q));
    }
    void testCastlingIsPromotion() {
        TS_ASSERT(!isPromotion(w00));
    }
    void testEpIsPromotion() {
        TS_ASSERT(!isPromotion(bxa4ep));
    }
    
    // testing isCastling()
    void testNormalIsCastling() {
        TS_ASSERT(!isCastling(f3g5));
    }
    void testPromotionIsCastling() {
        TS_ASSERT(!isCastling(e7e8Q));
    }
    void testCastlingIsCastling() {
        TS_ASSERT(isCastling(w00));
    }
    void testEpIsCastling() {
        TS_ASSERT(!isCastling(bxa4ep));
    }
    
    // testing isEp()
    void testNormalIsEp() {
        TS_ASSERT(!isEp(f3g5));
    }
    void testPromotionIsEp() {
        TS_ASSERT(!isEp(e7e8Q));
    }
    void testCastlingIsEp() {
        TS_ASSERT(!isEp(w00));
    }
    void testEpIsEp() {
        TS_ASSERT(isEp(bxa4ep));
    }
    void testGetPromotionType() {
        TS_ASSERT_EQUALS(getPromotionType(e7e8Q), QUEEN);
    }
};