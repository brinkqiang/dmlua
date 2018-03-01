
#ifndef __SAFEFORMATEX_H_INCLUDE__
#define __SAFEFORMATEX_H_INCLUDE__

#include <cstdio>
#include <climits>
#include <string>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <cassert>
#include <locale>
#include <iostream>
#include <sstream>
// long is 32 bit on 64-bit Windows!
// intptr_t used to get 64 bit on Win64
#if defined(_WIN32) || defined(_WIN64)
#  define SAFEFORMAT_SIGNED_LONG_LONG signed long long
#  define SAFEFORMAT_UNSIGNED_LONG_LONG unsigned long long
#else
#  define SAFEFORMAT_SIGNED_LONG_LONG signed long long
#  define SAFEFORMAT_UNSIGNED_LONG_LONG unsigned long long
#endif

// Windows headers could have min/max defined
#ifndef SAFEFORMAT_MAX
#define SAFEFORMAT_MAX(a,b)    (((a) > (b)) ? (a) : (b))
#endif

#ifndef SAFEFORMAT_MIN
#define SAFEFORMAT_MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

namespace
{
    // Crude writing method: writes straight to the file, unbuffered
    // Must be combined with a buffer to work properly (and efficiently)

    void write(std::FILE* f, const char* from, const char* to);

    // Write to an ostream

    void write(std::ostream& f, const char* from, const char* to);

    // Write to a string

    void write(std::string& s, const char* from, const char* to);

    // Write to a fixed-size buffer
    template <class Char>
        void write(std::pair<Char*, std::size_t>& s, const Char* from, const Char* to) {
            assert(from <= to);
            if(from + s.second < to)
              throw std::overflow_error("");
            // s.first: position one past the final copied element
            s.first = std::copy(from, to, s.first);
            // remaining buffer size
            s.second -= to - from;
        }

    ////////////////////////////////////////////////////////////////////////////////
    // PrintfState class template
    // Holds the formatting state, and implements operator() to format stuff
    // Todo: make sure errors are handled properly
    ////////////////////////////////////////////////////////////////////////////////

    template <class Device, class Char>
        struct PrintfState {
            PrintfState(Device dev, const Char * format)
                : device_(dev)
                  , format_(format)
                  , width_(0)
                  , prec_(0)
                  , flags_(0)
                  , result_(0) {
                      Advance();
                  }

            ~PrintfState() {
                assert(result_ != -1);
            }

#define PRINTF_STATE_FORWARD(type) \
            PrintfState& operator()(type par) {\
                return (*this)(static_cast< SAFEFORMAT_UNSIGNED_LONG_LONG >(par)); \
            }

            PRINTF_STATE_FORWARD(bool)
                PRINTF_STATE_FORWARD(char)
                PRINTF_STATE_FORWARD(signed char)
                PRINTF_STATE_FORWARD(unsigned char)
                PRINTF_STATE_FORWARD(signed short)
                PRINTF_STATE_FORWARD(unsigned short)
                PRINTF_STATE_FORWARD(signed int)
                PRINTF_STATE_FORWARD(unsigned int)
                PRINTF_STATE_FORWARD(signed long)
                PRINTF_STATE_FORWARD(unsigned long)
                PRINTF_STATE_FORWARD(signed long long)
                // Print (or gobble in case of the "*" specifier) an int
                PrintfState& operator()(SAFEFORMAT_UNSIGNED_LONG_LONG i) {
                    if (result_ == -1) return *this; // don't even bother
                    // % [flags] [width] [.prec] [modifier] type_char
                    // Fetch the flags
                    ReadFlags();
                    if (*format_ == '\0') {
                        result_ = -1;
                        return *this;
                    }

                    if (*format_ == '*') {
                        // read the width and get out
                        SetWidth(static_cast<size_t>(i));
                        ++format_;
                        return *this;
                    }
                    ReadWidth();
                    // precision
                    if (*format_ == '.') {
                        // deal with precision
                        if (format_[1] == '*') {
                            // read the precision and get out
                            SetPrec(static_cast<size_t>(i));
                            format_ += 2;
                            return *this;
                        }
                        ReadPrecision();
                    }
                    ReadModifiers();
                    // input size modifier
                    if (ForceShort()) {
                        // short int
                        const Char c = *format_;
                        if (c == 'x' || c == 'X' || c == 'u' || c == 'o') {
                            i = static_cast<SAFEFORMAT_UNSIGNED_LONG_LONG>(static_cast<unsigned short>(i));
                        }
                    }
                    FormatWithCurrentFlags(i);
                    return *this;
                }

            PrintfState& operator()(void* n) {
                if (result_ == -1) return *this; // don't even bother
                PrintUsing_snprintf(n,"p");
                return *this;
            }

            PrintfState& operator()(double n) {
                if (result_ == -1) return *this; // don't even bother
                PrintUsing_snprintf(n,"eEfgG");
                return *this;
            }

            PrintfState& operator()(long double n) {
                if (result_ == -1) return *this; // don't even bother
                PrintUsing_snprintf(n,"eEfgG");
                return *this;
            }

            // Store the number of characters printed so far
            PrintfState& operator()(int * pi) {
                return StoreCountHelper(pi);
            }

            // Store the number of characters printed so far
            PrintfState& operator()(short * pi) {
                return StoreCountHelper(pi);
            }

            // Store the number of characters printed so far
            PrintfState& operator()(long * pi) {
                return StoreCountHelper(pi);
            }

            PrintfState& operator()(const std::string& stdstr) {
                return operator()(stdstr.c_str());
            }

            PrintfState& operator()(const char *const s) {
                if (result_ == -1) return *this;
                ReadLeaders();
                const char fmt = *format_;
                if (fmt == 'p') {
                    FormatWithCurrentFlags(reinterpret_cast<SAFEFORMAT_UNSIGNED_LONG_LONG>(s));
                    return *this;
                }
                if (fmt != 's') {
                    result_ = -1;
                    return *this;
                }
                const size_t len = SAFEFORMAT_MIN(std::strlen(s), prec_);
                if (width_ > len) {
                    if (LeftJustify()) {
                        Write(s, s + len);
                        Fill(' ', width_ - len);
                    } else {
                        Fill(' ', width_ - len);
                        Write(s, s + len);
                    }
                } else {
                    Write(s, s + len);
                }
                Next();
                return *this;
            }

            PrintfState& operator()(const void *const p) {
                return (*this)(reinterpret_cast<SAFEFORMAT_UNSIGNED_LONG_LONG>(p));
            }

            // read the result
            operator int() const {
                return static_cast<int>(result_);
            }

        private:
            PrintfState& operator=(const PrintfState&);
            template <typename T>
                PrintfState& StoreCountHelper(T *const pi) {
                    if (result_ == -1) return *this; // don't even bother
                    ReadLeaders();
                    const char fmt = *format_;
                    if (fmt == 'p') { // pointer
                        FormatWithCurrentFlags(reinterpret_cast<SAFEFORMAT_UNSIGNED_LONG_LONG>(pi));
                        return *this;
                    }
                    if (fmt != 'n') {
                        result_ = -1;
                        return *this;
                    }
                    assert(pi != 0);
                    *pi = result_;
                    Next();
                    return *this;
                }

            void FormatWithCurrentFlags(const SAFEFORMAT_UNSIGNED_LONG_LONG i) {
                // look at the format character
                Char formatChar = *format_;
                bool isSigned = formatChar == 'd' || formatChar == 'i';
                if (formatChar == 'p') {
                    formatChar = 'x'; // pointers go to hex
                    SetAlternateForm(); // printed with '0x' in front
                    isSigned = true; // that's what gcc does
                }
                if (!strchr("cdiuoxX", formatChar)) {
                    result_ = -1;
                    return;
                }
                Char buf[
                    sizeof(SAFEFORMAT_UNSIGNED_LONG_LONG) * 3 // digits
                    + 1 // sign or ' '
                    + 2 // 0x or 0X
                    + 1]; // terminating zero
                const Char *const bufEnd = buf + (sizeof(buf) / sizeof(Char));
                Char * bufLast = buf + (sizeof(buf) / sizeof(Char) - 1);
                Char signChar = 0;
                unsigned int base = 10;

                if (formatChar == 'c') {
                    // Format only one character
                    // The 'fill with zeros' flag is ignored
                    ResetFillZeros();
                    *bufLast = static_cast<char>(i);
                } else {
                    // TODO: inefficient code, refactor
                    const bool negative = isSigned && static_cast<SAFEFORMAT_SIGNED_LONG_LONG>(i) < 0;
                    if (formatChar == 'o') base = 8;
                    else if (formatChar == 'x' || formatChar == 'X') base = 16;
                    bufLast = isSigned
                        ? RenderWithoutSign(static_cast<SAFEFORMAT_SIGNED_LONG_LONG>(i), bufLast, base,
                                    formatChar == 'X')
                        : RenderWithoutSign(i, bufLast, base,
                                    formatChar == 'X');
                    // Add the sign
                    if (isSigned) {
                        negative ? signChar = '-'
                            : ShowSignAlways() ? signChar = '+'
                            : Blank() ? signChar = ' '
                            : 0;
                    }
                }
                // precision
                size_t
                    countDigits = bufEnd - bufLast,
                                countZeros = prec_ != size_t(-1) && countDigits < prec_ &&
                                    formatChar != 'c'
                                    ? prec_ - countDigits
                                    : 0,
                                countBase = base != 10 && AlternateForm() && i != 0
                                    ? (base == 16 ? 2 : countZeros > 0 ? 0 : 1)
                                    : 0,
                                countSign = (signChar != 0),
                                totalPrintable = countDigits + countZeros + countBase + countSign;
                size_t countPadLeft = 0, countPadRight = 0;
                if (width_ > totalPrintable) {
                    if (LeftJustify()) {
                        countPadRight = width_ - totalPrintable;
                        countPadLeft = 0;
                    } else {
                        countPadLeft = width_ - totalPrintable;
                        countPadRight = 0;
                    }
                }
                if (FillZeros() && prec_ == size_t(-1)) {
                    // pad with zeros and no precision - transfer padding to precision
                    countZeros = countPadLeft;
                    countPadLeft = 0;
                }
                // ok, all computed, ready to print to device
                Fill(' ', countPadLeft);
                if (signChar != 0) Write(&signChar, &signChar + 1);
                if (countBase > 0) Fill('0', 1);
                if (countBase == 2) Fill(formatChar, 1);
                Fill('0', countZeros);
                Write(bufLast, bufEnd);
                Fill(' ', countPadRight);
                // done, advance
                Next();
            }

            void Write(const Char* b, const Char* e) {
                if (result_ < 0) return;
                const SAFEFORMAT_SIGNED_LONG_LONG x = e - b;
                write(device_, b, e);
                result_ += x;
            }

            template <class Value>
                void PrintUsing_snprintf(Value n, const char* check_fmt_char) {
                    const Char *const fmt = format_ - 1;

                    if (*fmt != '%') {
                        result_ = -1;
                        return;
                    }

                    // enforce format string validity
                    ReadLeaders();
                    // enforce format spec
                    if (!strchr(check_fmt_char, *format_)) {
                        result_ = -1;
                        return;
                    }
                    // format char validated, copy it to a temp and use legacy sprintf
                    ++format_;
                    Char fmtBuf[128], resultBuf[1024];
                    if (format_  >= fmt + sizeof(fmtBuf) / sizeof(Char)) {
                        result_ = -1;
                        return;
                    }
                    memcpy(fmtBuf, fmt, (format_ - fmt) * sizeof(Char));
                    fmtBuf[format_ - fmt] = 0;

                    const int stored =
#ifdef _MSC_VER
#if _MSC_VER < 1400
                        _snprintf
#else
                        _snprintf_s
#endif
#else
                        snprintf
#endif
                        (resultBuf, sizeof(resultBuf) / sizeof(Char), fmtBuf, n);

                    if (stored < 0) {
                        result_ = -1;
                        return;
                    }
                    Write(resultBuf, resultBuf + strlen(resultBuf));
                    Advance(); // output stuff to the next format directive
                }

            void Fill(const Char c, size_t n) {
                for (; n > 0; --n) {
                    Write(&c, &c + 1);
                }
            }

            Char* RenderWithoutSign(SAFEFORMAT_UNSIGNED_LONG_LONG n, char* bufLast,
                        unsigned int base, bool uppercase) {
                const Char hex1st = uppercase ? 'A' : 'a';
                for (;;) {
                    const SAFEFORMAT_UNSIGNED_LONG_LONG next = n / base;
                    Char c = static_cast<Char>(n - next * base);
                    c = static_cast<Char>(c + (c <= 9 ? '0' : static_cast<Char>(hex1st - 10)));
                    *bufLast = c;
                    n = next;
                    if (n == 0) break;
                    --bufLast;
                }
                return bufLast;
            }

            char* RenderWithoutSign(SAFEFORMAT_SIGNED_LONG_LONG n, char* bufLast, unsigned int base,
                        bool uppercase) {
                if (n != LONG_MIN) {
                    return RenderWithoutSign(static_cast<SAFEFORMAT_UNSIGNED_LONG_LONG>(n < 0 ? -n : n),
                                bufLast, base, uppercase);
                }
                // annoying corner case
                char* save = bufLast;
                ++n;
                bufLast = RenderWithoutSign(static_cast<SAFEFORMAT_UNSIGNED_LONG_LONG>(n),
                            bufLast, base, uppercase);
                --(*save);
                return bufLast;
            }

            void Next() {
                ++format_;
                Advance();
            }

            void Advance() {
                ResetAll();
                const Char* begin = format_;
                for (;;) {
                    if (*format_ == '%') {
                        if (format_[1] != '%') { // It's a format specifier
                            Write(begin, format_);
                            ++format_;
                            break;
                        }
                        // It's a "%%"
                        Write(begin, ++format_);
                        begin = ++format_;
                        continue;
                    }
                    if (*format_ == 0) {
                        Write(begin, format_);
                        break;
                    }
                    ++format_;
                }
            }

            void ReadFlags() {
                for (;; ++format_) {
                    switch (*format_) {
                        case '-': SetLeftJustify(); break;
                        case '+': SetShowSignAlways(); break;
                        case ' ': SetBlank(); break;
                        case '#': SetAlternateForm(); break;
                        case '0': SetFillZeros(); break;
                        default: return;
                    }
                }
            }

            void ParseDecimalSizeT(size_t& dest) {
                if (!std::isdigit(*format_, std::locale())) return;
                size_t r = 0;
                do {
                    // TODO: inefficient - rewrite
                    r *= 10;
                    r += *format_ - '0';
                    ++format_;
                } while (std::isdigit(*format_, std::locale()));
                dest = r;
            }

            void ReadWidth() {
                ParseDecimalSizeT(width_);
            }

            void ReadPrecision() {
                assert(*format_ == '.');
                ++format_;
                ParseDecimalSizeT(prec_);
            }

            void ReadModifiers() {
                switch (*format_) {
                    case 'h': SetForceShort(); ++format_; break;
                    case 'l': ++format_; break;
                              // more (C99 and platform-specific modifiers) to come
                }
            }

            void ReadLeaders() {
                ReadFlags();
                ReadWidth();
                if (*format_ == '.') ReadPrecision();
                ReadModifiers();
            }

            enum {
                leftJustify = 1,
                showSignAlways = 2,
                blank = 4,
                alternateForm = 8,
                fillZeros = 16,
                forceShort = 32
            };

            bool LeftJustify() const { return (flags_ & leftJustify) != 0; }
            bool ShowSignAlways() const { return (flags_ & showSignAlways) != 0; }
            void SetWidth(size_t w) { width_  = w; }
            void SetLeftJustify() { flags_  |= leftJustify; }
            void SetShowSignAlways() { flags_ |= showSignAlways; }
            bool Blank() const { return (flags_ & blank) != 0; }
            bool AlternateForm() const { return (flags_ & alternateForm) != 0; }
            bool FillZeros() const { return (flags_ & fillZeros) != 0; }
            bool ForceShort() const { return (flags_ & forceShort) != 0; }

            void SetPrec(size_t p) { prec_ = p; }
            void SetBlank() { flags_ |= blank; }
            void SetAlternateForm() { flags_ |=  alternateForm; }
            void SetFillZeros() { flags_ |= fillZeros; }
            void ResetFillZeros() { flags_ &= ~fillZeros; }
            void SetForceShort() { flags_ |= forceShort; }

            void ResetAll() {
                assert(result_ != EOF);
                width_ = 0;
                prec_ = size_t(-1);
                flags_ = 0;
            }

            // state
            Device device_;
            const Char* format_;
            size_t width_;
            size_t prec_;
            unsigned int flags_;
            SAFEFORMAT_SIGNED_LONG_LONG result_;
        };


    PrintfState<std::FILE*, char> Printf(const char* format);


    PrintfState<std::FILE*, char> Printf(const std::string& format);


    PrintfState<std::FILE*, char> FPrintf(std::FILE* f, const char* format);


    PrintfState<std::FILE*, char> FPrintf(std::FILE* f, const std::string& format);


    PrintfState<std::ostream&, char> FPrintf(std::ostream& f, const char* format);


    PrintfState<std::ostream&, char> FPrintf(std::ostream& f, const std::string& format);


    PrintfState<std::string&, char> SPrintf(std::string& s, const char* format);


    PrintfState<std::string&, char> SPrintf(std::string& s, const std::string& format);

    template <class T, class Char>
        PrintfState<T&, Char> XPrintf(T& device, const Char* format) {
            return PrintfState<T&, Char>(device, format);
        }

    template <class T>
        PrintfState<T&, char> XPrintf(T& device, const std::string& format) {
            return PrintfState<T&, char>(device, format.c_str());
        }

    template <class Char, std::size_t N>
        PrintfState<std::pair<Char*, std::size_t>, Char>
        BufPrintf(Char (&buf)[N], const Char* format) {
            std::pair<Char*, std::size_t> temp(buf, N);
            return PrintfState<std::pair<Char*, std::size_t>, Char>(temp, format);
        }

    void write(std::FILE* f, const char* from, const char* to) {
        assert(from <= to);
        ::std::fwrite(from, 1, to - from, f);
    }

    // Write to a string

    void write(std::string& s, const char* from, const char* to) {
        assert(from <= to);
        s.append(from, to);
    }

    // Write to a stream

    void write(std::ostream& f, const char* from, const char* to) {
        assert(from <= to);
        f.write(from, std::streamsize(to - from));
    }

    ////////////////////////////////////////////////////////////////////////////////
    // PrintfState class template
    // Holds the formatting state, and implements operator() to format stuff
    // Todo: make sure errors are handled properly
    ////////////////////////////////////////////////////////////////////////////////


    PrintfState<std::FILE*, char> Printf(const char* format) {
        return PrintfState<std::FILE*, char>(stdout, format);
    }

    PrintfState<std::FILE*, char> Printf(const std::string& format) {
        return PrintfState<std::FILE*, char>(stdout, format.c_str());
    }

    PrintfState<std::FILE*, char> FPrintf(std::FILE* f, const char* format) {
        return PrintfState<std::FILE*, char>(f, format);
    }

    PrintfState<std::FILE*, char> FPrintf(std::FILE* f, const std::string& format) {
        return PrintfState<std::FILE*, char>(f, format.c_str());
    }

    PrintfState<std::ostream&, char> FPrintf(std::ostream& f, const char* format) {
        return PrintfState<std::ostream&, char>(f, format);
    }

    PrintfState<std::ostream&, char> FPrintf(std::ostream& f, const std::string& format) {
        return PrintfState<std::ostream&, char>(f, format.c_str());
    }

    PrintfState<std::string&, char> SPrintf(std::string& s, const char* format) {
        return PrintfState<std::string&, char>(s, format);
    }

    PrintfState<std::string&, char> SPrintf(std::string& s, const std::string& format) {
        return PrintfState<std::string&, char>(s, format.c_str());
    }

    //////////////////////////////////////////////////////////////////////////
    template<class T>
        T& SafeFormat(T& device, const char* format){ XPrintf(device,format); return device;}

    template<class T,class V0>
        T& SafeFormat(T& device, const char* format, const V0& p0){ XPrintf(device,format)(p0); return device;}

    template<class T,class V0, class V1>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1){ XPrintf(device,format)(p0)(p1); return device;}

    template<class T,class V0, class V1, class V2>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2){ XPrintf(device,format)(p0)(p1)(p2); return device;}

    template<class T,class V0, class V1, class V2, class V3>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3){ XPrintf(device,format)(p0)(p1)(p2)(p3); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23, class V24>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23, const V24& p24){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23)(p24); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23, class V24, class V25>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23, const V24& p24, const V25& p25){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23)(p24)(p25); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23, class V24, class V25, class V26>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23, const V24& p24, const V25& p25, const V26& p26){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23)(p24)(p25)(p26); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23, class V24, class V25, class V26, class V27>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23, const V24& p24, const V25& p25, const V26& p26, const V27& p27){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23)(p24)(p25)(p26)(p27); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23, class V24, class V25, class V26, class V27, class V28>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23, const V24& p24, const V25& p25, const V26& p26, const V27& p27, const V28& p28){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23)(p24)(p25)(p26)(p27)(p28); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23, class V24, class V25, class V26, class V27, class V28, class V29>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23, const V24& p24, const V25& p25, const V26& p26, const V27& p27, const V28& p28, const V29& p29){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23)(p24)(p25)(p26)(p27)(p28)(p29); return device;}

    template<class T,class V0, class V1, class V2, class V3, class V4, class V5, class V6, class V7, class V8, class V9, class V10, class V11, class V12, class V13, class V14, class V15, class V16, class V17, class V18, class V19, class V20, class V21, class V22, class V23, class V24, class V25, class V26, class V27, class V28, class V29, class V30>
        T& SafeFormat(T& device, const char* format, const V0& p0, const V1& p1, const V2& p2, const V3& p3, const V4& p4, const V5& p5, const V6& p6, const V7& p7, const V8& p8, const V9& p9, const V10& p10, const V11& p11, const V12& p12, const V13& p13, const V14& p14, const V15& p15, const V16& p16, const V17& p17, const V18& p18, const V19& p19, const V20& p20, const V21& p21, const V22& p22, const V23& p23, const V24& p24, const V25& p25, const V26& p26, const V27& p27, const V28& p28, const V29& p29, const V30& p30){ XPrintf(device,format)(p0)(p1)(p2)(p3)(p4)(p5)(p6)(p7)(p8)(p9)(p10)(p11)(p12)(p13)(p14)(p15)(p16)(p17)(p18)(p19)(p20)(p21)(p22)(p23)(p24)(p25)(p26)(p27)(p28)(p29)(p30); return device;}

    //////////////////////////////////////////////////////////////////////////
    inline char* __SafeStrCopy(char *des, size_t des_len, const char *src)
    {
        if(NULL == src)
        {
            des[0] = '\0';
            return des;
        }
        size_t len = strnlen(src, des_len);
        if (len >= des_len)
        {
            //log
        }

        des[des_len-1] = 0;
        return len < des_len ? strcpy(des, src) : strncpy(des, src, des_len-1);
    }
	
	template <size_t N>
	inline char* SafeStrCopy(unsigned char (&des)[N], const char* src)
	{
		return __SafeStrCopy((char*)des, N, src);
	}

    template <size_t N>
        inline char* SafeStrCopy(char (&des)[N], const char* src)
        {
            return __SafeStrCopy(des, N, src);
        }
    template <size_t N>
        inline char* SafeStrCopy(char (&des)[N], const std::string& src)
        {
            return __SafeStrCopy(des, N, src.c_str());
        }

    template <typename T>
        inline void Zero(T& t)
        {
            memset(&t, 0, sizeof(t));
        }
    template <typename T>
        inline void Zero(T* t)
        {
            memset(t, 0, sizeof(*t));
        }


    template <size_t N>
        inline void Zero(char (&des)[N])
        {
            des[0] = '\0';
            des[sizeof(des)-1] = '\0';
        }
    template <size_t N>
        inline void ZeroString(char (&des)[N])
        {
            des[0] = '\0';
            des[sizeof(des)-1] = '\0';
        }
    template <size_t N>
        inline void SafeSprintf(char (&des)[N], const char *format, ...)
        {
            va_list args;
            va_start(args,format);
            int len = vsnprintf(des, sizeof(des)-1, format, args);
            des[sizeof(des)-1] = '\0';
            if (len < 0)
            {
                //log
            }
            va_end(args);
        }

    template<typename R ,typename T>
        static inline R convert(const T &format)
        {
            std::stringstream ss;
            R ret = R();
            ss << format;
            ss >> ret;
            return ret;
        }

    template<typename T, size_t N>
        inline size_t array_size(const T (&src)[N]){ return N; }
}// namespace


#endif // __SAFEFORMATEX_H_INCLUDE__
