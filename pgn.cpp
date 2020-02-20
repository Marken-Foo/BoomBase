#ifndef PGN_INCLUDED
#define PGN_INCLUDED

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream> //to refactor to unit test methods
#include <iterator>
#include <string>

#include "pgn_visitors.h"

bool skipEscapedLines(std::istream& input);

const std::string PGN_WHITESPACE_CHARS {" \t\r\n"};

// Lexing??
std::string readUntil(std::istream& input, char ch) {
    std::string str {};
    std::getline(input, str, ch);
    if (ch == '\n') {
        skipEscapedLines(input);
    }
    return str;
}

std::string readUntil(std::istream& input, std::string delims) {
    std::string str {};
    char ch = input.get();
    if (!input) {
        return str;
    }
    while (delims.find(ch) == std::string::npos) {
        str.append(1, ch);
        ch = input.get();
    }
    input.unget();
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
        if (!std::isspace(ch)) {
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
    while (std::isdigit(ch)) {
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

bool readTagSection(std::istream& input, ParserVisitor& parser) {
    bool res {true};
    while (input.peek() == '[' & res) {
        res = readTagPair(input, parser);
    }
    return res;
}

bool readMovetextToken(std::istream& input, ParserVisitor& parser) {
    // can be either movenum, SAN (with !?), NAG, {}, RAV, or termination
    // or also ;, %
    // first identify obvious tokens
    char ch = input.peek();
    if (input.eof()) {
        return false;
    }
    switch (ch) {
    case '[' :
        // previous game ended, new game begins
        return false;
    case '(' :
        return readRavStart(input, parser);
    case ')' :
        return readRavEnd(input, parser);
    case '{' :
        return readAccoladeComment(input, parser);
    case '*' :
        return readTerminationUnknown(input, parser);
    case '$' :
        return readNag(input, parser);
    case ';' :
        return readSemicolonComment(input, parser);
    case '!' : // intentional fallthrough
    case '?' :
        return readSuffix(input, parser);
    default:
        break;
    }
    
    // if not an obvious token, need to eat for SAN/movenum/termination.
    std::string token {};
    while (input) {
        ch = input.get();
        if (ch == '?' || ch == '!' || ch == '.') {
            // hit move suffix or period
            input.unget();
            break;
        }
        if (std::isspace(ch)) {
            input.unget();
            break;
        }
        if (input.eof()) {
            break;
        }
        token.append(1, ch);
    }
    
    // if token does not begin with a number, assume it is SAN
    if (::isalpha(token[0])) {
        return parser.acceptSan(token);
    }
    
    // if token matches pgn game end, parse appropriate token.
    if (token == "1-0") {
        return parser.acceptResult(RESULT_WHITE);
    } else if (token == "0-1") {
        return parser.acceptResult(RESULT_BLACK);
    } else if (token == "1/2-1/2") {
        return parser.acceptResult(RESULT_DRAW);
    }
    
    // if token only contains digits, assume movenumber, eat trailing periods
    if (std::all_of(token.begin(), token.end(), ::isdigit)) {
        readMoveNumber(input, parser); // cleanup
        bool res = parser.acceptMoveNumber(token);
        return res;
    }
    // something mysterious happened
    return parser.acceptUnknown(token);
}
/* 

int main() {
    ParserVisitor parser;
    std::ifstream file {"tests/pgn_tests.txt"};
    
    readTagSection(file, parser);
    
    std::cout << "Now looking at pgn\n";
    std::ifstream inpgn {"tests/pgn.pgn"};
    readTagSection(inpgn, parser);
    while (inpgn) {
        skipWhitespace(inpgn);
        readMovetextToken(inpgn, parser);
    }
    return 0;
}

 */
#endif //#ifndef PGN_INCLUDED