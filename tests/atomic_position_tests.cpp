#include "bitboard_lookup.h"
#include "chess_types.h"
#include "move.h"
#include "atomic_capture_masks.h" 
#include "atomic_position.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Test format (each line):
// [position];[fromSq] [toSq] [special] [promoPiece];[finalPosition]
// [position] and [finalPosition] are given in full FEN.
// [fromSq] and [toSq] are given in lowercase algebraic, e.g. "a2" or "d7".
// [special] is either "-", "promo", "castle" or "ep".
// [promoPiece] is "N", "B", "R", "Q" or "-" (only for nonpromotions).
// 
// Example:
// 4k3/8/8/8/8/8/8/4K2R w K - 0 1;e1 h1 castle -;4k3/8/8/8/8/8/8/5RK1 b - - 0 1

class SingleMoveTest {
    public:
    AtomicPosition posTest;
    Move mv;
    
    std::string strFenBefore, strFenAfter;
    AtomicPosition posBefore;
    AtomicPosition posAfter;
    
    SingleMoveTest(std::istringstream& issline) {
        std::string strFromSq, strToSq, strSpecial, strPromoPiece;
        
        std::getline(issline, strFenBefore, ';');
        
        std::getline(issline, strFromSq, ' ');
        std::getline(issline, strToSq, ' ');
        std::getline(issline, strSpecial, ' ');
        std::getline(issline, strPromoPiece, ';');
        
        std::getline(issline, strFenAfter, ';');
        
        // initialise move
        Square fromSq = square(strFromSq);
        Square toSq = square(strToSq);
        PieceType pcty = NO_PCTY; // will error if falls to makePromotion
        
        if (strPromoPiece == "-") { //do nothing
        } else if (strPromoPiece == "N") {
            pcty = KNIGHT;
        } else if (strPromoPiece == "B") {
            pcty = BISHOP;
        } else if (strPromoPiece == "R") {
            pcty = ROOK;
        } else if (strPromoPiece == "Q") {
            pcty = QUEEN;
        }
        
        if (strSpecial == "-") {
            mv = buildMove(fromSq, toSq);
        } else if (strSpecial == "promo") {
            mv = buildPromotion(fromSq, toSq, pcty);
        } else if (strSpecial == "castle") {
            mv = buildCastling(fromSq, toSq);
        } else if (strSpecial == "ep") {
            mv = buildEp(fromSq, toSq);
        }
        
        posBefore.fromFen(strFenBefore);
        posAfter.fromFen(strFenAfter);
    }
    
    bool runMake() {
        bool isPassed = true;
        posTest.fromFen(strFenBefore);
        posTest.makeMove(mv);
        if (posTest != posAfter) {
            isPassed = false;
            std::cout << strFenBefore << "\n";
            std::cout << posTest.pretty();
        }
        return isPassed;
    }
    
    bool runUnmake() {
        bool isPassed = true;
        posTest.fromFen(strFenBefore);
        posTest.makeMove(mv);
        posTest.unmakeMove(mv);
        if (posTest != posBefore) {
            isPassed = false;
            std::cout << strFenBefore << "\n";
            std::cout << posTest.pretty();
        }
        return isPassed;
    }
    
    private:
    // To refactor for future use if needed
    Square square(std::string cn) {
        //assert xn is of form "e5" or "c1" etc.
        return ::square(static_cast<int>(cn[0] - 'a'),
                        static_cast<int>(cn[1] - '1'));
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Run the perft tests with the command [filename] "
                     "[EPD file path] [0 for Make, 1 for Unmake] "
                     "(all arguments required).\n";
        return 0;
    }
    
    // Open EPD file.
    std::string epdFile {argv[1]};
    std::ifstream testSuite;
    testSuite.open(epdFile);
    
    // Setup
    int testOption {std::atoi(argv[2])};
    if (testOption != 0 && testOption != 1) {
        std::cout << "Invalid test type (Make = 0 / Unmake = 1).";
        return 0;
    }
    
    std::string strTest;
    int testId = 0;
    int numTests = 0;
    std::vector<int> idFails;
    
    initialiseBbLookup();
    initialiseAtomicMasks();
    
    
    auto timeStart = std::chrono::steady_clock::now();
    // Run each test in the testSuite (parsed from EPD).
    while (std::getline(testSuite, strTest)) {
        ++numTests;
        ++testId;
        bool isTestCorrect = true;
        std::istringstream iss {strTest};
        SingleMoveTest test {iss};
        if (testOption == 0) {
            isTestCorrect = test.runMake();
        } else if (testOption == 1) {
            isTestCorrect = test.runUnmake();
        }
        if (!isTestCorrect) {
            idFails.push_back(testId);
        }
    }
    auto timeEnd = std::chrono::steady_clock::now();
    auto timeTaken = timeEnd - timeStart;
    testSuite.close();
    
    // Print testing summary
    int numFails = idFails.size();
    float passRate = 100 * static_cast<float>(numTests - numFails) / static_cast<float>(numTests);
    std::cout << "\n======= Summary =======\n";
    std::cout << "Passrate = " << std::to_string(passRate) << "%\n";
    if (idFails.size() > 0) {
        std::cout << "Failed tests:";
        for (int idFail: idFails) {
            std::cout << " " << std::to_string(idFail);
        }
    }
    std::cout << std::chrono::duration <double, std::milli> (timeTaken).count() << " ms\n";
    
    
    AtomicPosition apos;
    apos.fromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq");
    apos.makeMove(buildMove(SQ_A1, SQ_A2));
    apos.makeMove(buildMove(SQ_H8, SQ_H7));
    apos.makeMove(buildCastling(SQ_E1, SQ_H1));
    apos.makeMove(buildMove(SQ_H7, SQ_H6));
    apos.makeMove(buildMove(SQ_A2, SQ_A3));
    apos.makeMove(buildCastling(SQ_E8, SQ_A8));
    std::cout << std::to_string(apos.explosionStack.size()) << "\n";
    
    return 0;
}

// Changes needed for atomic chess:
// > Captures: check exploded pawns/nonpawns are correct.
// > Captures: check capturer/capturee pawns are destroyed.
// > Promocaps: same deal
// > Castlings: check exploded rooks cause castling rights to be lost.