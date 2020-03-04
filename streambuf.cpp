#ifndef STREAMBUF_INCLUDED
#define STREAMBUF_INCLUDED

#include <algorithm>
#include <cstring>
#include <ios>
#include <istream>
#include <streambuf>

using RawToken = std::pair<char*, char*>;

class StreamBuffer : public std::streambuf {
    // Extension of std::streambuf to include readWhile/readUntil methods
    private:
    int bufferSize {16384}; // buffer size
    std::streambuf* source {nullptr};
    char* buffer {nullptr};
    
    // TODO: modify readUntil() and readWhile() to return pointers to the internal char* buffer. Upon reaching egptr, underflow() once, retaining egptr() - it characters (so write a private underflow(int i) method to do so) and find again. If std::find returns egptr() again without eof, then report buffer overflow (check if missing closing paren/accolade/brace etc).
    // Or instead of reporting buffer overflow, instead just continue, but silently return wrong pointers.
    // Note also that returned pointers are valid but incorrect if the buffer position (w.r.t. source) changes.
    
    // Basically, assumes that readUntil() and readWhile() will only be used to read things (tokens) that are shorter than bufferSize. (Nobody puts 32kB of raw text into a single PGN comment, right?) So returned pointers are valid only if "actual" token length < bufferSize, and valid only while the buffer position does not change.
    
    public:
    template <typename Condition>
    RawToken readUntil(Condition condition) {
        // Reads all nonmatching chars and leaves first matching in buffer.
        // Returns pointers to [start, end) of read characters.
        // Will only read characters up to bufferSize, returns a pair of
        // nullptr if attempting to read more than bufferSize chars.
        auto it = std::find_if(gptr(), egptr(), condition);
        if (it == egptr()) {
            if (underflow(egptr() - gptr()) == std::char_traits<char>::eof()) {
                setg(eback(), egptr(), egptr());
                return RawToken(eback(), egptr());
            }
            it = std::find_if(eback(), egptr(), condition);
            if (it == egptr()) {
                setg(eback(), eback(), egptr());
                return RawToken(nullptr, nullptr);
            }
        } else {
            RawToken res(gptr(), it);
            setg(eback(), it, egptr());
            return res;
        }
    }
    
    template <typename Condition>
    RawToken readWhile(Condition condition) {
        // Reads all matching chars and leaves first nonmatching in buffer.
        // Returns pointers to [start, end) of read characters.
        // Will only read characters up to bufferSize, returns a pair of
        // nullptr if attempting to read more than bufferSize chars.
        auto it = std::find_if_not(gptr(), egptr(), condition);
        if (it == egptr()) {
            if (underflow(egptr() - gptr()) == std::char_traits<char>::eof()) {
                setg(eback(), egptr(), egptr());
                return RawToken(eback(), egptr());
            }
            it = std::find_if_not(eback(), egptr(), condition);
            if (it == egptr()) {
                setg(eback(), eback(), egptr());
                return RawToken(nullptr, nullptr);
            }
        } else {
            RawToken res(gptr(), it);
            setg(eback(), it, egptr());
            return res;
        }
    }
    
    StreamBuffer(std::streambuf* sbuf) 
        : source(sbuf), buffer(new char[bufferSize])
    { }
    
    ~StreamBuffer() override {
        delete[] buffer;
    }
    
    private:
    int_type underflow(std::streamsize n) {
        // Keeps the last n characters (pushing them to the front of the
        // buffer) and reads bufferSize - n more characters into the buffer.
        // Assumes n <= bufferSize.
        for (int i = 0; i < n; ++i) {
            buffer[i] = buffer[bufferSize - n + i];
        }
        // Attempts to read from source -- note that if eof is found, the
        // buffer has already been altered (see above!).
        std::streamsize read = source->sgetn(buffer + n, bufferSize - n);
        setg(buffer, buffer + n, buffer + n + read);
        if (read == 0) {
            return std::char_traits<char>::eof();
        }
        return *gptr();
    }
    
    protected:    
    int_type underflow() override {
        // Attempts to move buffer window forward by bufferSize - 1 chars.
        return underflow(1);
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
    RawToken readUntil(Condition condition) {
        return fbuf->readUntil(vecbuf, condition);
    }
    
    template <typename Condition>
    RawToken readWhile(Condition condition) {
        return fbuf->readWhile(vecbuf, condition);
    }
};

#endif //#ifndef STREAMBUF_INCLUDED