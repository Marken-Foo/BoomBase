#ifndef PARSER_VISITORS_INCLUDED
#define PARSER_VISITORS_INCLUDED

#include <iostream> // for printer parser
#include <string>

enum PgnToken {
    // symbols used
    RESULT_WHITE, RESULT_BLACK, RESULT_DRAW, RESULT_UNKNOWN
};

class PgnVisitor {
    // Default is a "do nothing" visitor. Useful for benchmarking or skipping.
    // TODO: it should also track whether a PGN game is over or not?
    public:
    virtual bool acceptTagPair(std::vector<char> tagName, std::vector<char> tagValue) {
        return true;
    }
    virtual bool acceptComment(std::vector<char> comment) {return true;}
    virtual bool acceptNag(std::vector<char> i) {return true;}
    virtual bool acceptSan(std::vector<char> san) {return true;}
    virtual bool acceptSuffix(std::vector<char> suffix) {return true;}
    virtual bool acceptRavStart() {return true;}
    virtual bool acceptRavEnd() {return true;}
    virtual bool acceptResult(PgnToken tok) {return true;}
    virtual bool acceptMoveNumber(std::vector<char> movenum) {return true;}
    virtual bool acceptUnknown(std::vector<char> token) {return false;}
    virtual bool acceptNewline() {return true;}
};

/* 
class PrinterPgnVisitor : public PgnVisitor {
    // Prints all information received to standard output. Useful for debugging.
    public:
    bool acceptTagPair(std::string tagName, std::string tagValue) override {
        std::cout << "Tag name: <" << tagName << ">, tag value: <" << tagValue << ">\n";
        return true;
    }
    bool acceptComment(std::string comment) override {
        std::cout << "Comment: <" << comment << ">" << "\n";
        return true;
    }
    bool acceptNag(int i) override {
        std::cout << "Nag: \"" << std::to_string(i) << "\"" << "\n";
        return true;
    }
    bool acceptSan(std::string san) override {
        std::cout << san << " ";
        return true;
    }
    bool acceptSuffix(std::string suffix) override {
        std::cout << suffix << " ";
        return true;
    }
    bool acceptRavStart() override {
        // Needs to go into RAV mode, back one step and make a new variation.
        std::cout << "(enter RAV) ";
        return true;
    }
    bool acceptRavEnd() override {
        // Needs to exit RAV mode, back to most recent branch point where current variation is not mainline.
        std::cout << "(exit RAV) ";
        return true;
    }
    bool acceptResult(PgnToken tok) override {
        std::cout << "Result: " << std::to_string(tok) << "\n";
        return true;
    }
    bool acceptMoveNumber(std::string movenum) override {
        std::cout << "Mv#:" << movenum << " ";
        return true;
    }
    bool acceptUnknown(std::string token) override {
        std::cout << "Unknown: " << token << " ";
        return false;
    }
    bool acceptNewline() override {
        std::cout << "Newline\n";
        return true;
    }
};
 */

class GameBuilderPgnVisitor {};

#endif //#ifndef PARSER_VISITORS_INCLUDED