/*
Project: Arsene's Zero
File: zero_utility.h
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

#ifndef ZERO_UTILITY_H

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#include <math.h>

#define function static
#define local_persist static
#define global static

typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef __int8 s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef size_t mem_index;

typedef __int32 b32;
typedef bool b8;

typedef float f32;
typedef double f64;

#if ZERO_SLOW
#define ASSERT(check) if(!(check)) {*(s32 *)0 = 0;}
#define INVALID_CODE_PATH ASSERT(false)
#else
#define ASSERT(check) 
#define INVALID_CODE_PATH 
#endif

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define MAX(a, b) ((a > b) ? a : b)

#define TIMEOUT_MS 16

#define TIMING_COLUMN_COUNT_BASE 8

inline void CopyMem(void *dest, void *src, s32 size)
{
    u8 *destP = (u8 *)dest;
    u8 *srcP = (u8 *)src;
    while (size--)
    {
        *destP++ = *srcP++;
    }
}

inline s32 StringLength(char *str)
{
    s32 result = 0;
    while (*str++)
	{
		++result;
	}
    return result;
}

inline s32 RoundF32ToS32(f32 value)
{
    s32 result = (s32)(value + 0.5f);
    return result;
}

inline s32 Abs(s32 value)
{
    s32 result = (value < 0) ? -value : value;
    return result;
}

inline f32 Abs(f32 value)
{
    f32 result = (value < 0) ? -value : value;
    return result;
}

inline s32 Sq(s32 value)
{
    s32 result = value * value;
    return result;
}

inline f32 SqRt(f32 value)
{
    f32 result = sqrtf(value);
    return result;
}

inline s32 SqRt(s32 value)
{
    s32 result = (s32)sqrt(value);
    return result;
}

inline f32 Pow(f32 value, s32 exponent)
{
    ASSERT(exponent >= 0);
    
    f32 result = value;
    if (exponent == 0)
    {
        result = 1.0f;
    }
    else
    {
        while (--exponent)
        {
            result *= value;
        }
    }
    return result;
}

inline void FilterSecToMMSSMS(f64 timeInfo, s32 *min, s32 *sec, s32 *dec)
{
    f64 decimalValue = timeInfo - (s32)timeInfo;
    decimalValue *= 1000;
    *dec = (s32)decimalValue;
    
    *sec = (s32)timeInfo;
    while (*sec >= 60)
    {
        *min += 1;
        *sec -= 60;
    }
}

inline void FormatLapTime(f64 time, char *text)
{
    s32 min = 0;
    s32 sec = 0;
    s32 dec = 0;
    FilterSecToMMSSMS(time, &min, &sec, &dec);
    
    if (min >= 60 || sec < 0)
    {
        min = 0;
        sec = 0;
        dec = 0;
    }
    
    stbsp_sprintf(text, "%02d:%02d.%03d\0", min, sec, dec);
}

#ifndef WITHOUT_IMGUI
inline void CenteredText(char *str)
{
    f32 fontSize = ImGui::GetFontSize() * (StringLength(str) / 2.0f);
    ImGui::SameLine(ImGui::GetWindowSize().x / 2.0f - fontSize + (fontSize / 2.0f));
    ImGui::Text(str);
}
#endif


#define ZERO_UTILITY_H
#endif //ZERO_UTILITY_H
