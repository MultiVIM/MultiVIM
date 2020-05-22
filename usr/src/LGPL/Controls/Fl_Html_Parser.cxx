
#include "Fl_Html_Parser.H"
#include <FL/fl_utf8.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  TABLE TEMPLATE
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
0x0D, 0x0E, 0x0F,  // 0x00 - 0x0F 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,  // 0x10 - 0x1F 0x20,
0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D,
0x2E, 0x2F,  // 0x20 - 0x2F 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,  // 0x30 - 0x3F 0x40, 0x41,
0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E,
0x4F,  // 0x40 - 0x4F 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,  // 0x50 - 0x5F 0x60, 0x61, 0x62,
0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, //
0x60 - 0x6F 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
0x7B, 0x7C, 0x7D, 0x7E, 0x7F,  // 0x70 - 0x7F 0x80, 0x81, 0x82, 0x83, 0x84,
0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,  // 0x80 -
0x8F 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B,
0x9C, 0x9D, 0x9E, 0x9F,  // 0x90 - 0x9F 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5,
0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,  // 0xA0 - 0xAF
  0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC,
0xBD, 0xBE, 0xBF,  // 0xB0 - 0xBF 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6,
0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,  // 0xC0 - 0xCF 0xD0,
0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD,
0xDE, 0xDF,  // 0xD0 - 0xDF 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,  // 0xE0 - 0xEF 0xF0, 0xF1,
0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE,
0xFF   // 0xF0 - 0xFF
}
*/

static unsigned char to_lower_table_[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0x00 - 0x0F
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 0x10 - 0x1F
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 0x20 - 0x2F
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 0x30 - 0x3F

    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 0x40 - 0x4F
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 0x50 - 0x5F
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, // 0x60 - 0x6F
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // 0x70 - 0x7F

    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, // 0x80 - 0x8F
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, // 0x90 - 0x9F
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, // 0xA0 - 0xAF
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, // 0xB0 - 0xBF

    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, // 0xC0 - 0xCF
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, // 0xD0 - 0xDF
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, // 0xE0 - 0xEF
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF // 0xF0 - 0xFF
};

inline char to_lower (char c)
{
    return (char)(to_lower_table_[(unsigned)c]);
}
inline unsigned char to_lower (unsigned char c)
{
    return to_lower_table_[c];
}

static bool is_space_[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, // 0x00 - 0x0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x20 - 0x2F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x30 - 0x3F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x40 - 0x4F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x50 - 0x5F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x60 - 0x6F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x70 - 0x7F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80 - 0x8F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x90 - 0x9F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xA0 - 0xAF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xB0 - 0xBF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xC0 - 0xCF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xD0 - 0xDF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xE0 - 0xEF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xF0 - 0xFF
};

static inline bool is_space (unsigned a)
{
    return is_space_[a];
}
static inline bool is_space (int a)
{
    return is_space_[(unsigned)a];
}
static inline bool is_space (unsigned char a)
{
    return is_space_[a];
}
static inline bool is_space (char a)
{
    return is_space_[(unsigned char)a];
}

static unsigned char no_letter_table[256] = {
    //  0     1     2     3     4     5     6     7     8     9     A     B C D
    //  E     F
    0xFF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0x00 - 0x0F
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 0x10 - 0x1F
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 0x20 - 0x2F
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 0x30 - 0x3F
    0x40, 0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, // 0x40 - 0x4F
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 0x50 - 0x5F
    0x60, 0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, // 0x60 - 0x6F
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // 0x70 - 0x7F
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, // 0x80 - 0x8F
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, // 0x90 - 0x9F
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, // 0xA0 - 0xAF
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, // 0xB0 - 0xBF
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, // 0xC0 - 0xCF
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, // 0xD0 - 0xDF
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, // 0xE0 - 0xEF
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF // 0xF0 - 0xFF
};

static char no_letter (char a)
{
    return no_letter_table[(unsigned)a];
}

int Fl_Html_Tokenizer::spaces (const char * str, int len)
{
    int a = len;
    while (a)
    {
        if (!is_space (*str))
            return len - a;
        str++;
        a--;
    }
    return a - len;
}

int Fl_Html_Tokenizer::first_word_equal (const char * s, const char * word,
                                         int str_len)
{
    int len = str_len;
    if (!len)
        return 0;
    char c = *s;
    while (is_space (c))
    {
        len--;
        s++;
        if (!len)
            return 0;
        c = *s;
    }
    char wc = *word;
    while (len)
    {
        c = *s;
        s++;
        if (c != wc)
            return 0;
        len--;
        word++;
        wc = *word;
        if (!wc)
        {
            if (!len)
                return str_len;
            if (no_letter (*s))
                return str_len - len;
            return 0;
        }
    }
    return 0;
}

int Fl_Html_Tokenizer::case_first_word_equal (const char * s, const char * word,
                                              int str_len)
{
    int len = str_len;
    if (!len)
        return 0;
    char c = *s;
    while (is_space (c))
    {
        len--;
        s++;
        if (!len)
            return 0;
        c = *s;
    }
    char wc = *word;
    while (len)
    {
        c = *s;
        s++;
        if (to_lower (c) != wc)
            return 0;
        len--;
        word++;
        wc = *word;
        if (!wc)
        {
            if (!len)
                return str_len;
            if (no_letter (*s))
                return str_len - len;
            return 0;
        }
    }
    return 0;
}

static bool is_name_finish_[256] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, // 0x00 - 0x0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x10 - 0x1F
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x20 - 0x2F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, // 0x30 - 0x3F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x40 - 0x4F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x50 - 0x5F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x60 - 0x6F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x70 - 0x7F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x80 - 0x8F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x90 - 0x9F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xA0 - 0xAF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xB0 - 0xBF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xC0 - 0xCF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xD0 - 0xDF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xE0 - 0xEF
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xF0 - 0xFF
};

static inline bool is_name_finish (unsigned a)
{
    return is_name_finish_[a];
}
static inline bool is_name_finish (int a)
{
    return is_name_finish_[(unsigned)a];
}
static inline bool is_name_finish (unsigned char a)
{
    return is_name_finish_[a];
}
static inline bool is_name_finish (char a)
{
    return is_name_finish_[(unsigned char)a];
}

static unsigned long hex_values_[256] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x00 - 0x0F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x10 - 0x1F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0x80,
    0,
    0x81,
    0,
    0, // 0x20 - 0x2F
    0x70,
    0x71,
    0x72,
    0x73,
    0x74,
    0x75,
    0x76,
    0x77,
    0x68,
    0x69,
    0,
    0,
    0,
    0,
    0,
    0, // 0x30 - 0x3F , note that zero is
       // encoded as 0x80
    0,
    0x4A,
    0x4B,
    0x4C,
    0x4D,
    0x4E,
    0x4F,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x40 - 0x4F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x50 - 0x5F
    0,
    0x4A,
    0x4B,
    0x4C,
    0x4D,
    0x4E,
    0x4F,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x60 - 0x6F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x70 - 0x7F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x80 - 0x8F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0x90 - 0x9F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0xA0 - 0xAF
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0xB0 - 0xBF
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0xC0 - 0xCF
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0xD0 - 0xDF
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0xE0 - 0xEF
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0, // 0xF0 - 0xFF
};

static inline unsigned intvalue (const char * s, long long & value,
                                 unsigned mask, unsigned mult, int length)
{
    unsigned v;
    long long val = 0;
    int len = length;
    while (len && ((v = hex_values_[(unsigned)(*s)]) & mask))
    {
        val *= mult;
        val += v & 0xF;
        len--;
        s++;
    }
    value = val;
    return length - len;
}

static inline unsigned signedintvalue (const char * s, long long & value,
                                       long long & sign, int len)
{
    if (!len)
        return 0;
    char c = *s;
    if (0x80 & hex_values_[(unsigned)c])
    { // it is '+' or '-'
        if (c & 4)
            sign = -1;
        else
            sign = 1;
        len--;
        s++;
    }
    return intvalue (s, value, 0x20, 10, len);
}

unsigned Fl_Html_Tokenizer::signed_int_value (const char * src,
                                              long long & value,
                                              long long & sign, int len)
{
    const char * s = src;
    char c = *s;
    while (is_space (c))
    {
        len--;
        s++;
        if (!len)
            return 0;
        c = *s;
    }
    sign = 0;
    int ret = src - s;
    int r = signedintvalue (s, value, sign, len);
    if (r)
        return r + ret;
    return 0;
}

unsigned Fl_Html_Tokenizer::int_value (const char * s, long long & value,
                                       int format, int length)
{
    if (!length)
        return 0;
    char c = *s;
    unsigned mult;
    unsigned mask;
    int len = length;
    while (is_space (c))
    {
        len--;
        s++;
        if (!len)
            return 0;
        c = *s;
    }

    if ((format & FORMAT_0X) && c == '0' && (len - 1) && 'x' == to_lower (s[1]))
    {
        mult = 16;
        mask = 0x40;
        len -= 2;
        s += 2;
    }
    else if (((format & FORMAT_X) && 'x' == to_lower (c)) ||
             ((format & FORMAT_HASH) && '#' == c))
    {
        mult = 16;
        mask = 0x40;
        len--;
        s++;
        if (!len)
            return 0; // error
    }
    else if ((format & FORMAT_0) && '0' == c)
    {
        mult = 8;
        mask = 0x10;
    }
    else if (0xFF & format & FORMAT_8)
    {
        mult = 8;
        mask = 0x10;
    }
    else if (0xFF & format & FORMAT_16)
    {
        mult = 16;
        mask = 0x40;
    }
    else
    { // default is decadic
        mult = 10;
        mask = 0x20;
    }

    long long val;
    if (mult != 10)
    { // camnnot have sign
        int len2 = intvalue (s, val, mask, mult, len);
        if (len2)
        {
            value = val;
            return length - len + len2;
        }
        return 0;
    }

    long long sign = 0;
    int len2 = signedintvalue (s, val, sign, len);
    if (sign)
        len--;
    else
        sign = 1;
    if (len2)
    {
        value = val * sign;
        return length - len + len2;
    }
    return 0;
}

unsigned Fl_Html_Tokenizer::float_value (const char * src, double & value,
                                         int length)
{
    if (!length)
        return 0;
    int src_len = length;
    const char * o_src = src;
    while (is_space (*src))
    {
        src_len--;
        src++;
        if (!src_len)
            return 0;
    }
    long long sign = 0;
    long long val = 0;
    unsigned len2 = signedintvalue (src, val, sign, src_len);
    src_len -= len2;
    src += len2;
    if (sign)
    {
        src_len--;
        src++;
    }
    else
    {
        sign = 1;
    }
    if (!src_len || !(src_len - 1))
    {
        if (len2)
        {
            value = sign * val;
            return src - o_src;
        }
        return 0;
    }
    double vv;
    if (*src == '.')
    {
        src_len--;
        src++;
        unsigned v;
        long long val2 = 0;
        long long divider = 1;
        while (src_len && ((v = hex_values_[(unsigned)(*src)]) & 0x20))
        {
            val2 *= 10;
            val2 += v & 0xF;
            divider *= 10;
            src_len--;
            src++;
        }
        if (!len2 && divider == 1) // no digits before or after decimal point
            return 0;
        vv = double(val) + double(val2) / double(divider);
    }
    else
    {
        if (!len2)
            return 0; // no digits
        vv = double(val);
    }

    long long e_value;
    long long e_sign = 0;
    if (src_len && 'e' == to_lower (*src))
    {
        src++;
        src_len--;
        unsigned exp_len = signedintvalue (src, e_value, e_sign, src_len);
        if (e_sign)
            src_len--;
        else
            e_sign = 1;
        if (exp_len)
        {
            value = sign * (vv * pow (10, (double)(e_sign * e_value)));
            return length - src_len + exp_len;
        }
        value = sign * vv;
        return length - src_len - 1;
    }
    value = sign * vv;
    return length - src_len;
}

////////////////////////////  Fl_Html_Pair_Table /////////////////////

int Fl_Html_Pair_Table::find_substring_ (const char * src, const void *& value,
                                         int len) const
{
    for (int i = n - 1; i >= 0; i--)
    {
        int v = Fl_Html_Tokenizer::substring (src, table[i].name, len);
        if (v)
        {
            value = table[i].value;
            return v;
        }
    }
    return 0;
}

int Fl_Html_Pair_Table::find_case_substring_ (const char * src,
                                              const void *& value,
                                              int len) const
{
    for (int i = n - 1; i >= 0; i--)
    {
        int v = Fl_Html_Tokenizer::case_substring (src, table[i].name, len);
        if (v)
        {
            value = table[i].value;
            return v;
        }
    }
    return 0;
}

int Fl_Html_Pair_Table::find_ (const char * src, const void *& value,
                               int len) const
{
    for (int i = n - 1; i >= 0; i--)
    {
        int v = Fl_Html_Tokenizer::substring (src, table[i].name, len);
        if (len == v || (len < 0 && !src[len]))
        {
            value = table[i].value;
            return v;
        }
    }
    return 0;
}

int Fl_Html_Pair_Table::find_case_ (const char * src, const void *& value,
                                    int len) const
{
    for (int i = n - 1; i >= 0; i--)
    {
        int v = Fl_Html_Tokenizer::case_substring (src, table[i].name, len);
        if (len == v || (len < 0 && !src[len]))
        {
            value = table[i].value;
            return v;
        }
    }
    return 0;
}

const void * Fl_Html_Pair_Table::replace_ (const char * name,
                                           const void * value)
{
    for (int i = n - 1; i >= 0; i--)
    {
        if (Fl_Html_Tokenizer::str_equal (table[i].name, name))
        {
            const void * val = table[i].value;
            table[i].value = value;
            return val;
        }
    }
    add_ (name, value);
    return 0;
}

Fl_Html_Pair_Table::Fl_Html_Pair_Table (int size) : n (0), table_size_ (size)
{
    table = (Entry *)malloc (size * sizeof (Entry));
    static_table = 0;
}

Fl_Html_Pair_Table::Fl_Html_Pair_Table (Entry * const tb, int filled,
                                        int table_size)
    : n (filled), table_size_ (table_size)
{
    static_table = 1;
    table = tb;
}

Fl_Html_Pair_Table::~Fl_Html_Pair_Table ()
{
    if (!static_table)
        free (table);
}

void Fl_Html_Pair_Table::add_ (const char * name, const void * value)
{
    if (n >= table_size_)
    {
        table_size_ = n + 64;
        if (static_table)
        {
            static_table = 0;
            Entry * nt = (Entry *)malloc (table_size_ * sizeof (Entry));
            memcpy (nt, table, n * sizeof (Entry));
            table = nt;
        }
        else
            table = (Entry *)realloc (table, table_size_ * sizeof (Entry));
    }
    table[n].name = name;
    table[n].value = value;
    n++;
}

int Fl_Html_Tokenizer::Special_Character_Table::translate (
    const char * orig_src, unsigned src_len, const char *& value,
    char *& buffer, unsigned & buffer_size)
{
    const char * src = orig_src;
    value = src;
    if (!src_len)
        return 0;
    unsigned len = src_len;
    char * b = 0;
    while (len)
    {
        char c = *src++;
        len--;
        while (c == '&')
        {
            if (!len)
                break;
            if (!b)
            { // copying starting sequence to the buffer
                if (buffer_size < src_len)
                {
                    delete[] buffer;
                    buffer_size = src_len + 64;
                    buffer = new char[buffer_size];
                }
                value = buffer;
                unsigned copied = src_len - len - 1;
                memmove (buffer, orig_src, copied);
                b = buffer + copied;
            }
            c = *src;
            if (c == '#')
            { // unicode number
                src++;
                len--;
                long long iv = 0;
                unsigned conv = int_value (src, iv, FORMAT_X, len);
                if (conv)
                {
                    b += fl_utf8encode ((unsigned)iv, b);
                    src += conv;
                    len -= conv;
                    if (!len)
                        break;
                    c = *src++;
                    len--;
                    if (c == ';')
                    { // even if it is not terminated by ; we keep translated
                      // character anyway
                        src++;
                        len--;
                    }
                }
            }
            else
            {
                const char * value;
                unsigned converted = find (src, value, len);
                if (converted)
                {
                    src += converted;
                    len -= converted;
                    char c = *value;
                    while (c)
                    { // copying utf-8 to the buffer
                        *b++ = c;
                        c = *++value;
                    }
                }
            }
            while (len)
            { // copying next sequence until next special character
                c = *src++;
                len--;
                if (c == '&')
                    break; // go to beggining of the special character loop
                *b++ = c;
            }
        } // end of special character loop
    }
    if (b)
        return b - value;
    return src_len;
}

int Fl_Html_Tokenizer::Special_Character_Table::translate_copy (
    const char * src, unsigned src_len, char *& value)
{
    if (!src)
    {
        value = 0;
        return 0;
    }
    unsigned v_len = src_len + 2;
    value = new char[v_len];
    const char * val;
    int ret = translate (src, src_len, val, value, v_len);
    if (val != value) // original string returned, must perform copy
        memcpy (value, val, ret);
    value[ret] = 0; // assure null-termination
    value[ret + 1] =
        0; // another one if there would be added a space at the end of the word
    return ret;
}

int Fl_Html_Tokenizer::substring (const char * src, const char * orig_to,
                                  int src_len)
{
    const char * to = orig_to;
    char c = *to;
    while (c)
    {
        if (!src_len)
            return 0;
        if (c != *src++)
            return 0;
        src_len--;
        c = *++to;
    }
    return to - orig_to;
}

bool Fl_Html_Tokenizer::str_equal (const char * src, const char * to)
{
    char c = *to++;
    do
    {
        if (c != *src++)
            return 0;
        c = *to++;
    } while (c);
    if (*src)
        return 0;
    return 1;
}

int Fl_Html_Tokenizer::case_substring (const char * src, const char * orig_to,
                                       int src_len)
{
    const char * to = orig_to;
    char c = *to;
    while (c)
    {
        if (!src_len)
            return 0;
        if (c != to_lower (*src++))
            return 0;
        src_len--;
        c = *++to;
    }
    return to - orig_to;
}

int Fl_Html_Tokenizer::name_substring (const char * src, const char * orig_to,
                                       int src_len)
{
    const char * to = orig_to;
    char c = *to;
    while (c)
    {
        if (!src_len)
            return 0;
        if (c != *src++)
            return 0;
        src_len--;
        c = *++to;
    }
    if (!src_len || is_name_finish (*src))
        return to - orig_to;
    return 0;
}

int Fl_Html_Tokenizer::case_name_substring (const char * src,
                                            const char * orig_to, int src_len)
{
    const char * to = orig_to;
    char c = *to;
    while (c)
    {
        if (!src_len)
            return 0;
        if (c != to_lower (*src++))
            return 0;
        src_len--;
        c = *++to;
    }
    if (!src_len || is_name_finish (*src))
        return to - orig_to;
    return 0;
}

static bool compares (const char * src, const char * to)
{
    char c = *to++;
    while (c)
    {
        if (c != *src++)
            return 0;
        c = *to++;
    }
    return 1;
}

char Fl_Html_Tokenizer::skip_spaces ()
{
    char c = *pos;
    while (is_space (c))
        c = *++pos;
    return c;
}

int Fl_Html_Tokenizer::get_word (const char *& src, int & src_len)
{
    const char * c = src;
    int a = src_len;
    while (a && is_space (*c))
    {
        c++;
        a--;
    }
    src_len = a;
    src = c;
    while (a && !is_space (*c))
    {
        c++;
        a--;
    }
    return src_len - a;
}

// This is used to read quoted attribute value after "="
// Note that the value (and value length) INCLUDES surrounded quores
// Atgument quote is either double or single quote.
// Returns ATTRIBUTE_VALUE or error.
Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::read_quoted_value (char quote)
{
    val = pos;
    char c = *++pos;
    while (c && c != quote)
        c = *++pos;
    if (!c)
        return EOF_IN_TAG;
    ++pos;
    val_len = pos - val;
    in_tag = 1;
    return ATTRIBUTE_VALUE;
}

// This is used to read tag name, attribute name or unquotted attribute value
// Returns either argument ret or error.
Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::read_unquoted_value (Result ret)
{
    val = pos;
    char c = *pos;
    while (!is_name_finish (c))
        c = *++pos;
    if (!c)
        return EOF_IN_TAG;
    if (c == '>' && pos[-1] == '/')
        pos--;
    val_len = pos - val;
    return ret;
}

// This is called only after opening '<' or when in_tag is not zero
// to read value of next token.
// Returns either TAG_NAME, ATTRIBUTE_NAME, ATTRIBUTE_VALUE or error.
Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::read_value ()
{
    char c = skip_spaces ();
    switch (c)
    {
    // case 0: return EOF_IN_TAG;
    case '>':
        ++pos;
        if (!in_tag)
            return BAD_TAG_NAME;
        in_tag = 0;
        return END_TAG;
    case '/':
        if (pos[1] == '>')
        {
            pos += 2;
            if (!in_tag)
                return BAD_TAG_NAME;
            in_tag = 0;
            return END_SELF_CLOSED_TAG;
        }
        break;
    case '=':
        if (in_tag < 2)
            return BAD_ATTRIBUTE_NAME;
        ++pos;
        in_tag = 1;
        c = skip_spaces ();
        if (c == '"' || c == '\'')
            return read_quoted_value (c);
        return read_unquoted_value (ATTRIBUTE_VALUE);
    }

    if (in_tag)
        in_tag = ATTRIBUTE_NAME;
    else
        in_tag = TAG_NAME;
    return read_unquoted_value ((Result)in_tag);
}

// pos is at first character behind "<!", c is the character from that position.
Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::read_unknown_comment (char c)
{
    val = pos;
    while (c)
    {
        if (c == '>')
        {
            val_len = pos - val;
            pos++;
            return UNKNOWN_COMMENT;
        }
        c = *++pos;
    };
    return EOF_IN_COMMENT;
}

// use only wnhen string at pos-1 is "<?"
Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::read_question_comment ()
{
    char c = *++pos;
    val = pos;
    while (c && (c != '?' || pos[1] != '>'))
        c = *++pos;
    val_len = pos - val;
    if (!c)
        return EOF_IN_COMMENT;
    pos += 2;
    return QUESTION_COMMENT;
}

// Use only wnhen string at pos-1 is "<!", pos is at "!"
Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::read_comment ()
{
    char c = *++pos;
    if (!c)
        return EOF_IN_COMMENT;
    if (c != '-' || pos[1] != '-')
    {
        if (compares (pos, "[CDATA["))
        {
            pos += 7;
            val = pos;
            c = *pos;
            while (c && (c != ']' || pos[1] != ']' || pos[2] != '>'))
                c = *++pos;
            if (!c)
                return EOF_IN_COMMENT;
            val_len = pos - val;
            pos += 3;
            return CDATA;
        }
        return read_unknown_comment (c);
    }
    pos += 2;
    val = pos;
    c = *pos;
    while (c && (c != '-' || pos[1] != '-' || pos[2] != '>'))
        c = *++pos;
    val_len = pos - val;
    if (!c)
        return EOF_IN_COMMENT;
    pos += 3;
    return COMMENT;
}

// reads the body between tags
Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::read_body ()
{
    val = pos;
    char c = *++pos;
    while (c && c != '<')
        c = *++pos;
    val_len = pos - val;
    return BODY;
}

Fl_Html_Tokenizer::Result Fl_Html_Tokenizer::operator() ()
{
    char c = *pos;
    if (!c)
    {
        if (in_tag)
            return EOF_IN_TAG;
        return FINISH; // ok, end of file
    }
    if (!in_tag)
    {
        if (c != '<')
            return read_body ();
        // from now we are in tag, in_tag is set or reset by read_value
        c = *++pos;
        switch (c)
        {
        case '!':
            return read_comment ();
        case '?':
            return read_question_comment ();
        case '>':
            ++pos;
            return BAD_TAG_NAME;
        }
    }
    return read_value ();
}

static const char * const css_separators = ",:;{}";

Fl_Css_Tokenizer::Result Fl_Css_Tokenizer::operator() ()
{
    char c;
    while (len_ && is_space (c = *str_))
    {
        str_++;
        len_--;
    }
    if (!len_ || !c)
        return FINISH;
    int comma = 0;
    bool post = true;
    switch (c)
    {
    case ',':
        comma = COMMA;
        break;
    case ':':
        if (result_ != PROPERTY)
            return INVALID_SEPARATOR;
        result_ = VALUE;
        break;
    case ';':
        if (result_ != VALUE)
            return INVALID_SEPARATOR;
        result_ = PROPERTY;
        break;
    case '{':
        if (result_ != SELECTOR)
            return INVALID_SEPARATOR;
        result_ = PROPERTY;
        break;
    case '}':
        if (result_ == SELECTOR)
            return INVALID_SEPARATOR;
        result_ = SELECTOR;
        break;
    default:
        post = false;
    }
    if (post)
    {
        len_--;
        str_++;
        while (len_ && is_space (c = *str_))
        {
            str_++;
            len_--;
        }
    }
    if (!len_ || !c)
        return FINISH;
    value_ = str_;

    const char * end = str_;
    while (len_ && c && !(is_space (c) || strchr (css_separators, c)))
    {
        c = *++end;
        len_--;
    }
    str_ = end;
    value_length_ = end - value_;

    return (Result) (result_ | comma);
}

void Fl_Css_Parser::parse_inline (const char * str, int str_len,
                                  void (*callback) (Property & p, void *),
                                  Property & p, void * data)
{
    reset (str, str_len, PROPERTY);
    p.clear ();
    int p_res = (*this) ();
    while (PROPERTY == p_res)
    {
        p.clear (value (), value_length ());
        p_res = (*this) ();
        while ((p_res & ~COMMA) == VALUE)
        {
            if (p_res & COMMA)
                p.add (value (), value_length (), true);
            else
                p.add (value (), value_length (), false);
            p_res = (*this) ();
        }
        (*callback) (p, data);
    }
    p_res = (*this) ();
}

const int special_characters_size = 112;

static Fl_Html_Pair_Table::Entry special_characters[special_characters_size] = {

    {"Agrave;", "\xC3\x80"}, //  &#192; 	&#xC0; 	Capital A-grave
    {"agrave;", "\xC3\xA0"}, //  &#224; 	&#xE0; 	Lowercase A-grave
    {"Aacute;", "\xC3\x81"}, //  &#193; 	&#xC1; 	Capital A-acute
    {"aacute;", "\xC3\xA1"}, //  &#225; 	&#xE1; 	Lowercase A-acute
    {"Acirc;", "\xC3\x82"},  //  &#194; 	&#xC2; 	Capital A-circumflex
    {"acirc;", "\xC3\xA2"},  //  &#226; 	&#xE2; 	Lowercase A-circumflex
    {"Atilde;", "\xC3\x83"}, //  &#195; 	&#xC3; 	Capital A-tilde
    {"atilde;", "\xC3\xA3"}, //  &#227; 	&#xE3; 	Lowercase A-tilde
    {"Auml;", "\xC3\x84"},   //  &#196; 	&#xC4; 	Capital A-umlaut
    {"auml;", "\xC3\xA4"},   // 	&#228; 	&#xE4; 	Lowercase A-umlaut
    {"Aring;", "\xC3\x85"},  //	&#197; 	&#xC5; 	Capital A-ring
    {"aring;", "\xC3\xA5"},  //  &#229; 	&#xE5; 	Lowercase A-umlaut
    {"AElig;", "\xC3\x86"},  // 	&#198; 	&#xC6; 	Capital AE Ligature
    {"aelig;", "\xC3\xA6"},  // 	&#230; 	&#xE6; 	Lowercase AE Ligature
    {"Ccedil;", "\xC3\x87"}, // 	&#199; 	&#xC7; 	Capital C-cedilla
    {"ccedil;", "\xC3\xA7"}, // 	&#231; 	&#xE7; 	Lowercase C-cedilla
    {"ETH;", "\xC3\x90"},    // 	&#208; 	&#xD0; 	Capital Eth (Icelandic)
    {"eth;", "\xC3\xB0"},    // 	&#240; 	&#xF0; 	Lowercase Eth (Icelandic)
    {"Egrave;", "\xC3\x88"}, // 	&#200; 	&#xC8; 	Capital E-grave
    {"egrave;", "\xC3\xA8"}, // 	&#232; 	&#xE8; 	Lowercase E-grave
    {"Eacute;", "\xC3\x89"}, // 	&#201; 	&#xC9; 	Capital E-acute
    {"eacute;", "\xC3\xA9"}, // 	&#233; 	&#xE9; 	Lowercase E-acute
    {"Ecirc;", "\xC3\x8A"},  // 	&#202; 	&#xCA; 	Capital E-circumflex
    {"ecirc;", "\xC3\xAA"},  // 	&#234; 	&#xEA; 	Lowercase E-circumflex
    {"Euml;", "\xC3\x8B"},   // 	&#203; 	&#xCB; 	Capital E-umlaut
    {"euml;", "\xC3\xAB"},   // 	&#235; 	&#xEB; 	Lowercase E-umlaut
    {"Igrave;", "\xC3\x8C"}, // 	&#204; 	&#xCC; 	Capital I-grave
    {"igrave;", "\xC3\xAC"}, // 	&#236; 	&#xEC; 	Lowercase I-grave
    {"Iacute;", "\xC3\x8D"}, // 	&#205; 	&#xCD; 	Capital I-acute
    {"iacute;", "\xC3\xAD"}, // 	&#237; 	&#xED; 	Lowercase I-acute
    {"Icirc;", "\xC3\x8E"},  // 	&#206; 	&#xCE; 	Capital I-circumflex
    {"icirc;", "\xC3\xAE"},  // 	&#238; 	&#xEE; 	Lowercase I-circumflex
    {"Iuml;", "\xC3\x8F"},   // 	&#207; 	&#xCF; 	Capital I-umlaut
    {"iuml;", "\xC3\xAF"},   // 	&#239; 	&#xEF; 	Lowercase I-umlaut
    {"Ntilde;", "\xC3\x91"}, // 	&#209; 	&#xD1; 	Capital N-tilde
    {"ntilde;", "\xC3\xB1"}, // 	&#241; 	&#xF1; 	Lowercase N-tilde
    {"Ograve;", "\xC3\x92"}, // 	&#210; 	&#xD2; 	Capital O-grave
    {"ograve;", "\xC3\xB2"}, // 	&#242; 	&#xF2; 	Lowercase O-grave
    {"Oacute;", "\xC3\x93"}, // 	&#211; 	&#xD3; 	Capital O-acute
    {"oacute;", "\xC3\xB3"}, // 	&#243; 	&#xF3; 	Lowercase O-acute
    {"Ocirc;", "\xC3\x94"},  // 	&#212; 	&#xD4; 	Capital O-circumflex
    {"ocirc;", "\xC3\xB4"},  // 	&#244; 	&#xF4; 	Lowercase O-circumflex
    {"Otilde;", "\xC3\x95"}, // 	&#213; 	&#xD5; 	Capital O-tilde
    {"otilde;", "\xC3\xB5"}, // 	&#245; 	&#xF5; 	Lowercase O-tilde
    {"Ouml;", "\xC3\x96"},   // 	&#214; 	&#xD6; 	Capital O-umlaut
    {"ouml;", "\xC3\xB6"},   // 	&#246; 	&#xF6; 	Lowercase O-umlaut
    {"Oslash;", "\xC3\xB6"}, // 	&#216; 	&#xD8; 	Capital O-slash
    {"oslash;", "\xC3\xB8"}, // 	&#248; 	&#xF8; 	Lowercase O-slash
    {"OElig;", "\xC5\x92"},  // 	&#338; 	&#x152; Capital OE Ligature
    {"oelig;", "\xC5\x93"},  // 	&#339; 	&#x153; Lowercase OE Ligature
    {"szlig;", "\xC3\x9F"},  // 	&#223; 	&#xDF; 	Lowercase SZ Ligature
    {"THORN;", "\xC3\x9E"},  // 	&#222; 	&#xDE; 	Capital Thorn
    {"thorn;", "\xC3\xBE"},  // 	&#254; 	&#xFE; 	Lowercase Thorn
    {"Ugrave;", "\xC3\x99"}, // 	&#217; 	&#xD9; 	Capital U-grave
    {"ugrave;", "\xC3\xB9"}, // 	&#249; 	&#xF9; 	Lowercase U-grave
    {"Uacute;", "\xC3\x9A"}, // 	&#218; 	&#xDA; 	Capital U-acute
    {"uacute;", "\xC3\xBA"}, // 	&#250; 	&#xFA; 	Lowercase U-acute
    {"Ucirc;", "\xC3\x9B"},  // 	&#219; 	&#xDB; 	Capital U-circumflex
    {"ucirc;", "\xC3\xBB"},  // 	&#251; 	&#xFB; 	Lowercase U-circumflex
    {"Uuml;", "\xC3\x9C"},   // 	&#220; 	&#xDC; 	Capital U-umlaut
    {"uuml;", "\xC3\xBC"},   // 	&#252; 	&#xFC; 	Lowercase U-umlaut
    {"Yacute;", "\xC3\x9D"}, // 	&#221; 	&#xDD; 	Capital Y-acute
    {"yacute;", "\xC3\xBD"}, // 	&#253; 	&#xFD; 	Lowercase Y-acute
    {"Yuml;", "\xC2\x9F"},   // 	&#159; 	&#x9F; 	Capital Y-umlaut
    {"yuml;", "\xC3\xBF"},   // 	&#255; 	&#xFF; 	Lowercase Y-umlaut

    // Missing extended ASCII symbols
    {"frac14;", "\xC2\xBC"},
    {"frac12;", "\xC2\xBD"},
    {"frac34;", "\xC2\xBE"},
    {"curren;", "\xC2\xA4"},
    {"brvbar;", "\xC2\xA6"},
    {"uml;", "\xC2\xA8"},
    {"ordf;", "\xC2\xAA"},
    {"not;", "\xC2\xAC"},
    {"shy;", "\xC2\xAD"},
    {"macr;", "\xC2\xAF"},
    {"deg;", "\xC2\xB0"},
    {"sup2;", "\xC2\xB2"},
    {"sup3;", "\xC2\xB3"},
    {"acute;", "\xC2\xB4"},
    {"cedil;", "\xC2\xB8"},
    {"sup1;", "\xC2\xB9"},
    {"ordm;", "\xC2\xBA"},
    {"ETH;", "\xC3\x90"},
    {"times;", "\xC2\x97"},

    {"ndash;", "\xE2\x80\x93"}, // 	&#8211; 	en dash
    {"mdash;", "\xE2\x80\x94"}, // 	&#8212; 	em dash
    {"iexcl;", "\xC2\xA1"},     // 	&#161; 	inverted exclamation
    {"iquest;", "\xC2\xBF"},    // 	&#191; 	inverted question mark
    {"ldquo;", "\xE2\x80\x9C"}, // 	&#8220; 	left double curly quote
    {"rdquo;", "\xE2\x80\x9D"}, // 	&#8221; 	right double curly quote
    {"laquo;", "\xC2\xAB"},     // 	&#171; 	left guillemets
    {"raquo;", "\xC2\xBB"},     // 	&#187; 	right guillemets
    {"cent;", "\xC2\xA2"},      // 	&#162; 	cent
    {"divide;", "\xC3\xB7"},    // 	&#247; 	divide
    {"micro;", "\xC2\xB5"},     // 	&#181; 	micron
    {"middot;", "\xC2\xB7"},    // 	&#183; 	middle dot
    {"para;", "\xC2\xB6"},      // 	&#182; 	pilcrow (paragraph sign)
    {"plusmn;", "\xC2\xB1"},    // 	&#177; 	plus/minus
    {"euro;", "\xE2\x82\xAC"},  // 	&#8364; 	Euro
    {"pound;", "\xC2\xA3"},     // 	&#163; 	British Pound Sterling
    {"reg;", "\xC2\xAE"},       // 	&#174; 	registered
    {"sect;", "\xC2\xA7"},      // 	&#167; 	section
    {"trade;", "\xC2\x99"},     // 	&#153; 	trademark
    {"yen;", "\xC2\xA5"},       // 	&#165; 	Japanese Yen

    // those are most common: for speed they are placed at the end of the table
    {"lsquo;", "\xE2\x80\x98"}, // 	&#8216; 	left single curly quote
    {"rsquo;", "\xE2\x80\x99"}, // 	&#8217; 	right single curly quote
    {"copy;", "\xC2\xA9"},      // 	&#169; 	copyright
    {"quot;", "\x22"},          // 	&#34; 	quotation mark
    {"gt;", "\x3E"},            // 	&#62; 	greater than
    {"lt;", "\x3C"},            // 	&#60; 	less than
    {"amp;", "\x26"},           // 	&#38; 	ampersand
    {"nbsp;", "\xC2\xA0"}       // 	&#160; 	non-breaking space

};

static Fl_Html_Tokenizer::Special_Character_Table
    def_spec_char_table_ (special_characters, special_characters_size,
                          special_characters_size);

Fl_Html_Tokenizer::Special_Character_Table *
Fl_Html_Tokenizer::Special_Character_Table::default_table ()
{
    return &def_spec_char_table_;
}

void Fl_Xml_Parser::push_parent (Fl_Xml_Object * o)
{
    if (current_parent_ >= parent_table_size_)
    {
        int ns = 2 * parent_table_size_;
        Fl_Xml_Object ** new_parent_table = new Fl_Xml_Object *[ns];
        Fl_Xml_Object *** new_parent_table_next = new Fl_Xml_Object **[ns];
        for (int i = 0; i < parent_table_size_; i++)
        {
            new_parent_table[i] = parent_table_[i];
            new_parent_table_next[i] = parent_table_next_[i];
        }
        parent_table_size_ = ns;
        delete[] parent_table_;
        delete[] parent_table_next_;
        parent_table_ = new_parent_table;
        parent_table_next_ = new_parent_table_next;
    }
    parent_table_next_[current_parent_ + 1] =
        parent_table_[current_parent_]->aux_address ();
    parent_table_[current_parent_ + 1] = 0; // no brothers yet
    current_parent_++;
    currentParent.push (o);
}

int Fl_Xml_Parser::parse (const char * src, Fl_Xml_Object ** parse_result)
{

    // Initialization
    current_parent_ = 0;
    reset (src);
    *parent_table_ = 0;
    *parent_table_next_ = parse_result;
    *parse_result = 0;
    Fl_Xml_Object * in_tag = 0;
    const char * unprocessed_attribute_name = 0;
    int unprocessed_attribute_name_length = 0;
    Result last_result_ = (*this) ();
    while (last_result_)
    {
        if (last_result_ < 0)
            return last_result_; // parse error
        switch (last_result_)
        {
        case TAG_NAME:
            if (*value () == '/')
            { // this is a closing tag, inspecting if it has proper name
                Fl_Xml_Object * p = parent ();
                if (!p)
                {
                    if (!handle_bad_closing_tag ())
                        return BAD_CLOSING_TAG_NAME;
                }
                else
                {
                    while (
                        !(p->name_compares (value () + 1, value_length () - 1)))
                    {
                        if (!handle_bad_closing_tag ())
                            return BAD_CLOSING_TAG_NAME;
                        finish_tag ();
                        pop_parent ();
                        p = parent ();
                        if (!p && !!handle_bad_closing_tag ())
                            return BAD_CLOSING_TAG_NAME;
                    }
                    finish_tag (); // this might modify start_whitespace and the
                                   // end whitespace of the previous word
                    pop_parent ();
                    in_tag = 0;
                }
                last_result_ = (*this) (); // popping out ">" bracket
                if (last_result_ != END_TAG)
                    return BAD_CLOSING_TAG;
            }
            else
            { // creating new tag
                in_tag = create_tag (value (), value_length ());
                // add_object(in_tag);
            }
            unprocessed_attribute_name = 0;
            break;
        case ATTRIBUTE_NAME:
            if (unprocessed_attribute_name)
                in_tag->process_attribute (
                    this,
                    unprocessed_attribute_name,
                    unprocessed_attribute_name_length,
                    0,
                    0); // processing previous attribute with no value
            unprocessed_attribute_name = value ();
            unprocessed_attribute_name_length = value_length ();
            break;
        case ATTRIBUTE_VALUE:
        {
            const char * val = value ();
            int val_len = value_length ();
            if (val && val_len > 1)
            {
                char c = *val;
                if (c == '"' || c == '\'')
                {
                    val += 1;
                    val_len -= 2;
                }
            }
            if (unprocessed_attribute_name)
                in_tag->process_attribute (this,
                                           unprocessed_attribute_name,
                                           unprocessed_attribute_name_length,
                                           val,
                                           val_len);
            unprocessed_attribute_name = 0;
        }
        break;
        case END_TAG:
        case END_SELF_CLOSED_TAG:
            if (unprocessed_attribute_name)
            {
                in_tag->process_attribute (this,
                                           unprocessed_attribute_name,
                                           unprocessed_attribute_name_length,
                                           0,
                                           0);
                unprocessed_attribute_name = 0;
            }

            add_object (in_tag);
            if (last_result_ == END_TAG && in_tag && (!(in_tag->no_body ())))
                push_parent (in_tag);
            else
            {
                push_parent (in_tag);
                finish_tag ();
                pop_parent ();
            }
            in_tag = 0;
            break;
        case CDATA:
            break;
        case BODY:
            if (!process_body ())
            { // splitting body to words and creating the word objects
                const char * src = value ();
                int len = value_length ();
                const char * s = src;
                int r = get_word (s, len);
                while (r)
                {
                    Fl_Xml_Object * o = create_word (
                        s,
                        r); // this might also modify previous word whitespace
                    add_object (o);
                    s += r;
                    len -= r;
                    src = s;
                    r = get_word (s, len);
                }
            }
            break;
        case COMMENT:
        case QUESTION_COMMENT:
        case UNKNOWN_COMMENT:
            create_comment (last_result_, value (), value_length ());
            break;
        default:
            return UNKNOWN_ERROR; // unknown result of the tokenizer which can
                                  // not be handled
        }
        last_result_ = (*this) ();
    }
    if (in_tag)
        return UNFINISHED_TAG;
    Fl_Xml_Object * p = parent ();
    while (p)
    { // this indicates unclosed tags
        if (!handle_bad_closing_tag ())
            return UNCLOSED_TAG;
        finish_tag ();
        pop_parent ();
        p = parent ();
    }
    return 0;
}

bool Fl_Html_Parser::process_body ()
{
    Fl_Html_Object_ * p = (Fl_Html_Object_ *)parent ();
    if (!p || !p->process_body ())
        return false;
    const char * src = value ();
    *last_word_finish = ' ';
    last_word_concatenation = &dummy_word_concatenation;
    if (!src)
        return true;
    int len = value_length ();

    while (len > 0)
    {
        const char * s = (const char *)memchr (src, '\n', len);
        if (!s)
        {
            add_object (create_word (src, len));
            return true;
        }
        int p_len = s - src;
        int kk = p_len;
        if (kk && src[kk - 1] == '\r')
            kk--;
        add_object (create_word (src, kk, 1));
        p_len++;
        src += p_len;
        len -= p_len;
    }
    return true;
}

Fl_Xml_Object * Fl_Html_Parser::create_word (const char * word,
                                             unsigned word_len, int type)
{
    int shift = word - value (); // indicates space before first word, every
                                 // other words have stbefore space too
    if (shift)
    { // before whitespace
        *last_word_finish = ' ';
        last_word_concatenation = &dummy_word_concatenation;
    }
    Fl_Xml_Object * w;
    if (last_result () == CDATA)
        w = create_object_table_->create_word (word, word_len);
    else
        w = create_object_table_->create_word (
            special_character_table (), word, word_len, type);
    last_word_finish = (char *)(w->value () + strlen (w->value ()));
    *last_word_concatenation = w;
    int val_len = value_length ();
    if (val_len - shift != (int)word_len)
    { // afrer whitespace
        *last_word_finish = ' ';
        last_word_concatenation = &dummy_word_concatenation;
    }
    else
    {
        last_word_concatenation = w->aux_address ();
    }
    index++;
    ((Fl_Html_Object_ *)w)->index (index);
    return w;
}

void Fl_Html_Parser::add_object (Fl_Xml_Object * o)
{
    Fl_Html_Object_ * w = (Fl_Html_Object_ *)o;
    Fl_Html_Object_ * p = (Fl_Html_Object_ *)parent ();
    while (p && (p->closed_by () & w->type ()))
    {
        finish_tag ();
        pop_parent ();
        p = (Fl_Html_Object_ *)parent ();
    }
    if (w->breaks_word ())
    {
        if (1 & w->breaks_word ())
            *last_word_finish = ' ';
        last_word_concatenation = &dummy_word_concatenation;
    }
    // last_word_concatenation = &dummy_word_concatenation;

    Fl_Xml_Parser::add_object (o);
}

Fl_Xml_Object * Fl_Html_Parser::create_tag (const char * name, unsigned len)
{
    Fl_Html_Object_ * w;
    Fl_Html_Object_::Create_Tag_Function fn;
    if (create_object_table_->find (name, fn, len))
        w = (*fn) ();
    else
        w = create_object_table_->create_unknown_tag (name, len);
    /*
    if(w) {
      if(w->breaks_word()) {
        *last_word_finish = ' ';
       last_word_concatenation = &dummy_word_concatenation;
      }
        Fl_Html_Object_ * p = (Fl_Html_Object_ *)parent();
        while(p && (p->closed_by() & w->type())) {
          finish_tag();
          pop_parent();
          p = (Fl_Html_Object_ *)parent();
        }


    }
    */
    index++;
    ((Fl_Html_Object_ *)w)->index (index);
    return w;
}

int Fl_Html_Parser::parse (const char * src, Fl_Xml_Object ** parse_result)
{
    index = 0;
    char dummy_word_finish[1];
    *dummy_word_finish = 0;
    /* *** */ last_word_concatenation = &dummy_word_concatenation;

    last_word_finish = dummy_word_finish; // we dont have a word yet...
    int ret = Fl_Xml_Parser::parse (src, parse_result);
    return ret;
}

void Fl_Html_Parser::filename (const char * s)
{
    int len = 0;
    if (s)
        len = strlen (s);
    if (filename_size_ < len + 1)
    {
        delete[] filename_;
        delete[] directory_;
        filename_size_ = len + 1;
        filename_ = new char[filename_size_];
        directory_ = new char[filename_size_];
    }
    if (!len)
    {
        *filename_ = 0;
        *directory_ = 0;
        return;
    }
    memcpy (filename_, s, len + 1);
    memcpy (directory_, s, len + 1);
    char * slash = strrchr (directory_, '/');

#ifdef WIN32
    char * slash2 = strrchr (directory_, '\\');
    if (slash2 && (!slash || slash < slash2))
        slash = slash2;
#endif // WIN32
    if (slash)
        *slash = 0;
    else
        *directory_ = 0; // no slash found: no directory name was included
}

//////////////////////////

void Fl_Css_Parser::Property::add (const char * value, int v_length, bool comma)
{
    if (no_values_ >= size_)
    {
        size_ *= 2;
        const char ** nt = new const char *[size_];
        memcpy (nt, values_, no_values_ * sizeof (const char *));
        delete[] values_;
        values_ = nt;
        bool * ct = new bool[size_];
        memcpy (ct, commas_, no_values_ * sizeof (bool));
        delete[] commas_;
        commas_ = ct;
        int * vl = new int[size_];
        memcpy (vl, values_length_, no_values_ * sizeof (int));
        delete[] values_length_;
        values_length_ = vl;
    }
    values_[no_values_] = value;
    commas_[no_values_] = comma;
    values_length_[no_values_] = v_length;
    no_values_++;
}
