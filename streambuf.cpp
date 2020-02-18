#ifndef STREAMBUF_INCLUDED
#define STREAMBUF_INCLUDED

#include <algorithm>
#include <cstring>
#include <ios>
#include <streambuf>

#include <iostream>

class FileBuffer : public std::streambuf {
    private:
    int bufferSize {16384}; // buffer size
    std::streambuf* source {nullptr};
    char* buffer {nullptr};
    
    public:
    // gotoinFile(pos_type pos) {}
    
    template <typename Condition>
    char* readUntil(char* cstr, Condition condition) {
        // leaves matching char as next char in buffer
        auto it = std::find_if(gptr(), egptr(), condition);
        strncat(cstr, gptr(), it - gptr());
        while (it == egptr()) {
            // fully read buffer, underflow and check eof
            if (underflow() == std::char_traits<char>::eof()) {
                return cstr;
            }
            it = std::find_if(gptr(), egptr(), condition);
            strncat(cstr, gptr(), it - gptr());
        }
        setg(eback(), it, egptr());
        return cstr;
    }
    
    template <typename Condition>
    char* readWhile(char* cstr, Condition condition) {
        // reads all matching chars and leaves first nonmatching in buffer
        auto it = std::find_if_not(gptr(), egptr(), condition);
        strncat(cstr, gptr(), it - gptr());
        while (it == egptr()) {
            // fully read buffer, underflow and check eof
            if (underflow() == std::char_traits<char>::eof()) {
                return cstr;
            }
            it = std::find_if_not(gptr(), egptr(), condition);
            strncat(cstr, gptr(), it - gptr());
        }
        setg(eback(), it, egptr());
        return cstr;
    }
    
    FileBuffer(std::streambuf* sbuf) 
        : source(sbuf), buffer(new char[bufferSize])
    { }
    
    ~FileBuffer() override {
        delete[] buffer;
    }
    
    protected:
    int_type underflow() override {
        std::streamsize read = source->sgetn(buffer, bufferSize);
        if (read == 0) {
            return std::char_traits<char>::eof();
        }
        setg(buffer, buffer, buffer + read);
        return *gptr();
    }
    
    int_type pbackfail(int_type ch = std::char_traits<char>::eof()) override {
        // if called with 1 arg, attempt to put back different char in get area. Should place/overwrite said char into character sequence if allowed. Underlying file not altered.
        if (ch != std::char_traits<char>::eof()) {
            setg(eback(), gptr() - 1, egptr());
            *gptr() = ch;
            return ch;
        } else {
            // if called with no args, there is no putback position in get area. Should back up one char if allowed.
            // (??)
        }
        return std::char_traits<char>::eof();
    }
    
    std::streamsize xsgetn(char* s, std::streamsize count) override {
        std::streamsize available {egptr() - gptr()};
        std::streamsize read {0};
        while (count > available) {
            strncat(s, gptr(), available);
            count -= available;
            read += available;
            // fully read buffer, underflow and check eof.
            if (underflow() == std::char_traits<char>::eof()) {
                // handles eof
                return read;
            }
            available = egptr() - gptr();
        }
        // count <= distance
        strncat(s, gptr(), count);
        read += count;
        setg(eback(), gptr() + count, egptr());
        return read;
    }
};
#include <iostream>
class iFstream {
    // wrapper for a std::ifstream
    public:
    
};

bool isg(char ch) {
    // returns true if character is a lowercase g.
    return (ch == 'd');
}

#include <fstream>
#include "pgn.cpp"
int main() {
    std::ifstream file {"tests/pgn_tests.txt"};
    FileBuffer fbuf {file.rdbuf()};
    std::istream infbuf {&fbuf};
    
    ParserVisitor parser;
    readTagSection(infbuf, parser);
    
    std::ifstream file2 {"tests/pgn.pgn"};
    FileBuffer fbuf2 {file2.rdbuf()};
    std::istream inpgn {&fbuf2};
    
    std::cout << "HELLO\n";
    
    char out[4500]{};
    // fbuf2.sgetc();
    fbuf2.readUntil(out, isg);
    std::cout << std::string{out};
    std::cout << "\nWARGLEWARGLEWARGLE\n";
    char wow[4500]{};
    fbuf2.readUntil(out, [](char ch){return (ch == 's');});
    std::cout << std::string{out};
    // readTagSection(inpgn, parser);
    // while (inpgn) {
        // skipWhitespace(inpgn);
        // readMovetextToken(inpgn, parser);
    // }
    return 0;
}

#endif //#ifndef STREAMBUF_INCLUDED