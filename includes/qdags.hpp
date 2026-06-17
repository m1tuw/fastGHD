
#ifndef INCLUDED_QDAGS
#define INCLUDED_QDAGS

#include <sdsl/bit_vectors.hpp>
#include "se_quadtree.hpp"
#include "initArray.hpp"

#include<bits/stdc++.h>
#include<ratio>
#include<chrono>
#include<ctime>

typedef uint16_t type_mapping_M;

bool compare_pairs(const pair<uint64_t, uint64_t> &i, const pair<uint64_t, uint64_t> &j) {
    return i.second < j.second;
}


class qdag {
public:

    typedef vector<uint64_t> att_set;
    se_quadtree *Q;

private:

    type_mapping_M *M;    // mapping

    att_set attribute_set;

    uint64_t grid_side;

    uint16_t Msize;  // number of children of every qdag node

    bool is_extended_qdag;

    vector<vector<type_mapping_M> *> M_prime;


public:

    qdag() = default;


    uint64_t size() {
        uint64_t s = Q->size() + Msize * sizeof(uint16_t) + attribute_set.size() * sizeof(uint64_t)
                     + M_prime.size() * sizeof(vector<type_mapping_M> *) + sizeof(uint64_t) + sizeof(uint16_t);

        for (uint64_t i = 0; i < M_prime.size(); i++)
            s += M_prime[i]->size() * sizeof(type_mapping_M);

        return s;
    }


    void setAtts(uint64_t att1, uint64_t att2) {
        attribute_set[0] = att1;
        attribute_set[1] = att2;
    }

    qdag(const qdag &_Q) {
        this->Q = _Q.Q;
        _Q.Q->inc_ref_count();
        this->M = _Q.M;
        for (uint64_t i = 0; i < _Q.attribute_set.size(); i++)
            this->attribute_set.push_back(_Q.attribute_set[i]);

        this->grid_side = _Q.grid_side;
        this->Msize = _Q.Msize;
        this->is_extended_qdag = _Q.is_extended_qdag;

    }

    qdag(std::vector<std::vector<uint64_t>> &points,
         att_set &_attribute_set,
         const uint64_t _grid_side,
         uint8_t k, uint8_t d
    ) {

        //cout << "creando qdag con k=" << unsigned(k) << " y d=" << unsigned(d) << endl;
        Msize = std::pow(k, d);
        //cout << "k^d=" << Msize << endl;

        M = new type_mapping_M[Msize];

        for (uint64_t i = 0; i < Msize; i++)
            M[i] = i;  // identity mapping

        attribute_set = _attribute_set;

        Q = new se_quadtree(points, _grid_side, k, d);

        grid_side = _grid_side;
        is_extended_qdag = false;

        //M_prime.reserve(Msize);

        for (uint64_t i = 0; i < Msize; i++)
            M_prime.push_back(new std::vector<type_mapping_M>());

        for (uint64_t i = 0; i < Msize; i++)
            M_prime[i]->push_back(i);

    }


    qdag(vector<uint64_t> bv[],
         att_set &_attribute_set,
         const uint64_t _grid_side,
         uint8_t k, uint8_t d
    ) {
        Q = new se_quadtree(bv, bv, _grid_side, k, d);

        Msize = std::pow(k, d);

        M = new type_mapping_M[Msize];

        for (uint64_t i = 0; i < Msize; i++)
            M[i] = i;  // identity mapping

        attribute_set = _attribute_set;
        grid_side = _grid_side;
        is_extended_qdag = false;

        //M_prime.reserve(Msize);

        for (uint64_t i = 0; i < Msize; i++)
            M_prime.push_back(new std::vector<type_mapping_M>());

        for (uint64_t i = 0; i < Msize; i++) {
            M_prime[M[i]]->push_back(i);
        }

    }

    ~qdag() {
        //if (Q && !is_extended_qdag) {
        //    delete Q;
        //    Q = NULL;
        //}
        if (is_extended_qdag) delete M;
    }


    qdag *extend(att_set &attribute_set_A) {
        uint16_t dim = attribute_set_A.size();
        uint16_t dim_prime = attribute_set.size();
        uint64_t p = std::pow(Q->getK(), dim);

        //cout << "extendiendo de [" << attribute_set << "] a [" << attribute_set_A << "]" << endl;

        type_mapping_M *_M = new type_mapping_M[p];

        map<uint64_t, uint64_t> indice;

        uint64_t att_index, i;
        for (uint16_t j = 0; j < dim_prime; ++j) {
            for (uint16_t i = 0; i < dim; ++i) {
                if (attribute_set_A[i] == attribute_set[j]) {
                    indice[attribute_set[j]] = i;
                    break;
                }
            }
        }

        uint64_t mask, i_prime;

        for (i = 0; i < p; ++i) {
            mask = 1 << (dim_prime - 1);
            i_prime = 0;

            for (uint16_t j = 0; j < dim_prime; ++j) {
                // Tomar todos los 1 que estan en la pos adecuada en el attribute set
                if (i & (1 << (dim - indice[attribute_set[j]] - 1)))
                    i_prime |= mask;

                mask >>= 1;
            }

            _M[i] = M[i_prime];
        }

        qdag *q = new qdag();

        q->Q = this->Q;
        q->M = _M;
        q->attribute_set = attribute_set_A;
        q->grid_side = this->grid_side;
        q->is_extended_qdag = true;
        q->Msize = p; // this.Msize;

        uint64_t j = std::pow(Q->getK(), dim_prime);

        for (i = 0; i < j; i++)
            q->M_prime.push_back(new std::vector<type_mapping_M>());

        for (i = 0; i < p; i++) {
            q->M_prime[q->M[i]]->push_back(i);
        }

        return q;
    }


    uint64_t nAttr() {
        return attribute_set.size();
    }


    uint64_t getAttr(uint64_t i) {
        return attribute_set[i];
    }


    uint64_t getGridSide() {
        return grid_side;
    }


    uint64_t getHeight() {
        return Q->getHeight();
    }


    uint64_t n_ones() {
        return Q->bv[getHeight() - 1].n_ones();
    }


    uint8_t getK() {
        return Q->getK();
    }


    uint16_t nChildren() {
        return Msize;
    }


    uint64_t getKD() {
        return Q->getKD();
    }


    uint16_t getM(uint16_t i) {
        return M[i];
    }



    void get_children(uint16_t level, const uint64_t node,
        initializable_array& C,
        uint64_t n_relations_join,
        uint16_t* children_to_recurse,
        uint64_t& size_children_to_recurse,
        uint64_t* rank_vector,
        const uint64_t k_d, int flag)
    {
        //            start_rank = high_resolution_clock::now();
        uint64_t r = Q->rank(level, node);
//            stop_rank = high_resolution_clock::now();
//            time_span_rank = duration_cast<duration<double>>(stop_rank - start_rank);
//            total_time_rank += time_span_rank.count();

        uint16_t n_children, n_active;
        uint64_t children_array[k_d], active_array[k_d];

        //start_rank = high_resolution_clock::now();
        Q->get_children(level, node, children_array, n_children);
        Q->get_children_active(level, node, active_array, n_active);
        //          stop_rank = high_resolution_clock::now();
        //          time_span_rank = duration_cast<duration<double>>(stop_rank - start_rank);
        //          total_time_rank += time_span_rank.count();

        uint16_t cur_child, i_active = 0;
        uint64_t j, size, element;

        for (uint16_t ic = 0; ic < n_children, i_active < n_active; ++ic) {
            cur_child = children_array[ic];
            rank_vector[cur_child] = ++r;

            // filter only active childs
            if (cur_child != active_array[i_active])
                continue;
            i_active++;

            size = M_prime[cur_child]->size();
            for (j = 0; j < size; ++j) {
                element = (*M_prime[cur_child])[j];
                C.increment(element);
                if (C[element] == n_relations_join) {
                    children_to_recurse[size_children_to_recurse++] = element;
                    //cout << "marked child " << "level " << level  << " node " << node << " child " << cur_child << "extended child "<< element<< endl;
                }
                //if (C[element] >= n_relations_join * 2 + 1)
                  //  cout << "level " << level  << " node " << node << " skipped child " << cur_child << endl;
            };
        }
    }

    void filterChildren(uint16_t level, const uint64_t node,
                     initializable_array &C,
                     uint64_t n_relations_join,
                     uint16_t *children_to_recurse,
                     uint64_t &size_children_to_recurse,
                     const uint64_t k_d,
                     rank_bv_64 result
    ) {
        uint64_t children_array[k_d], n_children, ic;
        Q->get_children_result(level, node, children_array, n_children, result);

        uint16_t cur_child;
        uint64_t j, size, element;

        for (ic = 0; ic < n_children; ++ic) {
            cur_child = children_array[ic];
            size = M_prime[cur_child]->size();
            for (j = 0; j < size; ++j) {
                element = (*M_prime[cur_child])[j];
                C.assign(element, n_relations_join + 1);
                //cout << "marked level " << level << " node " << node << "child " << cur_child << " extended " << element <<endl;
            }
        }

    }

    void get_children_lastlevel(const uint64_t node,
        initializable_array& C,
        uint64_t n_relations_join,
        uint16_t* children_to_recurse,
        uint64_t& size_children_to_recurse,
        const uint64_t k_d)
    {
        uint16_t n_children;
        uint64_t children_array[k_d];

        Q->get_children(Q->getHeight() - 1, node, children_array, n_children);

        uint16_t cur_child;
        uint64_t j, size, element;

        for (uint16_t ic = 0; ic < n_children; ++ic) {
            cur_child = children_array[ic];
            size = M_prime[cur_child]->size();
            for (j = 0; j < size; ++j) {
                element = (*M_prime[cur_child])[j];
                C.increment(element);
                if (C[element] == n_relations_join)
                    children_to_recurse[size_children_to_recurse++] = element;
            }
        }
    }

    void print(std::ostream &ost) {
        Q->print(ost);
    }


    void print_active(std::ostream &ost) {
        Q->print_active(ost);
    }
};

#endif