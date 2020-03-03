#ifndef STREAMBUF_INCLUDED
#define STREAMBUF_INCLUDED

#include <algorithm>
#include <cstring>
#include <ios>
#include <istream>
#include <streambuf>
#include <vector>

using VecBuf = std::vector<char>;
using RawToken = std::pair<VecBuf::iterator, VecBuf::iterator>;

class StreamBuffer : public std::streambuf {
    // Extension of std::streambuf to include readWhile/readUntil methods
    private:
    int bufferSize {16384}; // buffer size
    std::streambuf* source {nullptr};
    char* buffer {nullptr};
    
    public:
    template <typename Condition>
    VecBuf& readUntil(VecBuf& vec, Condition condition) {
        // leaves matching char as next char in buffer
        auto it = std::find_if(gptr(), egptr(), condition);
        vec.insert(vec.end(), gptr(), it);
        while (it == egptr()) {
            // buffer has been fully read; underflow and check eof
            if (underflow() == std::char_traits<char>::eof()) {
                setg(eback(), it, egptr());
                return vec;
            }
            it = std::find_if(gptr(), egptr(), condition);
            vec.insert(vec.end(), gptr(), it);
        }
        setg(eback(), it, egptr());
        return vec;
    }
    
    template <typename Condition>
    VecBuf& readWhile(VecBuf& vec, Condition condition) {
        // reads all matching chars and leaves first nonmatching in buffer
        auto it = std::find_if_not(gptr(), egptr(), condition);
        vec.insert(vec.end(), gptr(), it);
        while (it == egptr()) {
            // buffer has been fully read; underflow and check eof
            if (underflow() == std::char_traits<char>::eof()) {
                setg(eback(), it, egptr());
                return vec;
            }
            it = std::find_if_not(gptr(), egptr(), condition);
            vec.insert(vec.end(), gptr(), it);
        }
        setg(eback(), it, egptr());
        return vec;
    }
    
    StreamBuffer(std::streambuf* sbuf) 
        : source(sbuf), buffer(new char[bufferSize])
    { }
    
    ~StreamBuffer() override {
        delete[] buffer;
    }
    
    protected:    
    int_type underflow() override {
        // attempts to move buffer window forward by bufferSize - 1 chars.
        buffer[0] = buffer[bufferSize - 1];
        std::streamsize read = source->sgetn(buffer+1, bufferSize-1);
        if (read == 0) {
            return std::char_traits<char>::eof();
        }
        setg(buffer, buffer + 1, buffer + 1 + read);
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
            if (seekoff(-1, std::ios_base::cur, std::ios_base::in) == pos_type(off_type(-1))) {
                // failed to back up (start of file)
                return std::char_traits<char>::eof();
            } else {
                *gptr() = ch;
                return ch;
            }
        }
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
    
    pos_type seekoff(off_type off, std::ios_base::seekdir dir,
                     std::ios_base::openmode which =
                     std::ios_base::in | std::ios_base::out) override {
        auto res {source->pubseekoff(off, dir, which)};
        underflow();
        return res;
    }
    
    pos_type seekpos(pos_type pos, std::ios_base::openmode which =
                     std::ios_base::in | std::ios_base::out) override {
        auto res {source->pubseekpos(pos, which)};
        underflow();
        return res;
    }
    
    int sync() override {
        underflow();
        return 0;
    }
};

class Istream : public std::istream {
    // Custom istream to access readWhile/readUntil methods of StreamBuffer
    public:
    StreamBuffer* fbuf;
    
    public:
    Istream(StreamBuffer* sbuf)
        : std::istream(sbuf)
        , fbuf(sbuf)
    { }
    
    template <typename Condition>
    std::string readUntil(Condition condition) {
        VecBuf vecbuf {};
        fbuf->readUntil(vecbuf, condition);
        RawToken rt = std::make_pair(vecbuf.begin(), vecbuf.end());
        return std::string(rt.first, rt.second);
    }
    
    template <typename Condition>
    std::string readWhile(Condition condition) {
        VecBuf vecbuf {};
        fbuf->readWhile(vecbuf, condition);
        RawToken rt = std::make_pair(vecbuf.begin(), vecbuf.end());
        return std::string(rt.first, rt.second);
    }
};

#endif //#ifndef STREAMBUF_INCLUDED