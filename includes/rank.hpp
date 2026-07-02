#ifndef RANK_BV
#define RANK_BV

#include <sdsl/bit_vectors.hpp>
#include <sdsl/int_vector_buffer.hpp>

using namespace sdsl;
using namespace std;


class rank_bv_64
{
    uint32_t* block;
    uint64_t u;  //bit vector length
    uint64_t n; // # ones
    
   public:
    uint64_t* seq;
    rank_bv_64() = default;
    
    rank_bv_64(bit_vector &bv)
    {
        uint64_t i;
        uint8_t byte_mask;
        uint32_t cur_word = 0, count = 0;

        u = bv.size();   
             
        seq = new uint64_t[(u+63)/64]();     
        block = new uint32_t[(u+63)/64]();
        
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

    rank_bv_64(vector<uint64_t> _bv)
    {
        // empty element fix
        if(bv_.empty()){
            u = 0;
            n = 0;
            seq = new uint64_t[1]();
            block = new uint32_t[1]();
            return;
        }
        u = _bv[_bv.size() - 1] + 1; //last element
        n = _bv.size(); // each element  is a position with a 1

        bit_vector bv = bit_vector(u, 0);
        // for each 1 in _bv, we mark it in bv
        for (int i = 0;  i < n; i++) {
            bv[_bv[i]] = 1;
        }

        uint64_t i;
        uint8_t byte_mask;
        uint32_t cur_word = 0, count = 0;

        seq = new uint64_t[(u+63)/64]();
        block = new uint32_t[(u+63)/64]();

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
    }


    inline uint64_t rank(uint64_t i) 
    {
        return block[i>>6] + bits::cnt(seq[i>>6] & ~(0xffffffffffffffff << (i&0x3f)));
    }

    inline uint8_t get_4_bits(uint64_t start_pos)
    {
        //toma la seq, toma el bloque al cual pertenece la posicion, luego lo desplaza hacia la derecha start_pos%64, dentro del bloque y toma los último 4 bits.
        return ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0x0f);
    }

    inline uint8_t get_2_bits(uint64_t start_pos)
    {
        return ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0x03);
    }

    inline uint8_t get_8_bits(uint64_t start_pos)
    {
        return ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0xff);
    }

    void print_4_bits(uint64_t start_pos)
    {
        uint8_t x = ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0x0f);

        for (int l = 0; l < 4; l++)
        {
            cout << ((x & (1 << l)) ? "1" : "0");
        }
        cout << " ";
    }


    void print_8_bits(uint64_t start_pos)
    {
        uint8_t x = ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0xff);

        for (int l = 0; l < 8; l++)
        {
            cout << ((x & (1 << l)) ? "1" : "0");
        }
        cout << " ";
    }

    uint64_t get_bits(uint64_t start_pos, k2_tree_ns::size_type dim)
    {

        switch (dim){
            case 2:
                return ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0x03);
                break;
            case 4:
                return ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0x0f);
                break;
            case 8:
                return ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0xff);
                break;
            case 16:
                return ((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0xffff);
                break;
            case 32:

                return((seq[start_pos >> 6] >>(start_pos & 0x3f) ) & 0xffffffff);
                break;

        }
        /*for (int l = 0; l < dim; l++)
        {
            cout << ((x & (1 << l)) ? "1" : "0");
        }
        cout << " ";*/
    }

    void bv_and(rank_bv_64 bv){
        seq[0] &= *bv.seq;
    }
 
    // number of bits in the bv
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
        return sizeof(uint64_t)*((u+63)/64) + sizeof(uint32_t)*(u+63)/64 
	       + sizeof(uint64_t*) + sizeof(uint32_t*)
	       + 2*sizeof(uint64_t);
    }

};

#endif