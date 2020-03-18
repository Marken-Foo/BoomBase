#ifndef PARSER_VISITORS_INCLUDED
#define PARSER_VISITORS_INCLUDED

#include <iostream> // for printer parser
#include <string>
#include "streambuf.cpp"

enum PgnToken {
    // symbols used
    RESULT_WHITE, RESULT_BLACK, RESULT_DRAW, RESULT_UNKNOWN
};

class PgnVisitor {
    // Default is a "do nothing" visitor. Useful for benchmarking or skipping.
    // TODO: it should also track whether a PGN game is over or not?
    public:
    virtual bool acceptTagPair(std::string tagName, std::string tagValue) {
        return true;
    }
    virtual bool acceptComment(RawToken comment) {return true;}
    virtual bool acceptNag(RawToken nag) {return true;}
    virtual bool acceptSan(RawToken san) {return true;}
    virtual bool acceptSuffix(RawToken suffix) {return true;}
    virtual bool acceptRavStart() {return true;}
    virtual bool acceptRavEnd() {return true;}
    virtual bool acceptResult(PgnToken tok) {return true;}
    virtual bool acceptMoveNumber(RawToken movenum) {return true;}
    virtual bool acceptUnknown(RawToken token) {return false;}
    virtual bool acceptNewline() {return true;}
};


class PrinterPgnVisitor : public PgnVisitor {
    // Prints all information received to standard output. Useful for debugging.
    public:
    bool acceptTagPair(std::string tagName, std::string tagValue) override {
        std::cout << "Tag name: <" << tagName << ">, tag value: <" << tagValue << ">\n";
        return true;
    }
    bool acceptComment(RawToken comment) override {
        std::cout << "Comment: <";
        auto it = comment.begin;
        while (it != comment.end) {
            std::cout << *it;
            ++it;
        }
        std::cout << ">" << "\n";
        return true;
    }
    bool acceptNag(RawToken nag) override {
        std::cout << "Nag: \"";
        auto it = nag.begin;
        while (it != nag.end) {
            std::cout << *it;
            ++it;
        }
        std::cout << "\"" << "\n";
        return true;
    }
    bool acceptSan(RawToken san) override {
        char* it = san.begin;
        while (it != san.end) {
            std::cout << *it;
            ++it;
        }
        std::cout << " ";
        // std::cout << std::string(san.begin, san.end) << " ";
        return true;
    }
    bool acceptSuffix(RawToken suffix) override {
        char* it = suffix.begin;
        while (it != suffix.end) {
            std::cout << *it;
            ++it;
        }
        std::cout << " ";
        // std::cout << std::string(suffix.begin, suffix.end) << " ";
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
    bool acceptMoveNumber(RawToken movenum) override {
        std::cout << "Mv#:";
        auto it = movenum.begin;
        while (it != movenum.end) {
            std::cout << *it;
            ++it;
        }
        std::cout << " ";
        return true;
    }
    bool acceptUnknown(RawToken token) override {
        std::cout << "Unknown: " << std::string(token.begin, token.end) << " ";
        return false;
    }
    bool acceptNewline() override {
        std::cout << "Newline\n";
        return true;
    }
};


class GameBuilderPgnVisitor {};

#endif //#ifndef PARSER_VISITORS_INCLUDED