#include <algorithm>
#include "qdags.hpp"

void tobinary(unsigned number){
    if (number > 1){
        tobinary(number/2);
    }
    cout << number % 2;
}


bool propagate_active(qdag* Q, uint16_t cur_level, uint16_t max_level, vector<rank_bv_64> temp_bv, uint64_t node){
    bool has_children = false;
    uint32_t children;
    if (cur_level == max_level){
        //caso base: hay un 1 en un nodo hoja de active
        if (temp_bv[cur_level].get_bits(node, Q->getKD()) != 0){
            has_children = true;
        }

    }
    else{

        //caso recursivo: debo revisar si hay 1s en el nivel inferior, hijo por hijo
        uint64_t root_temp;
        uint64_t rank_vector[64];

        uint64_t r = Q->Q->rank(cur_level, node);
        children = Q->Q->get_node(cur_level, node, rank_vector, r);
        uint64_t k_d = Q->getKD();


        // por cuántos hijos voy a bajar, cuenta la cantitdad de 1s en un arreglo de bits/entero
        uint64_t children_to_recurse_size = bits::cnt((uint64_t)children);
        uint64_t i = 0;
        uint64_t msb;
        uint16_t children_to_recurse[512];

        // obtener todos los hijos
        while (i < children_to_recurse_size)
        {
            // obtener el más significativo
            msb = __builtin_clz(children);
            children_to_recurse[i] = 31 - msb;
            ++i;
            children &= (((uint32_t)0xffffffff) >> (msb + 1)); // borra el más significativo
        }

        uint16_t child;

        // bajar por todos los hijos marcados
        for (i = 0; i < children_to_recurse_size; ++i)
        {

            // hijo actual
            child = children_to_recurse[i];

            root_temp = k_d * (rank_vector[child]-1);

            if (propagate_active(Q, cur_level + 1, max_level, temp_bv, root_temp)){
                uint64_t mask_one = 1 << (node + child);
                *(temp_bv[cur_level]).seq = ( *(temp_bv[cur_level]).seq | (mask_one));
                has_children = true;
                }

            }
    }

    return has_children;

}


void ANDCount(qdag *Q[], uint64_t *roots, uint16_t nQ,
              uint16_t cur_level, uint16_t max_level,
              uint64_t &ntuples, uint64_t nAtt)
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
        uint64_t k_d[16 /*nQ*/];       // CUIDADO, solo hasta 16 relaciones por query
        uint64_t root_temp[16 /*nQ*/]; // CUIDADO, solo hasta 16 relaciones por query
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

        while (/*children &&*/ i < children_to_recurse_size)
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

            for (uint64_t j = 0; j < nQ; j++)
                root_temp[j] = k_d[j] * (rank_vector[j][Q[j]->getM(child)] - 1);

            ANDCount(Q, root_temp, nQ, cur_level + 1, max_level, ntuples, nAtt);
        }
    }
}

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
         bool bounded_result, uint64_t UPPER_BOUND)
{
    uint64_t p = Q[0]->nChildren();
    bool result = false;
    //uint64_t root_temp[nQ];
    bool just_zeroes = true;
    uint64_t k_d[16 /*nQ*/]; //CUIDADO, solo hasta 16 relaciones por query

    uint16_t children_to_recurse[512 /*p*/]; // CUIDADO, solo hasta 9 atributos distintos por query

    uint64_t i;
    uint64_t children_to_recurse_size = 0;

    uint32_t children = 0xffffffff; // puros 1s

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



        // todos los hijos son resultados
        while (/*children &&*/ i < children_to_recurse_size)
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
                bv[cur_level].push_back(last_pos[cur_level]++);
                just_zeroes = false;
            }
        }

        if (p - last_child > 1)
            last_pos[cur_level] += (p - last_child - 1);
    }
    else
    {
        uint64_t root_temp[16 /*nQ*/]; // CUIDADO, solo hasta 16 relaciones por query
        uint64_t rank_vector[16][64];

        // pide el nodo actual de cada qdag (devuelve entero de 32 bits) y hace AND con children
        // sobreviven solo las ramas que tienen hijos en cada qdag
        for (i = 0; i < nQ && children; ++i)
        {
            k_d[i] = Q[i]->getKD();
            if (nAtt == 3) {
                children &= Q[i]->materialize_node_3(cur_level, roots[i], rank_vector[i]);
            } else if (nAtt == 4)
                children &= Q[i]->materialize_node_4(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5(cur_level, roots[i], rank_vector[i]);
        }

        // por cuántos hijos voy a bajar, cuenta la cantitdad de 1s en un arreglo de bits/entero
        children_to_recurse_size = bits::cnt((uint64_t)children);
        i = 0;
        uint64_t msb;

        // obtener todos los hijos
        while (/*children &&*/ i < children_to_recurse_size)
        {
            // obtener el más significativo
            msb = __builtin_clz(children);
            children_to_recurse[i] = msb; // baja por el más significativo
            ++i;
            children &= (((uint32_t)0xffffffff) >> (msb + 1)); // borra el más significativo
        }

        int64_t last_child = -1;
        uint16_t child;

        // bajar por todos los hijos marcados
        for (i = 0; i < children_to_recurse_size; ++i)
        {

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
            else if (cur_level == max_level || AND(Q, root_temp, nQ, cur_level + 1, max_level, bv, last_pos, nAtt, bounded_result, UPPER_BOUND))
            {
                //si se llega al último nivel o si hay resultados en el subárbol, se pone un 1 en la posición para indicar que hay resultados
                bv[cur_level].push_back(last_pos[cur_level]++);
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
             bool bounded_result, uint64_t UPPER_BOUND, vector<rank_bv_64> temp_bv)
{
    uint64_t p = Q[0]->nChildren();
    bool result = false;
    //uint64_t root_temp[nQ];
    bool just_zeroes = true;
    uint64_t k_d[16 /*nQ*/]; //CUIDADO, solo hasta 16 relaciones por query

    uint16_t children_to_recurse[512 /*p*/]; // CUIDADO, solo hasta 9 atributos distintos por query

    uint64_t i;
    uint64_t children_to_recurse_size = 0;

    uint32_t children = 0xffffffff; // puros 1s

    if (cur_level == max_level)
    {
        for (i = 0; i < nQ && children; ++i)
        {
            if (nAtt == 3){
                children &= Q[i]->materialize_node_3_lastlevel(cur_level, roots[i]);//entero representando el nodo en el qdag, al hacer and eliminamos bits de children
            }
            else if (nAtt == 4)
                children &= Q[i]->materialize_node_4_lastlevel(cur_level, roots[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5_lastlevel(cur_level, roots[i]);
        }
        // cout << cur_level << "    " << std::bitset<32>(children).to_string()<< endl;
        children_to_recurse_size = bits::cnt((uint64_t)children); //cuantos 1 hay en children
        i = 0;
        uint64_t msb;


        // todos los hijos son resultados
        while (/*children &&*/ i < children_to_recurse_size)
        {
            //Queremos obtener las posiciones correspondientes en Q[0] de los hijos, quitarle la dimensión del extend
            msb = __builtin_clz(children);//count leading 0s
            children_to_recurse[i] = msb;//almacena pos de bit mas significativo de children
            ++i;
            children &= (((uint32_t)0xffffffff) >> (msb + 1)); //shift para obtener el siguiente
        }//obtenemos las posiciones de todos los 1 en children en tiempo proporcional a la cantidad de 1s

        int64_t last_child = -1;
        uint16_t child;
        uint16_t child_left_Q;

        for (i = 0; i < children_to_recurse_size; ++i)
        {
            child = children_to_recurse[i];
            if (child - last_child > 1) {
                last_pos[cur_level] += (child - last_child - 1);
                // si la diferencia es mayor a 1, se debe agregar. Se resta 1 porque al hacer pushback se hace =
            }


            last_child = child;
            if (bounded_result)// && active_s[max_level].size() >= UPPER_BOUND) TODO: fix bounded result
                return false;
            else
            {

                // obtener el bit del nodo
                uint64_t mask_one =1;
                *(temp_bv[cur_level].seq) = (*(temp_bv[cur_level]).seq | (mask_one << (roots[0] + Q[0]->getM(last_pos[cur_level] % p))));
                last_pos[cur_level]++;
                just_zeroes = false;
            }
        }

         if  (p - last_child > 1) { // mover puntero al inicio del siguiente nodo
             last_pos[cur_level] += (p - last_child - 1);
         }//(p_left - last_child - 1); //
    }
    else
    {
        // arreglo con raices que serán usadas en llamda recursiva
        uint64_t root_temp[16 /*nQ*/]; // CUIDADO, solo hasta 16 relaciones por query
        uint64_t rank_vector[16][64];

        // pide el nodo actual de cada qdag (devuelve entero de 32 bits) y hace AND con children
        // sobreviven solo las ramas que tienen hijos en cada qdag

        for (i = 0; i < nQ && children; ++i)
        {
            k_d[i] = Q[i]->getKD();
            if (nAtt == 3){
                children &= Q[i]->materialize_node_3(cur_level, roots[i], rank_vector[i]);
            } else if (nAtt == 4)
                children &= Q[i]->materialize_node_4(cur_level, roots[i], rank_vector[i]);
            else if (nAtt == 5)
                children &= Q[i]->materialize_node_5(cur_level, roots[i], rank_vector[i]);
        }
        //cout << cur_level << " pre pruning  " << bits::cnt((uint64_t)children) << endl;
        // Stop the descent through branches that are marked in active
        if (nAtt == 3) {
            children &= Q[0]->materialize_active_node_3(cur_level, roots[0], temp_bv);
        } else if (nAtt == 4)
            children &= Q[0]->materialize_active_node_4(cur_level, roots[0], temp_bv);
        else if (nAtt == 5)
            children &= Q[0]->materialize_active_node_5(cur_level, roots[0], temp_bv);
        //cout << "post pruning  " << bits::cnt((uint64_t)children) << endl;

        // por cuántos hijos voy a bajar, cuenta la cantitdad de 1s en un arreglo de bits/entero
        children_to_recurse_size = bits::cnt((uint64_t)children);
        i = 0;
        uint64_t msb;

        // obtener todos los hijos
        while (/*children &&*/ i < children_to_recurse_size)
        {
            // obtener el más significativo
            msb = __builtin_clz(children);
            children_to_recurse[i] = msb;
            ++i;
            children &= (((uint32_t)0xffffffff) >> (msb + 1)); // borra el más significativo
        }

        int64_t last_child = -1;
        uint16_t child;

        // bajar por todos los hijos marcados
        for (i = 0; i < children_to_recurse_size; ++i)
        {

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
            else if (cur_level == max_level || SemiAND(Q, root_temp, nQ, cur_level + 1, max_level, last_pos, nAtt, bounded_result, UPPER_BOUND, temp_bv))
            {
                // si se llega al último nivel o si hay resultados en el subárbol, se pone un 1 en la posición para indicar que hay resultados
                // check if my children are marked, if they are, mark me.
                uint64_t temp_children = temp_bv[cur_level+1].get_bits(root_temp[0], Q[0]->Q->getKD());
                uint64_t leftQ_children = Q[0]->Q->bv[cur_level+1].get_bits(root_temp[0], Q[0]->Q->getKD());

                if (temp_children == leftQ_children){
                    uint64_t mask_one =1;
                    //cout << "mark active" << endl;
                    *(temp_bv[cur_level]).seq = ( *(temp_bv[cur_level]).seq | (mask_one << (roots[0] + Q[0]->getM(last_pos[cur_level] % p))));
                }
                last_pos[cur_level]++;

                just_zeroes = false;
            }
            else
            {
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


uint64_t multiJoinCount(vector<qdag> &Q)
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
    ANDCount(Q_star, Q_roots, Q.size(), 0, Q_star[0]->getHeight() - 1, ntuples, A.size());

    for (uint64_t i = 0; i < Q.size(); i++)
        delete Q_star[i];

    return ntuples;
}

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

qdag *multiJoin(vector<qdag> &Q, bool bounded_result, uint64_t UPPER_BOUND)
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

    vector<uint64_t> bv[Q_star[0]->getHeight()]; // OJO, asume que todos los qdags son de la misma altura
    uint64_t last_pos[Q_star[0]->getHeight()];

    for (uint64_t i = 0; i < Q_star[0]->getHeight(); i++)
        last_pos[i] = 0;

    AND(Q_star, Q_roots, Q.size(), 0, Q_star[0]->getHeight() - 1, bv, last_pos, A.size(), bounded_result, UPPER_BOUND);

    qdag *qResult = new qdag(bv, A, Q_star[0]->getGridSide(), Q_star[0]->getK(), (uint8_t)A.size());
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


void semiJoin(vector<qdag> &Q, bool bounded_result, uint64_t UPPER_BOUND)
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


    // para el semijoin bv debe ser igual a active
    vector<uint64_t> bv[Q[0].getHeight()]; // OJO, asume que todos los qdags son de la misma altura
    uint64_t last_pos[Q[0].getHeight()];

    for (uint64_t i = 0; i < Q[0].getHeight(); i++)
        last_pos[i] = 0;

    // create temp active of 0s
    bit_vector blank = bit_vector(4, 0);
    vector<rank_bv_64> temp(Q[0].getHeight());
    for (int i = 0; i < Q[0].getHeight(); i++ ) {
        temp[i] = rank_bv_64(blank);
    }

    SemiAND(Q_star, Q_roots, Q.size(), 0, Q_star[0]->getHeight() - 1, last_pos, A.size(), bounded_result, UPPER_BOUND, temp);

    //bajar por temp recursivamente, si hay un 1 en el nodo hijo, marcar padre
    propagate_active(Q_star[0], 1, Q_star[0]->getHeight() - 1, temp, 0);

    // actualizar bv izquierdo
    for (int i = 1; i<Q[0].getHeight(); i++){
        Q[0].Q->active[i].bv_and(temp[i]);
    }

}
