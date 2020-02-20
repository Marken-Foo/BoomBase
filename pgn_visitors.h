#ifndef PARSER_VISITORS_INCLUDED
#define PARSER_VISITORS_INCLUDED

#include <iostream> // for printer parser
#include <string>

enum PgnToken {
    // symbols used
    RESULT_WHITE, RESULT_BLACK, RESULT_DRAW, RESULT_UNKNOWN
};

class ParserVisitor {
    // TODO: turn into base class from which we inherit several parser types
    // default should be DoNothingVisitor (i.e. SkipVisitor)
    // it should also track whether a PGN game is over or not?
    // this is currently printing parser for debugging
    public:
    // need variadic template to set optional args.
    virtual bool accept(PgnToken tok) {
        std::cout << tok;
        return true;
    }
    virtual bool acceptTagPair(std::string tagName, std::string tagValue) {
        std::cout << "Tag name: <" << tagName << ">, tag value: <" << tagValue << ">\n";
        return true;
    }
    virtual bool acceptComment(std::string comment) {
        std::cout << "Comment: \"" << comment << "\"" << "\n";
        return true;
    }
    virtual bool acceptNag(int i) {
        std::cout << "Nag: \"" << std::to_string(i) << "\"" << "\n";
        return true;
    }
    virtual bool acceptSan(std::string san) {
        std::cout << san << " ";
        return true;
    }
    virtual bool acceptSuffix(std::string suffix) {
        std::cout << suffix << " ";
        return true;
    }
    virtual bool acceptRavStart() {
        // Needs to go into RAV mode, back one step and make a new variation.
        return true;
    }
    virtual bool acceptRavEnd() {
        // Needs to exit RAV mode, back to most recent branch point where current variation is not mainline.
        return true;
    }
    virtual bool acceptResult(PgnToken tok) {
        std::cout << "Result: " << std::to_string(tok) << "\n";
        return true;
    }
    virtual bool acceptMoveNumber(std::string movenum) {
        std::cout << "Movenum: " << movenum << " ";
        return true;
    }
    virtual bool acceptUnknown(std::string token) {
        std::cout << "Unknown: " << token << " ";
        return false;
    }
    virtual bool acceptNewline() {
        std::cout << "Newline\n";
        return true;
    }
};

#endif //#ifndef PARSER_VISITORS_INCLUDED