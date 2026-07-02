#include <algorithm>
#include "qdags.hpp"

void tobinary(unsigned number) {
    if (number > 1) {
        tobinary(number / 2);
    }
    cout << number % 2;
}


void mark_result_bv(vector<rank_bv_64> temp_bv, uint16_t cur_level, uint64_t i) {
    //cout << "mark result " << cur_level << " " << i << endl;
    temp_bv[cur_level].mark_bit(i);
}


bool propagate_active(qdag* Q, uint16_t cur_level, uint16_t max_level, vector<rank_bv_64> temp_bv, uint64_t node){
    bool has_children = false;
    uint64_t children;
    if (cur_level == max_level) {
        //caso base: hay un 1 en un nodo hoja de active
        vector<uint64_t> x = temp_bv[cur_level].get_bits(node, Q->getKD());
        for (uint64_t& it : x) {
            if (it != 0) {
                has_children = true;
            }
        }
    } else {

        //caso recursivo: debo revisar si hay 1s en el nivel inferior, hijo por hijo
        uint64_t root_temp;
        uint64_t rank_vector[64];

        uint64_t k_d = Q->getKD();
        uint64_t r = Q->Q->rank(cur_level, node);

        uint16_t n_children;
        uint64_t children_array[k_d];
        Q->Q->get_children(cur_level, node, children_array, n_children);

        // por cuántos hijos voy a bajar, cuenta la cantitdad de 1s en un arreglo de bits/entero
        uint64_t children_to_recurse_size = bits::cnt((uint64_t) children);
        uint64_t i = 0;
        uint16_t child;

        // bajar por todos los hijos marcados
        for (i = 0; i < n_children; ++i) {

            // hijo actual
            child = children_array[i];
            rank_vector[child] = ++r;

            root_temp = k_d * (rank_vector[child] - 1);

            if (propagate_active(Q, cur_level + 1, max_level, temp_bv, root_temp)) {
                mark_result_bv(temp_bv, cur_level, node + child);
                has_children = true;
            }
        }
    }

    return has_children;

}

/*
void ANDCount(qdag *Q[], uint64_t *roots, uint16_t nQ,
              uint16_t cur_level, uint16_t max_level,
              uint64_t &ntuples, uint64_t nAtt) {

    uint64_t p = Q[0]->nChildren();

    uint64_t i;
    uint64_t children_to_recurse_size = 0;

    if (cur_level == max_level) {
        uint32_t children = 0xffffffff;
        for (i = 0; i < nQ && children; ++i) {
            if (nAtt == 3)
                children &= Q[i]->materialize_node_3_lastlevel(cur_level, roots[i]);
            else if (nAtt == 4)
                children &= Q[i]->materialize_node_4_lastlevel(cur_level, roots[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5_lastlevel(cur_level, roots[i]);
        }
        ntuples += bits::cnt((uint64_t) children);
        return;
    } else {
        uint64_t k_d[16];       // CUIDADO, solo hasta 16 relaciones por query
        uint64_t root_temp[16]; // CUIDADO, solo hasta 16 relaciones por query
        uint64_t rank_vector[16][64];
        uint16_t children_to_recurse[512]; // CUIDADO, solo hasta 9 atributos distintos

        uint32_t children = 0xffffffff;
        for (i = 0; i < nQ && children; ++i) {
            k_d[i] = Q[i]->getKD();
            if (nAtt == 3)
                children &= Q[i]->materialize_node_3(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 4)
                children &= Q[i]->materialize_node_4(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5(cur_level, roots[i], rank_vector[i]);
        }

        children_to_recurse_size = bits::cnt((uint64_t) children);
        i = 0;
        uint64_t msb;

        while ( i < children_to_recurse_size) {
            msb = __builtin_clz(children);
            children_to_recurse[i] = msb;
            ++i;
            children &= (((uint32_t) 0xffffffff) >> (msb + 1));
        }

        uint16_t child;

        for (i = 0; i < children_to_recurse_size; ++i) {
            child = children_to_recurse[i];

            for (uint64_t j = 0; j < nQ; j++)
                root_temp[j] = k_d[j] * (rank_vector[j][Q[j]->getM(child)] - 1);

            ANDCount(Q, root_temp, nQ, cur_level + 1, max_level, ntuples, nAtt);
        }
    }
}*/

/*
void parANDCount(uint16_t totalThreads, uint16_t threadId, uint16_t levelOfCut,
                 qdag *Q[], uint64_t *roots, uint16_t nQ,
                 uint16_t cur_level, uint16_t max_level,
                 uint64_t &ntuples, uint64_t nAtt, uint64_t ancestor[])
{

    uint64_t p = Q[0]->nChildren();

    uint64_t i;
    uint64_t children_to_recurse_size = 0;

    if (cur_level == max_level)
    {
        uint32_t children = 0xffffffff;
        for (i = 0; i < nQ && children; ++i)
        {
            if (nAtt == 3)
                children &= Q[i]->materialize_node_3_lastlevel(cur_level, roots[i]);
            else if (nAtt == 4)
                children &= Q[i]->materialize_node_4_lastlevel(cur_level, roots[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5_lastlevel(cur_level, roots[i]);
        }
        ntuples += bits::cnt((uint64_t)children);
        return;
    }
    else
    {
        uint64_t k_d[16]; //nQ       CUIDADO, solo hasta 16 relaciones por query
        uint64_t root_temp[16]; // nQ CUIDADO, solo hasta 16 relaciones por query
        uint64_t rank_vector[16][64];
        uint16_t children_to_recurse[512]; // CUIDADO, solo hasta 9 atributos distintos

        uint32_t children = 0xffffffff;
        for (i = 0; i < nQ && children; ++i)
        {
            k_d[i] = Q[i]->getKD();
            if (nAtt == 3)
                children &= Q[i]->materialize_node_3(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 4)
                children &= Q[i]->materialize_node_4(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5(cur_level, roots[i], rank_vector[i]);
        }

        children_to_recurse_size = bits::cnt((uint64_t)children);
        i = 0;
        uint64_t msb;
        uint64_t firstNodeId;

        if (cur_level == levelOfCut)
        {
            firstNodeId = 0;
            uint64_t nodesPerLevel = 1 << nAtt;
            uint64_t multiplier = nodesPerLevel;

            for (int level = cur_level - 1; level >= 0; level--)
            {
                firstNodeId += ancestor[level] * multiplier;
                multiplier *= nodesPerLevel;
            }
        }

        while (i < children_to_recurse_size)
        {
            msb = __builtin_clz(children);
            children_to_recurse[i] = msb;
            ++i;
            children &= (((uint32_t)0xffffffff) >> (msb + 1));
        }

        uint16_t child;

        for (i = 0; i < children_to_recurse_size; ++i)
        {
            child = children_to_recurse[i];
            // cout << child << ' ';

            if (cur_level == levelOfCut)
            {
                // CHECK WHETHER TO SKIP THIS NODE
                // THIS ASSUMES THE FIRST LEVELS ARE FULL
                if ((firstNodeId + child) % totalThreads != threadId)
                    // This node corresponds to another thread, skip it!
                    continue;
            }



            for (uint64_t j = 0; j < nQ; j++)
                root_temp[j] = k_d[j] * (rank_vector[j][Q[j]->getM(child)] - 1);

            if (cur_level <= levelOfCut)
            {
                ancestor[cur_level] = child;
                parANDCount(totalThreads, threadId, levelOfCut, Q, root_temp, nQ, cur_level + 1, max_level, ntuples, nAtt, ancestor);
            }
            else
                ANDCount(Q, root_temp, nQ, cur_level + 1, max_level, ntuples, nAtt);
        }
    }
}
*/

bool AND(qdag *Q[], uint64_t *roots, uint16_t nQ,
         uint16_t cur_level, uint16_t max_level,
         vector<uint64_t> bv[], uint64_t last_pos[], uint64_t nAtt,
         bool bounded_result, uint64_t UPPER_BOUND) {
    //cout << "------------ entro al AND " << cur_level << endl;
    uint64_t p = Q[0]->nChildren();
    bool result = false;
    //uint64_t root_temp[nQ];
    bool just_zeroes = true;
    uint64_t k_d[nQ];

    uint16_t children_to_recurse[p];
    initializable_array C(p);

    //uint64_t rank_vector[16][64];

    uint64_t i;
    uint64_t children_to_recurse_size = 0;


    if (cur_level == max_level) {
        for (i = 0; i < nQ; ++i) {
            k_d[i] = Q[i]->getKD();
            Q[i]->get_children_lastlevel(roots[i], C, nQ, children_to_recurse, children_to_recurse_size, k_d[i]);
        }

        std::sort(children_to_recurse, children_to_recurse + children_to_recurse_size);

        int64_t last_child = -1;
        uint16_t child;

        for (i = 0; i < children_to_recurse_size; ++i) {
            //cout << "match encontrado" <<endl;

            child = children_to_recurse[i];
            //cout << "level " << cur_level << "  " << child << endl;

            if (child - last_child > 1)
                last_pos[cur_level] += (child - last_child - 1);

            last_child = child;

            if (bounded_result && bv[max_level].size() >= UPPER_BOUND) {
                return false;
            } else {
                bv[cur_level].push_back(last_pos[cur_level]++);

                just_zeroes = false;
            }

        }

        if (p - last_child > 1)
            last_pos[cur_level] += (p - last_child - 1);
    } else {
        uint64_t root_temp[nQ];
        uint64_t level_size = 0;
        for (i = 0; i < nQ; ++i) {
            level_size = std::max(level_size, Q[i]->Q->bv->size());
        }
        uint64_t rank_vector[nQ][level_size];
        for (i = 0; i < nQ; ++i) {
            k_d[i] = Q[i]->getKD();
            Q[i]->get_children(cur_level, roots[i], C, nQ, children_to_recurse, children_to_recurse_size, rank_vector[i], k_d[i], i);
        }

        std::sort(children_to_recurse, children_to_recurse + children_to_recurse_size);

        int64_t last_child = -1;
        uint16_t child;

        // bajar por todos los hijos marcados
        for (i = 0; i < children_to_recurse_size; ++i) {

            // hijo actual
            child = children_to_recurse[i];

            // obtener la raíz de cada qdag, dónde está en el quadtree
            for (uint64_t j = 0; j < nQ; j++)
                root_temp[j] = k_d[j] * (rank_vector[j][Q[j]->getM(child)] - 1);

            if (child - last_child > 1)
                last_pos[cur_level] += (child - last_child - 1);

            last_child = child;

            if (bounded_result && bv[max_level].size() >= UPPER_BOUND)
                return false;
            else if (cur_level == max_level ||
                     AND(Q, root_temp, nQ, cur_level + 1, max_level, bv, last_pos, nAtt, bounded_result, UPPER_BOUND)) {
                //si se llega al último nivel o si hay resultados en el subárbol, se pone un 1 en la posición para indicar que hay resultados
                //cout << "level " << cur_level << "  " << last_pos[cur_level]+1 << endl;
                bv[cur_level].push_back(last_pos[cur_level]++);
                just_zeroes = false;
            } else {
                if (cur_level < max_level)
                    last_pos[cur_level + 1] -= p;
                last_pos[cur_level]++;
            }
        }

        if (p - last_child > 1)
            last_pos[cur_level] += (p - last_child - 1);
    }
    return !just_zeroes;
}

/*
bool parAND(uint16_t totalThreads, uint16_t threadId, uint16_t levelOfCut, std::mutex &sharedMutex,
            qdag *Q[], uint64_t *roots, uint16_t nQ,
            uint16_t cur_level, uint16_t max_level,
            vector<uint64_t> bv[], uint64_t last_pos[], uint64_t ancestor[], uint64_t nAtt,
            bool bounded_result, uint64_t UPPER_BOUND)
{
    uint64_t p = Q[0]->nChildren();
    bool result = false;
    //uint64_t root_temp[nQ];
    bool just_zeroes = true;
    uint64_t k_d[16]; // nQ  CUIDADO, solo hasta 16 relaciones por query

    uint16_t children_to_recurse[512]; //p CUIDADO, solo hasta 9 atributos distintos por query

    uint64_t i;
    uint64_t children_to_recurse_size = 0;

    uint32_t children = 0xffffffff;

    if (cur_level == max_level)
    {
        for (i = 0; i < nQ && children; ++i)
        {
            //k_d[i] = Q[i]->getKD();
            if (nAtt == 3)
                children &= Q[i]->materialize_node_3_lastlevel(cur_level, roots[i]);
            else if (nAtt == 4)
                children &= Q[i]->materialize_node_4_lastlevel(cur_level, roots[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5_lastlevel(cur_level, roots[i]);
        }

        children_to_recurse_size = bits::cnt((uint64_t)children);
        i = 0;
        uint64_t msb;

        while (i < children_to_recurse_size)
        {
            msb = __builtin_clz(children);
            children_to_recurse[i] = msb;
            ++i;
            children &= (((uint32_t)0xffffffff) >> (msb + 1));
        }

        int64_t last_child = -1;
        uint16_t child;

        for (i = 0; i < children_to_recurse_size; ++i)
        {
            child = children_to_recurse[i];

            if (child - last_child > 1)
                last_pos[cur_level] += (child - last_child - 1);

            last_child = child;
            if (bounded_result && bv[max_level].size() >= UPPER_BOUND)
                return false;
            else
            {
                sharedMutex.lock();
                bv[cur_level].push_back(last_pos[cur_level]++);
                sharedMutex.unlock();

                just_zeroes = false;
            }
        }

        if (p - last_child > 1)
            last_pos[cur_level] += (p - last_child - 1);
    }
    else
    {
        uint64_t root_temp[16]; // nQ CUIDADO, solo hasta 16 relaciones por query
        uint64_t rank_vector[16][64];

        for (i = 0; i < nQ && children; ++i)
        {
            k_d[i] = Q[i]->getKD();
            if (nAtt == 3)
                children &= Q[i]->materialize_node_3(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 4)
                children &= Q[i]->materialize_node_4(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5(cur_level, roots[i], rank_vector[i]);
        }

        children_to_recurse_size = bits::cnt((uint64_t)children);
        i = 0;
        uint64_t msb;

        uint64_t firstNodeId;

        if (cur_level == levelOfCut)
        {
            firstNodeId = 0;
            uint64_t nodesPerLevel = 1 << nAtt;
            uint64_t multiplier = nodesPerLevel;

            for (int level = cur_level - 1; level >= 0; level--)
            {
                firstNodeId += ancestor[level] * multiplier;
                multiplier *= nodesPerLevel;
            }
        }

        while (i < children_to_recurse_size)
        {
            msb = __builtin_clz(children);
            children_to_recurse[i] = msb;
            ++i;
            children &= (((uint32_t)0xffffffff) >> (msb + 1));
        }

        int64_t last_child = -1;
        uint16_t child;

        for (i = 0; i < children_to_recurse_size; ++i)
        {

            child = children_to_recurse[i];

            if (cur_level == levelOfCut)
            {
                // CHECK WHETHER TO SKIP THIS NODE
                // THIS ASSUMES THE FIRST LEVELS ARE FULL
                if ((firstNodeId + child) % totalThreads != threadId)
                    // This node corresponds to another thread, skip it!
                    continue;
            }

            for (uint64_t j = 0; j < nQ; j++)
                root_temp[j] = k_d[j] * (rank_vector[j][Q[j]->getM(child)] - 1);

            if (child - last_child > 1)
                last_pos[cur_level] += (child - last_child - 1);

            last_child = child;

            if (bounded_result && bv[max_level].size() >= UPPER_BOUND)
                return false;

            ancestor[cur_level] = child;
            if (cur_level == max_level ||
                //  (cur_level > levelOfCut
                //       ? AND(Q, root_temp, nQ, cur_level + 1, max_level, bv, last_pos, nAtt, bounded_result, UPPER_BOUND)
                //       : parAND(totalThreads, threadId, levelOfCut, Q, root_temp, nQ, cur_level + 1, max_level, bv, last_pos, nAtt, bounded_result, UPPER_BOUND))
                parAND(totalThreads, threadId, levelOfCut, sharedMutex, Q, root_temp, nQ, cur_level + 1, max_level, bv, last_pos, ancestor, nAtt, bounded_result, UPPER_BOUND))
            {
                sharedMutex.lock();
                bv[cur_level].push_back(last_pos[cur_level]++);
                sharedMutex.unlock();

                just_zeroes = false;
            }
            else
            {
                if (cur_level < max_level)
                    last_pos[cur_level + 1] -= p;
                last_pos[cur_level]++;
            }
        }

        if (p - last_child > 1)
            last_pos[cur_level] += (p - last_child - 1);
    }

    return !just_zeroes;
}
*/


bool SemiAND(qdag **Q, uint64_t *roots, uint16_t nQ,
             uint16_t cur_level, uint16_t max_level, uint64_t *last_pos, uint64_t nAtt,
             bool bounded_result, uint64_t UPPER_BOUND, vector<rank_bv_64> result_bv) {
    uint64_t p = Q[0]->nChildren();
    bool result = false;
    //uint64_t root_temp[nQ];
    bool just_zeroes = true;
    uint64_t k_d[nQ]; //CUIDADO, solo hasta 16 relaciones por query

    uint16_t children_to_recurse[p];
    initializable_array C(p);

    uint64_t i;
    uint64_t children_to_recurse_size = 0;

    if (cur_level == max_level) {
        for (i = 0; i < nQ; ++i) {
            k_d[i] = Q[i]->getKD();
            Q[i]->get_children_lastlevel(roots[i], C, nQ, children_to_recurse, children_to_recurse_size, k_d[i]);
        }
        std::sort(children_to_recurse, children_to_recurse + children_to_recurse_size);

        int64_t last_child = -1;
        uint16_t child;
        uint16_t child_left_Q;

        for (i = 0; i < children_to_recurse_size; ++i) {
            child = children_to_recurse[i];
            if (child - last_child > 1) {
                last_pos[cur_level] += (child - last_child - 1);
                // si la diferencia es mayor a 1, se debe agregar. Se resta 1 porque al hacer pushback se hace =
            }

            last_child = child;
            if (bounded_result)
                return false;
            else {

                // obtener el bit del nodo
                //cout << endl
                     //<< "----------------------------------------" << endl
                     //<< "encontro match: " << roots[0] + Q[0]->getM(last_pos[cur_level] % p) << endl;
                mark_result_bv(result_bv, cur_level, roots[0] + Q[0]->getM(last_pos[cur_level] % p));
                last_pos[cur_level]++;
                just_zeroes = false;
            }
        }

        if (p - last_child > 1) { // mover puntero al inicio del siguiente nodo
            last_pos[cur_level] += (p - last_child - 1);
        }//(p_left - last_child - 1); //
    } else {
        // arreglo con raices que serán usadas en llamada recursiva
        uint64_t root_temp[nQ];
        uint64_t level_size = 0;
        for (i = 0; i < nQ; ++i) {
            level_size = std::max(level_size, Q[i]->Q->bv->size());
        }
        uint64_t rank_vector[nQ][level_size];

        k_d[0] = Q[0]->getKD();
        Q[0]->filterChildren(cur_level, roots[0], C, nQ, children_to_recurse, children_to_recurse_size, k_d[0], result_bv[cur_level]);
        for (i = 0; i < nQ; ++i) {
            k_d[i] = Q[i]->getKD();
            Q[i]->get_children(cur_level, roots[i], C, nQ, children_to_recurse, children_to_recurse_size, rank_vector[i], k_d[i], i);
        }


        std::sort(children_to_recurse, children_to_recurse + children_to_recurse_size);


        int64_t last_child = -1;
        uint16_t child;

        // bajar por todos los hijos marcados
        for (i = 0; i < children_to_recurse_size; ++i) {

            // hijo actual
            child = children_to_recurse[i];

            // obtener la raíz de cada qdag, dónde está en el quadtree
            for (uint64_t j = 0; j < nQ; j++)
                root_temp[j] = k_d[j] * (rank_vector[j][Q[j]->getM(child)] - 1);//hijo equivalente en el quadtree

            if (child - last_child > 1)
                last_pos[cur_level] += (child - last_child - 1);

            last_child = child;

            if (bounded_result)// TODO: fix bounded result
                return false;
            else if (cur_level == max_level ||
                     SemiAND(Q, root_temp, nQ, cur_level + 1, max_level, last_pos, nAtt, bounded_result, UPPER_BOUND,
                             result_bv)) {
                // si se llega al último nivel o si hay resultados en el subárbol, se pone un 1 en la posición para
                // indicar que hay resultados
                // check if my children are marked, if they are, mark me.
                vector<uint64_t> result_children = result_bv[cur_level + 1].get_bits(root_temp[0], Q[0]->Q->getKD());
                vector<uint64_t> leftQ_children = Q[0]->Q->bv[cur_level + 1].get_bits(root_temp[0], Q[0]->Q->getKD());

                if (result_children == leftQ_children) {
                    mark_result_bv(result_bv, cur_level, roots[0] + Q[0]->getM(last_pos[cur_level] % p));
                }
                last_pos[cur_level]++;

                just_zeroes = false;
            } else {
                if (cur_level < max_level)
                    last_pos[cur_level + 1] -= p;
                last_pos[cur_level]++;
            }
        }

        if (p - last_child > 1) {
            last_pos[cur_level] += (p - last_child - 1);
        }
    }

    return !just_zeroes;
}

/*
uint64_t multiJoinCount(vector<qdag> &Q) {
    qdag::att_set A;
    map<uint64_t, uint8_t> attr_map;

    // computes the union of the attribute sets
    for (uint64_t i = 0; i < Q.size(); i++) {
        uint64_t nAttr = Q[i].nAttr();
        for (uint64_t j = 0; j < nAttr; j++)
            attr_map[Q[i].getAttr(j)] = 1;
    }

    for (map<uint64_t, uint8_t>::iterator it = attr_map.begin(); it != attr_map.end(); it++)
        A.push_back(it->first);

    qdag *Q_star[Q.size()];
    uint64_t Q_roots[Q.size()];

    for (uint64_t i = 0; i < Q.size(); i++) {
        Q_star[i] = Q[i].extend(A);
        if (A.size() == 3)
            Q_star[i]->createTableExtend3();
        else if (A.size() == 4)
            Q_star[i]->createTableExtend4();
        else if (A.size() == 5)
            Q_star[i]->createTableExtend5();
        else {
            cout << "Code only works for queries of up to 5 attributes..." << endl;
            exit(1);
        }
        Q_roots[i] = 0; // root of every qdag
    }

    uint64_t ntuples = 0;
    ANDCount(Q_star, Q_roots, Q.size(), 0, Q_star[0]->getHeight() - 1, ntuples, A.size());

    for (uint64_t i = 0; i < Q.size(); i++)
        delete Q_star[i];

    return ntuples;
}
*/
/*
uint64_t parMultiJoinCount(vector<qdag> &Q)
{
    qdag::att_set A;
    map<uint64_t, uint8_t> attr_map;

    // computes the union of the attribute sets
    for (uint64_t i = 0; i < Q.size(); i++)
    {
        uint64_t nAttr = Q[i].nAttr();
        for (uint64_t j = 0; j < nAttr; j++)
            attr_map[Q[i].getAttr(j)] = 1;
    }

    for (map<uint64_t, uint8_t>::iterator it = attr_map.begin(); it != attr_map.end(); it++)
        A.push_back(it->first);

    qdag *Q_star[Q.size()];
    uint64_t Q_roots[Q.size()];

    for (uint64_t i = 0; i < Q.size(); i++)
    {
        Q_star[i] = Q[i].extend(A);
        if (A.size() == 3)
            Q_star[i]->createTableExtend3();
        else if (A.size() == 4)
            Q_star[i]->createTableExtend4();
        else if (A.size() == 5)
            Q_star[i]->createTableExtend5();
        else
        {
            cout << "Code only works for queries of up to 5 attributes..." << endl;
            exit(1);
        }
        Q_roots[i] = 0; // root of every qdag
    }

    uint64_t ntuples = 0;

    // -------
    unsigned nb_threads_hint = THREADS_BY_CORE * std::thread::hardware_concurrency();
    unsigned nb_threads = nb_threads_hint == 0 ? 8 : (nb_threads_hint);
    uint16_t levelOfCut = 5;

    std::mutex tuplesMutex;
    auto height = Q_star[0]->getHeight();

    parallel_for(nb_threads, [&](int start, int end) {
        for (int i = start; i < end; ++i)
        {
            uint64_t ntuples_i = 0;
            uint64_t ancestor[height];

            parANDCount(nb_threads, i, levelOfCut, Q_star, Q_roots, Q.size(), 0, height - 1, ntuples_i, A.size(), ancestor);

            tuplesMutex.lock();
            ntuples += ntuples_i;
            tuplesMutex.unlock();
        }
    });

    for (uint64_t i = 0; i < Q.size(); i++)
        delete Q_star[i];

    return ntuples;
}
 */

qdag *multiJoin(vector<qdag> &Q, bool bounded_result, uint64_t UPPER_BOUND) {
    qdag::att_set A;
    map<uint64_t, uint8_t> attr_map;


    // computes the union of the attribute sets
    for (uint64_t i = 0; i < Q.size(); i++) {
        uint64_t nAttr = Q[i].nAttr();
        for (uint64_t j = 0; j < nAttr; j++)
            attr_map[Q[i].getAttr(j)] = 1;
    }

    for (map<uint64_t, uint8_t>::iterator it = attr_map.begin(); it != attr_map.end(); it++)
        A.push_back(it->first);

    qdag *Q_star[Q.size()];
    uint64_t Q_roots[Q.size()];

    for (uint64_t i = 0; i < Q.size(); i++) {
        Q_star[i] = Q[i].extend(A);
        Q_roots[i] = 0; // root of every qdag
    }

    vector<uint64_t> bv[Q_star[0]->getHeight()]; // OJO, asume que todos los qdags son de la misma altura
    uint64_t last_pos[Q_star[0]->getHeight()];

    for (uint64_t i = 0; i < Q_star[0]->getHeight(); i++)
        last_pos[i] = 0;

    AND(Q_star, Q_roots, Q.size(), 0, Q_star[0]->getHeight() - 1, bv, last_pos, A.size(), bounded_result, UPPER_BOUND);
    qdag *qResult = new qdag(bv, A, Q_star[0]->getGridSide(), Q_star[0]->getK(), (uint8_t) A.size());
    return qResult;
}

/*
qdag *parMultiJoin(vector<qdag> &Q, bool bounded_result, uint64_t UPPER_BOUND)
{
    qdag::att_set A;
    map<uint64_t, uint8_t> attr_map;

    // computes the union of the attribute sets
    for (uint64_t i = 0; i < Q.size(); i++)
    {
        uint64_t nAttr = Q[i].nAttr();
        for (uint64_t j = 0; j < nAttr; j++)
            attr_map[Q[i].getAttr(j)] = 1;
    }

    for (map<uint64_t, uint8_t>::iterator it = attr_map.begin(); it != attr_map.end(); it++)
        A.push_back(it->first);

    unsigned nb_threads_hint = THREADS_BY_CORE * std::thread::hardware_concurrency();
    unsigned nb_threads = nb_threads_hint == 0 ? 8 : (nb_threads_hint);
    uint16_t levelOfCut = 5;

    std::mutex tuplesMutex;

    qdag *Q_star[Q.size()];
    uint64_t Q_roots[Q.size()];

    for (uint64_t i = 0; i < Q.size(); i++)
    {
        Q_star[i] = Q[i].extend(A);
        if (A.size() == 3)
            Q_star[i]->createTableExtend3();
        else if (A.size() == 4)
            Q_star[i]->createTableExtend4();
        else if (A.size() == 5)
            Q_star[i]->createTableExtend5();
        else
        {
            cout << "Code only works for queries of up to 5 attributes..." << endl;
            exit(1);
        }
        Q_roots[i] = 0; // root of every qdag
    }

    auto height = Q_star[0]->getHeight();
    vector<uint64_t> bv[height]; // OJO, asume que todos los qdags son de la misma altura

    parallel_for(nb_threads, [&](int start, int end) {
        for (int threadId = start; threadId < end; ++threadId)
        {

            uint64_t last_pos[height];
            uint64_t ancestor[height];

            for (uint64_t i = 0; i < height; i++)
                last_pos[i] = 0;

            parAND(nb_threads, threadId, levelOfCut, tuplesMutex, Q_star, Q_roots, Q.size(), 0, height - 1, bv, last_pos, ancestor, A.size(), bounded_result, UPPER_BOUND);
        }
    });

    qdag *qResult = new qdag(bv, A, Q_star[0]->getGridSide(), Q_star[0]->getK(), (uint8_t)A.size());

    return qResult;
}
*/



void semiJoin(vector<qdag> &Q, bool bounded_result, uint64_t UPPER_BOUND) {
    qdag::att_set A;
    map<uint64_t, uint8_t> attr_map;

    if (Q.size() == 1) {
        return;
    }

    // handle empty case
    for (uint64_t i = 0; i < Q.size(); i++) {
        if (Q[i].n_ones() == 0) {
            if (i == 0) {
                return;
            }

            for (int level = 0; level < Q[0].getHeight(); level++) {
                Q[0].Q->active[level] = Q[0].Q->active[level].clone_empty();
            }

            return;
        }
    }

    // computes the union of the attribute sets
    for (uint64_t i = 0; i < Q.size(); i++) {
        //cout << "attrs " << i << ":";
        uint64_t nAttr = Q[i].nAttr();
        for (uint64_t j = 0; j < nAttr; j++) {
            attr_map[Q[i].getAttr(j)] = 1;
            //cout << " " << Q[i].getAttr(j);
        }
        //cout << endl;
    }

    for (map<uint64_t, uint8_t>::iterator it = attr_map.begin(); it != attr_map.end(); it++)
        A.push_back(it->first);

    //cout << "union de attrs: " << A << endl;
    qdag *Q_star[Q.size()];
    uint64_t Q_roots[Q.size()];

    for (uint64_t i = 0; i < Q.size(); i++) {
        Q_star[i] = Q[i].extend(A);
        Q_roots[i] = 0; // root of every qdag
    }

    // para el semijoin bv debe ser igual a active
    uint64_t last_pos[Q[0].getHeight()];
    for (uint64_t i = 0; i < Q[0].getHeight(); i++)
        last_pos[i] = 0;

    // create result_bv of 0s
    vector<rank_bv_64> result_bv(Q[0].getHeight());
    for (int i = 0; i < Q[0].getHeight(); i++) {
        result_bv[i] = Q[0].Q->bv[i].clone_empty();
    }

    SemiAND(Q_star, Q_roots, Q.size(), 0, Q_star[0]->getHeight() - 1, last_pos, A.size(), bounded_result, UPPER_BOUND, result_bv);

    // bajar por result_bv recursivamente, si hay un 1 en el nodo hijo, marcar padre
    propagate_active(Q_star[0], 0, Q_star[0]->getHeight() - 1, result_bv, 0);

    // actualizar bv izquierdo
    for (int i = 0; i < Q[0].getHeight(); i++) {
        Q[0].Q->active[i].bv_and(result_bv[i]);
    }
}
