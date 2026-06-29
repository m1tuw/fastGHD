#ifndef RANK_BV
#define RANK_BV

#include <bitset>
#include <sdsl/bit_vectors.hpp>
#include <sdsl/int_vector_buffer.hpp>

using namespace sdsl;
using namespace std;


class rank_bv_64
{
    uint32_t* block;
    uint64_t u;  //bit vector length
    uint64_t n; // # ones
    uint64_t nw; // number of 64-bit words in array seq

public:
    rank_bv_64() = default;
    uint64_t* seq;

    rank_bv_64(bit_vector& bv, int kd)
    {
        uint64_t i;
        uint8_t byte_mask;
        uint32_t cur_word = 0, count = 0;

        u = bv.size();

        if (kd < 64) {
            nw = (u + 63) / 64;
        } else {
            nw = (u + kd - 1) / kd * (kd / 64);
        }
        //cout << "u: " << u << " nw: " << nw << " kd: " << kd << endl;
        seq = new uint64_t[nw]();
        block = new uint32_t[nw]();

        for (i = 0; i < u; ++i) {

            if (i%64 == 0)
                block[cur_word++] = count;

            if (bv[i]) {
                count++;
                seq[i/64] |= (1L<<(i%64));
            }
            else
                seq[i/64] &= ~(1L<<(i%64));

        }
        n = count;
    }

    rank_bv_64(vector<uint64_t> _bv, int kd)
    {
        // fix: segfault when query yields 0 results
        if(_bv.empty()){
            u = 0;
            n = 0;

            seq = new uint64_t[1]();
            block = new uint32_t[1]();

            return;
        }
        //cout<< "esto llega a rank " << _bv << endl;
        u = _bv[_bv.size() - 1] + 1; //last element
        n = _bv.size(); // each element  is a position with a 1
        if (kd < 64) {
            nw = (u + 63) / 64;
        } else {
            nw = (u + kd - 1) / kd * (kd / 64);
        }
        //cout << "u: " << u << " nw: " << nw << " kd: " << kd << endl;
        u = 64 * nw;
        bit_vector bv = bit_vector(u, 0);
        // for each 1 in _bv, we mark it in bv
        for (int i = 0;  i < n; i++) {
            bv[_bv[i]] = 1;
        }

        uint64_t i;
        uint8_t byte_mask;
        uint32_t cur_word = 0, count = 0;

        seq = new uint64_t[nw]();
        block = new uint32_t[nw]();

        for (i = 0; i < u; ++i) {

            if (i%64 == 0)
                block[cur_word++] = count;

            if (bv[i]) {
                count++;
                seq[i / 64] |= (1ULL << (i % 64));
            }
            else
                seq[i / 64] &= ~(1ULL << (i % 64));
        }
    }

    rank_bv_64 clone_empty()
    {
        rank_bv_64 *bv = new rank_bv_64();
        bv->n = this->n;
        bv->u = this->u;
        bv->nw = this->nw;
        bv->seq = new uint64_t[(u+63)/64]();
        bv->block = new uint32_t[(u+63)/64]();
        for (uint64_t i = 0; i < nw; i++) {
            bv->seq[i] &= 0;
        }

        return *bv;
    }

    inline uint64_t rank(uint64_t i)
    {
        return block[i>>6] + bits::cnt(seq[i>>6] & ~(0xffffffffffffffff << (i&0x3f)));
    }

    // Given a position i, returns the position of the next 1 in the bit vector
    // returns u if there is no next 1
    uint64_t select_next(uint64_t i)
    {
        uint64_t t = seq[i/64] & (0xffffffffffffffff << ((i%64)));

        if (t) {
            return bits::lo(t) + i - (i%64);
        } else {
            uint64_t j = i/64;
            while (++j < nw && !seq[j]);
            if (j == nw) return u; // there is no next 1
            //cout << "el bit siguiente  " << bits::lo(seq[j]) + j*64 << endl;
            return bits::lo(seq[j]) + j*64;
        }
    }

    uint64_t select_next_active(uint64_t i, rank_bv_64 active)
    {

        uint64_t t = (seq[i/64] & active.seq[i/64]) & (0xffffffffffffffff << (i%64));

        if (t) {
            return bits::lo(t) + i - (i%64);
        } else {
            uint64_t j = i/64;
            while (++j < nw && !(seq[j] & active.seq[j]));
            if (j == nw){
                return u; // there is no next 1
            }
            return bits::lo(seq[j] & active.seq[j]) + j*64;
        }
    }

    // number of bits in the bitvector
    inline uint64_t size()
    {
        return u;
    }

    inline uint64_t n_ones()
    {
        return n;
    }

    inline uint64_t size_in_bytes()
    {
        return sizeof(uint64_t)*nw + sizeof(uint32_t)*nw
               + sizeof(uint64_t*) + sizeof(uint32_t*)
               + 3*sizeof(uint64_t);
    }

    vector<uint64_t> get_bits(uint64_t start_pos, uint64_t dim)
    {
        uint64_t mask, shift, size = (dim + 63) / 64;
        vector<uint64_t> result = vector<uint64_t>(size);
        for (size_t i = 0; i < size; i++, dim -= 64, start_pos += 64) {
            shift = start_pos & 0x3f;
            if (dim >= 64) {
                result[i] = (seq[start_pos >> 6] >> shift);
            } else {
                mask = (1ULL << dim) - 1;
                result[i] = (seq[start_pos >> 6] >> shift) & mask;
            }
        }

        return result;
    }

    void bv_and(rank_bv_64 bv){
        for (uint64_t i = 0; i < nw; i++) {
            seq[i] &= bv.seq[i];
        }
    }

    void mark_bit(uint64_t i){
        seq[i>>6] |= 1ULL << (i % 64);
    }

    void empty() {
        for (uint64_t i = 0; i < nw; i++) {
            seq[i] &= 0;
        }
    }
};

#endif