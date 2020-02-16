#ifndef PGN_INCLUDED
#define PGN_INCLUDED

#include <algorithm>
#include <iostream> //to refactor to unit test methods
#include <fstream>
#include <string>

// !!! MOST OF THIS CODE ASSUMES NO STRANGE ZERO-WIDTH CHARACTERS IN PGN !!!
// ALSO LARGELY ASSUMES ASCII (stream content is 8-bit char)
bool skipEscapedLines(std::istream& input);

std::string PGN_WHITESPACE_CHARS {" \t\r"};

enum PgnToken {
    // symbols used
    RESULT_WHITE, RESULT_BLACK, RESULT_DRAW, RESULT_UNKNOWN
};


class ParserVisitor {
    // this is currently printing parser for debugging
    public:
    // need variadic template to set optional args.
    bool accept(PgnToken tok) {
        std::cout << tok;
        return true;
    }
    bool acceptTagPair(std::string tagName, std::string tagValue) {
        std::cout << "Tag name: <" << tagName << ">, tag value: <" << tagValue << ">\n";
        return true;
    }
    bool acceptComment(std::string comment) {
        std::cout << "Comment: \"" << comment << "\"" << "\n";
        return true;
    }
    bool acceptNag(int i) {
        std::cout << "Nag: \"" << std::to_string(i) << "\"" << "\n";
        return true;
    }
    bool acceptSan(std::string san) {
        std::cout << san << " ";
        return true;
    }
    bool acceptSuffix(std::string suffix) {
        std::cout << suffix << " ";
        return true;
    }
    bool acceptRavStart() {
        // Needs to go into RAV mode, back one step and make a new variation.
        return true;
    }
    bool acceptRavEnd() {
        // Needs to exit RAV mode, back to most recent branch point where current variation is not mainline.
        return true;
    }
    bool acceptResult(PgnToken tok) {
        std::cout << "Result: " << std::to_string(tok) << "\n";
        return true;
    }
    bool acceptMoveNumber(std::string movenum) {
        std::cout << "Movenum: " << movenum << " ";
        return true;
    }
    bool acceptUnknown(std::string token) {
        std::cout << "Unknown: " << token << " ";
        return false;
    }
};


// Lexing??
std::string readUntil(std::istream& input, char ch) {
    std::string str {};
    std::getline(input, str, ch);
    return str;
}

void skipWhitespace(std::istream& input) {
    while (input) {
        char ch = input.get();
        if (ch == '\n') {
            // Check for % escape character and skip line if it is
            skipEscapedLines(input);
            continue;
        }
        if (PGN_WHITESPACE_CHARS.find(ch) == std::string::npos) {
            input.unget();
            break;
        }
    }
    return;
}

std::string trimWhitespace(std::string str) {
    const auto strBegin {str.find_first_not_of(PGN_WHITESPACE_CHARS)};
    if (strBegin == std::string::npos) {
        return "";
    }
    const auto strEnd {str.find_last_not_of(PGN_WHITESPACE_CHARS)};
    return str.substr(strBegin, strEnd - strBegin + 1);
}

std::string readString(std::istream& input, ParserVisitor& parser) {
    // Assumes it has been passed an input beginning with the char '"'.
    // UNUSED FOR NOW
    input.get(); // assert = '"';
    // TODO: Handle escaped double quotes "\"" and backslash "\\"
    // TODO: Handle colon separator ':'
    std::string outStr {readUntil(input, '"')};
    if (input.eof()) {
        // ERROR: unexpected EOF (expected closing '"' for string)
        // TODO: error handling here or by caller?
    }
    return outStr;
}

bool skipEscapedLines(std::istream& input) {
    // skips over lines escaped by '%' at column 1.
    // assert previous character was '\n'
    while (input.peek() == '%') {
        readUntil(input, '\n');
    }
    return true;
}

bool readSemicolonComment(std::istream& input, ParserVisitor& parser) {
    // reads (and discards) semicolon comments.
    input.get(); // assert = ';';
    readUntil(input, '\n'); // discards semicolon comments
    return true; // can end on eof.
}

bool readAccoladeComment(std::istream& input, ParserVisitor& parser) {
    input.get(); // assert = '{';
    std::string comment {readUntil(input, '}')};
    if (input.eof()) {
        //syntax error: EOF, expected closing '}' in accolade comment
        return false;
    }
    return parser.acceptComment(comment);
}

/// PGN import format move number indications may have zero or more period characters following the digit sequence that gives the move number; one or more white space characters may appear between the digit sequence and the period(s).
bool readMoveNumber(std::istream& input, ParserVisitor& parser) {
    // reads (and discards) move numbers.
    // eats all digits, eats whitespace, then eats all periods.
    char ch = input.get();
    while (std::string{"0123456789"}.find(ch) != std::string::npos) {
        ch = input.get();
    }
    input.unget();
    skipWhitespace(input);
    ch = input.get();
    while (ch == '.') {
        ch = input.get();
    }
    input.unget();
    return true;
}

bool readNag(std::istream& input, ParserVisitor& parser) {
    // assert last character read = '$';
    int i;
    input >> i; // check failbit after this
    if (input.fail()) {
        //error: could not parse as integer
        return false;
    }
    return parser.acceptNag(i);
}

bool readSan(std::istream& input, ParserVisitor& parser) {
    std::string san {};
    input >> san; // read until whitespace character, sanitisation deferred to parser
    if (input.eof()) {
        // ERROR: unexpected EOF (EOF while reading SAN move)
    }
    return parser.acceptSan(san);
}

bool readRavStart(std::istream& input, ParserVisitor& parser) {
    input.get(); // assert = '('
    return parser.acceptRavStart();
}

bool readRavEnd(std::istream& input, ParserVisitor& parser) {
    input.get(); // assert = ')'
    return parser.acceptRavEnd();
}

bool readSuffix(std::istream& input, ParserVisitor& parser) {
    // Reads move suffix annotations: '!' '?' '!!' '??' '!?' '?!' and longer
    // strings of '!' and '?'
    // May also read invalid suffixes, parser will validate.
    char ch = input.get();
    std::string suffix {};
    while (ch == '!' || ch == '?') {
        suffix.append(1, ch);
        ch = input.get();
    }
    input.unget();
    parser.acceptSuffix(suffix);
    return true;
}

bool readTerminationUnknown(std::istream& input, ParserVisitor& parser) {
    if (input.get() == '*') {
        return parser.acceptResult(RESULT_UNKNOWN);
    }
    return false;
}

bool readTagPair(std::istream& input, ParserVisitor& parser) {
    // TODO: Handle escaped double quotes "\"" and backslash "\\"
    // TODO: Handle colon separator ':'
    if (input.get() != '[') {
        input.unget();
        return false;
    }
    skipWhitespace(input);
    std::string tagName {readUntil(input, '"')};
    if (input.eof()) {
        //syntax error: EOF, expected opening '"' in tag value
        return false;
    }
    tagName = trimWhitespace(tagName);
    
    std::string tagValue {readUntil(input, '"')};
    if (input.eof()) {
        //syntax error: EOF, expected closing '"' in tag value
        return false;
    }
    
    skipWhitespace(input);
    if (input.get() != ']') {
        // Do I test for input.eof() here?
        //syntax error: expected closing ']' in tag pair
        return false;
    }
    skipWhitespace(input);
    return parser.acceptTagPair(tagName, tagValue);
}

bool readTagSection(std::ifstream& input, ParserVisitor& parser) {
    bool res {true};
    while (input.peek() == '[' & res) {
        res = readTagPair(input, parser);
    }
    return res;
}

bool readMovetextToken(std::ifstream& input, ParserVisitor& parser) {
    // can be either movenum, SAN (with !?), NAG, {}, RAV, or termination
    // or also ;, %
    // first identify obvious tokens
    char ch = input.peek();
    switch (ch) {
    case '(' :
        readRavStart(input, parser);
        return true;
    case ')' :
        readRavEnd(input, parser);
        return true;
    case '{' :
        readAccoladeComment(input, parser);
        return true;
    case '*' :
        readTerminationUnknown(input, parser);
        return true;
    case '$' :
        readNag(input, parser);
        return true;
    case ';' :
        readSemicolonComment(input, parser);
        return true;
    case '!' : // intentional fallthrough
    case '?' :
        readSuffix(input, parser);
        return true;
    }
    
    // if not an obvious token, need to eat for SAN/movenum/termination.
    std::string token {};
    while (input) {
        ch = input.get();
        if (ch == '?' || ch == '!') {
            // hit move suffix
            input.unget();
            break;
        }
        if (ch == '\n' || PGN_WHITESPACE_CHARS.find(ch) != std::string::npos) {
            // hit whitespace
            input.unget();
            break;
        }
        if (input.eof()) {
            break;
        }
        token.append(1, ch);
    }
    
    // if token does not begin with a number, assume it is SAN
    if (std::string{"0123456789"}.find(token[0]) == std::string::npos) {
        parser.acceptSan(token);
    }
    
    // if token matches pgn game end, parse appropriate token.
    if (token == "1-0") {
        parser.acceptResult(RESULT_WHITE);
        return true;
    } else if (token == "0-1") {
        parser.acceptResult(RESULT_BLACK);
        return true;
    } else if (token == "1/2-1/2") {
        parser.acceptResult(RESULT_DRAW);
        return true;
    }
    
    // if token only contains digits, assume it is a movenumber
    if (token.find_first_not_of("0123456789.") == std::string::npos) {
        parser.acceptMoveNumber(token);
        readMoveNumber(input, parser);
        return true;
    }
    // something mysterious happened
    return parser.acceptUnknown(token);
}


int main() {
    ParserVisitor parser;
    std::ifstream input {"tests/pgn_tests.txt"};
    readTagSection(input, parser);
    
    std::cout << "Now looking at pgn\n";
    std::ifstream inpgn {"tests/pgn.pgn"};
    readTagSection(inpgn, parser);
    while (inpgn) {
        skipWhitespace(inpgn);
        readMovetextToken(inpgn, parser);
    }
    return 0;
}


#endif //#ifndef PGN_INCLUDED