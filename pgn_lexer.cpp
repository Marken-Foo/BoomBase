#ifndef PGN_LEXER_INCLUDED
#define PGN_LEXER_INCLUDED

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "pgn_visitors.h"
#include "streambuf.cpp"
// #include "filebuf.cpp"

// testing
#include <chrono>
#include <fstream>
#include <iostream>

bool readEscape(Istream& input, PgnVisitor& parser);
bool readLineComment(Istream& input, PgnVisitor& parser);


class IsOneOf {
    // functor
    public:
    IsOneOf(std::string str) : targets(str.begin(), str.end()) {}
    IsOneOf(std::vector<char> vec) : targets(vec) {}
    
    bool operator()(char ch) const {
        return std::find(targets.begin(), targets.end(), ch) != targets.end();
    }
    
    private:
    std::vector<char> targets{};
};

const std::string PGN_WHITESPACE {" \n\r\t\v"};
const IsOneOf isIdentifierEnd{PGN_WHITESPACE + "\""};
const IsOneOf isTokenEnd{PGN_WHITESPACE + ".?!"};

void skipWhitespace(Istream& input, PgnVisitor& parser) {
    auto token = input.readWhile(IsOneOf{" \r\t\v"});
    while (token.begin != token.end) {
        if (input.peek() == '\n') {
            input.get();
            parser.acceptNewline();
        }
        token = input.readWhile(IsOneOf{" \r\t\v"});
    }
    return;
}

bool skipToToken(Istream& input, PgnVisitor& parser) {
    // skips to next token, while handling whitespace, '%' escape, and semicolon comments
    skipWhitespace(input, parser);
    char ch {};
    do {
        ch = input.get();
        if (!input) {
            return true;
        }
        if (ch == '%') {
            if (!readEscape(input, parser)) {
                return false;
            }
        } else if (ch == ';') {
            readLineComment(input, parser);
        } else {
            input.unget();
        }
        skipWhitespace(input, parser);
    } while (ch == '%' || ch == ';');
    return true;
}

bool readEscape(Istream& input, PgnVisitor& parser) {
    // entry point: just read a '%' character
    input.unget(); // unconsumes '%'
    input.unget(); // attempts to unconsume again
    if (!input) {
        return false; // unread too much (e.g. '%' is first char in file)
    }
    char ch = input.get();
    if (ch == '\n') {
        input.get(); // reconsumes newline
        input.get(); // reconsumes '%'
        input.readUntil([](char c){return c == '\n';});
        return true;
    } else {
        // unexepected '%' token not preceded by '\n'
        return false;
    }
}

bool readLineComment(Istream& input, PgnVisitor& parser) {
    // entry point: just read a ';' character
    input.readUntil([](char ch){return ch == '\n';});
    return true;
}

bool readTagPair(Istream& input, PgnVisitor& parser) {
    // entry point: just read a '[' character
    // Now skip whitespace while handling escapes and semicolon comments
    skipToToken(input, parser);
    
    // read tag name
    auto token = input.readUntil(isIdentifierEnd);
    std::string tagName {token.begin, token.end};
    skipToToken(input, parser);
    
    // read tag value (enclosed in double quotes)
    if (input.get() != '"') {
        return false; // expected opening double quote
    }
    std::string tagValue {};
    char ch {};
    while (ch != '"' && input) {
        // read until closing double quote, while correctly reading escaped backslashes/double quotes.
        auto continuation = input.readUntil(IsOneOf{"\"\\"});
        tagValue.append(continuation.begin, continuation.end); // is double quote or backslash
        ch = input.get();
        if (ch == '"') {break;}
        else if (ch == '\\') {
            char escaped = input.get();
            if (!input) {break;}
            else if (escaped != '\\' && escaped != '"') {
                tagValue.append("\\");
            }
            tagValue.append(1, escaped);
        }
    }
    // ensure tag pair is closed with ']'
    skipToToken(input, parser);
    if (input.get() == ']') {
        return parser.acceptTagPair(tagName, tagValue);
    } else {
        return false; // something went wrong (can't tell what)
    }
}

// Rely on parser to catch grammar errors, here we just lex and delegate.
bool readToken(Istream& input, PgnVisitor& parser) {
    skipWhitespace(input, parser);
    char ch = input.get();
    if (input.eof()) {
        return true;
    }
    // if next token is obvious from the next character, lex and parse.
    switch (ch) {
    case '\n' :
        return parser.acceptNewline();
    case '[' :
        return readTagPair(input, parser);
    case '(' :
        return parser.acceptRavStart();
    case ')' :
        return parser.acceptRavEnd();
    case '{' : {
        auto comment {input.readUntil([](char c){return c == '}';})};
        if (input.get() == '}') {
            return parser.acceptComment(comment);
        } else {
            return false; // missing closing '}'
        }
    }
    case '*' :
        return parser.acceptResult(RESULT_UNKNOWN);
    case '$' : {
        auto nag = input.readWhile(::isdigit);
        return parser.acceptNag(nag);
    }
    case '%' :
        return readEscape(input, parser);
    case ';' :
        return readLineComment(input, parser);
    case '?' : //intentional fallthrough, '?' and '!' are treated the same
    case '!' :
        return parser.acceptSuffix(input.readWhile([](char c){return c == '?' || c == '!';}));
    default :
        break;
    }
    // if next token is not obvious from the first character alone, we need to grab it and check its type.
    input.unget();
    RawToken token = input.readUntil(isTokenEnd);
    // if token does not begin with a number, assume it is SAN
    if (::isalpha(token.begin[0])) {
        return parser.acceptSan(token);
    }
    
    // if token only contains digits, assume movenumber, eat trailing periods
    if (std::all_of(token.begin, token.end, ::isdigit)) {
        bool res = parser.acceptMoveNumber(token);
        // cleanup
        skipWhitespace(input, parser);
        input.readWhile([](char ch){return ch == '.';});
        return res;
    }
    
    // if token matches pgn game end, parse appropriate token.
    std::string stok(token.begin, token.end);
    if (stok == "1-0") {
        return parser.acceptResult(RESULT_WHITE);
    } else if (stok == "0-1") {
        return parser.acceptResult(RESULT_BLACK);
    } else if (stok == "1/2-1/2") {
        return parser.acceptResult(RESULT_DRAW);
    }
    
    // something mysterious happened
    return parser.acceptUnknown(token);
}


int main() {
    PrinterPgnVisitor parser;
    std::ifstream infile {"tests/pgn.pgn"};
    StreamBuffer fbuf = infile.rdbuf();
    Istream inpgn {&fbuf};
    
    auto timeStart = std::chrono::steady_clock::now();
    while (readToken(inpgn, parser)) {
        if (!inpgn) {break;}
    }
    auto timeEnd = std::chrono::steady_clock::now();
    auto timeTaken = timeEnd - timeStart;
    std::cout << std::chrono::duration<double, std::milli>(timeTaken).count()
              << " ms\n";
    std::cout << "Done! DONE!";
    return 0;
}

#endif //#ifndef PGN_LEXER_INCLUDED