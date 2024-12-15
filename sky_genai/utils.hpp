#pragma once

#include <cstdio>
#include <cstdint>

namespace mod
{

inline int uuid_to_string(uint8_t* a1, char* s) {
    return sprintf(
        s,
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        a1[0],
        a1[1],
        a1[2],
        a1[3],
        a1[4],
        a1[5],
        a1[6],
        a1[7],
        a1[8],
        a1[9],
        a1[10],
        a1[11],
        a1[12],
        a1[13],
        a1[14],
        a1[15]
    );
};

}