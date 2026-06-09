//
// Created by anouk on 08-08-23.
//

#ifndef CCQ_QDAGS_GHD_HPP
#define CCQ_QDAGS_GHD_HPP

#include <inttypes.h>
#include <vector>
#include "qdags.hpp"
#include "../src/joins_old.cpp"
#include <optional>

using namespace std;

class ghd {
    vector<qdag> relations;
    vector<ghd> children;

public:

    ghd() = default;

    ghd(vector<qdag> qdags, vector<ghd> subtrees) {
        relations = qdags;
        children = subtrees;
    }

    vector<qdag> get_relations(){
        return relations;
    }

    vector<ghd> get_children(){
        return children;
    }

    void collect_all_nodes(vector<ghd*> &subtree){

        subtree.push_back(this);

        // Recursively collect nodes from children
        for (auto child = children.begin(); child != children.end(); child++) {
            child->collect_all_nodes(subtree);
        }
    }

    vector<qdag> get_child_qdags(){
        // This will be used during semijoin, so there will only be 1 qdag per vector
        // obtengo el primer qdag que guarda cada uno de mis hijos en su nodo
        vector<qdag> results;
        for (auto child = children.begin(); child != children.end(); child++){
            results.push_back(child->get_relations().front());
        }
        return results;
    }

    void get_subtree_qdags(vector<qdag> &subtree){

        subtree.push_back(relations.front());
        for (auto child = children.begin(); child != children.end(); child++){

            child->get_subtree_qdags(subtree);
        }
    }

    void set_relations(vector<qdag> new_relations){
        relations = new_relations;
    }

    void exec_multijoin(){
        // ejecuta multijoin entre las relaciones del nodo y reemplaza el vector de relaciones
        if (relations.size() == 1) {
            return;
        }
        qdag* join_result = multiJoin(relations, false, 1000);
        relations.clear();
        relations.push_back(*join_result);
        relations.shrink_to_fit();
    }

    void deep_exec_multijoin(){
        exec_multijoin();
        for (auto child = children.begin(); child != children.end(); child++){
            child->deep_exec_multijoin();
        }
    }


    void constrained_by_children(){
        // si soy hoja empiezo a subir
        if (children.empty()){
            return;
        }
        else{
            // bajo por el árbol
            for (auto child = children.begin(); child != children.end(); child++){
                child->constrained_by_children();
            }
            // semijoin entre nodo y sus hijos. Debo pasarle un vector en el cual el primer elemento sea
            // mi qdag, y el resto son los qdag de children
            // esto debería alterar mi qdag
            vector<qdag> rels = get_child_qdags();
            rels.insert(rels.begin(), relations.front());
            semiJoin(rels, false, 1000);
        }
    }

    // constrain children
    //iterar sobre hijos y llamar semijoin entre hijo_i y nodo
    void constrain_children(){
        // si soy hoja termino
        if (children.empty()){
            return;
        }
        else {
            vector<qdag> pair(2);
            pair[1] = relations.front();
            for (auto child = children.begin(); child != children.end(); child++) {
                // obtengo el qdag guardado en el hijo
                pair[0] = child->get_relations().front();
                // retrinjo al hijo
                semiJoin(pair, false, 1000);
                // bajo por el árbol
                child->constrain_children();
            }
        }

    }

    uint64_t size() {
        uint64_t total = 0;
        for (auto qdag = relations.begin(); qdag != relations.end(); qdag++){
            total += qdag->size();
        }
        for (auto child = children.begin(); child != children.end(); child++){
            total += child->size();
        }
        return total;
    }
    
    void print_n_ones(std::optional<std::reference_wrapper<std::ofstream>> outfile)
    {
        outfile->get() << n_ones();
        for (auto child = children.begin(); child != children.end(); child++) {
            outfile->get() << "-";
            child->print_n_ones(outfile);
        }
    }

    void print_all_ones(std::optional<std::reference_wrapper<std::ofstream>> outfile)
    {
        int n_tuples = 0;
        for (auto rel = relations.begin(); rel != relations.end(); rel++) {
            n_tuples += rel->n_ones();
        }
        outfile->get() << n_tuples;
        for (auto child = children.begin(); child != children.end(); child++) {
            outfile->get() << "-";
            child->print_all_ones(outfile);
        }
    }

    uint64_t n_ones()
    {
        return relations.front().n_ones();
    }

    /*
     *  creates a GHD given a set of bags of vertices
     */
    ghd get_optimal_ghd(int number_of_nodes, int number_of_edges, const vector<pair<int,int>>& edges) {

    }
};


#endif //CCQ_QDAGS_GHD_HPP
