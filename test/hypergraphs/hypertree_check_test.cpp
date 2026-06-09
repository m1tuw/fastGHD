#include "project/hypergraphs/hypertree_check.hpp"

#include <bits/stdc++.h>
#include <iostream>

void triangular_hypergraph(){
    std::vector<std::vector<int>> bags = {
        {0, 1},
        {1, 2},
        {2, 0}
    };
    bool ans = hypergraph::is_hypertree(bags, 3);
    int expected = 0;
    std::cout << "Triangular hypergraph: ";
    if(ans == expected){
        std::cout << "Accepted" << '\n';
    }
    else{
        std::cout << "Wrong Answer" << '\n';
        std::cout << "Expected " << expected << ", got " << ans << " instead." << '\n';
    }
}

void wikipedia_example(){
    std::vector<std::vector<int>> bags = {
    {0, 1, 2},
    {1, 2, 4},
    {2, 3, 4},
    {1, 4, 6},
    {1, 5, 6},
    {4, 6, 7}
    };
    bool ans = hypergraph::is_hypertree(bags, 8);
    int expected = 1;
    std::cout << "Wikipedia hypergraph: ";
    if(ans == expected){
        std::cout << "Accepted" << '\n';
    }
    else{
        std::cout << "Wrong Answer" << '\n';
        std::cout << "Expected " << expected << ", got " << ans << " instead." << '\n';
    }
}

void disconnected_hypergraph(){
    std::vector<std::vector<int>> bags = {
        {0, 1, 2},
        {1, 2, 3},
        {4, 5, 6},
        {5, 6, 7}
    };
    bool ans = hypergraph::is_hypertree(bags, 8);
    int expected = 1;
    std::cout << "Disconnected hypergraph: ";
    if(ans == expected){
        std::cout << "Accepted" << '\n';
    }
    else{
        std::cout << "Wrong Answer" << '\n';
        std::cout << "Expected " << expected << ", got " << ans << " instead." << '\n';
    }
}

void cyclic_hypergraph(){
    std::vector<std::vector<int>> bags = {
        {0, 1, 2},
        {2, 3, 4},
        {4, 5, 0}
    };
    bool ans = hypergraph::is_hypertree(bags, 6);
    int expected = 0;
    std::cout << "Alpha-cyclic hypergraph: ";
    if(ans == expected){
        std::cout << "Accepted" << '\n';
    }
    else{
        std::cout << "Wrong Answer" << '\n';
        std::cout << "Expected " << expected << ", got " << ans << " instead." << '\n';
    }
}

void alpha_acyclic_hypergraph(){
    std::vector<std::vector<int>> bags = {
        {0, 1, 2},
        {2, 3, 4},
        {4, 5}
    };
    bool ans = hypergraph::is_hypertree(bags, 6);
    int expected = 1;
    std::cout << "Alpha-acyclic hypergraph: ";
    if(ans == expected){
        std::cout << "Accepted" << '\n';
    }
    else{
        std::cout << "Wrong Answer" << '\n';
        std::cout << "Expected " << expected << ", got " << ans << " instead." << '\n';
    }
}



int main(){
    triangular_hypergraph();
    wikipedia_example();
    disconnected_hypergraph();
    cyclic_hypergraph();
    alpha_acyclic_hypergraph();
    return 0;
}