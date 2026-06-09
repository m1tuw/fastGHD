#ifndef INIT_ARRAY
#define INIT_ARRAY

#include <bits/stdc++.h>


class initializable_array
{
    uint64_t D; // array size

    uint64_t V[2048];  // array

    uint64_t S[2048];
    uint64_t top;

    uint64_t U[2048]; // auxiliary array

public:

    initializable_array() = default;


    initializable_array(const uint64_t _D)
    {
        D = _D;
        top = 0;
    }


    ~initializable_array()
    {
    }


    inline uint64_t operator[](const uint64_t i) const
    {
        return V[i];
    }


    void increment(const uint64_t i)
    {
        if (U[i] < top && S[U[i]] == i)
            ++V[i];
        else {
            V[i] = 1;
            U[i] = top;
            S[top++] = i;
        }
    }

    void assign(const uint64_t i, const uint64_t value)
    {
        if (U[i] < top && S[U[i]] == i)
            V[i] = value;
        else {
            V[i] = value;
            U[i] = top;
            S[top++] = i;
        }
    }

};

#endif