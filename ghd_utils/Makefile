CXX := gcc
CXXFLAGS := -std=c++17 -Wall -Wextra -I../includes -g -O3
LDLIBS := -lstdc++

ifeq ($(OS),Windows_NT)
EXE := .exe
RM := del /Q
else
EXE :=
RM := rm -f
endif

.PHONY: all tests clean hypertree_test fec_test tree_decomp_brute

all: tests tree_decomp_brute

tests: hypertree_test fec_test

hypertree_test: hypertree_test$(EXE)
fec_test: fec_test$(EXE)
tree_decomp_brute: tree_decomp_brute$(EXE)

hypertree_test$(EXE): ../test/hypergraphs/hypertree_check_test.cpp ../src/hypergraphs/hypertree_check.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

fec_test$(EXE): ../test/fractional_edge_cover/fractional_edge_cover_test.cpp ../src/fractional_edge_cover/fractional_edge_cover_solver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

tree_decomp_brute$(EXE): ../src/tree_decomp_brute/tree_decomp_brute.cpp ../src/hypergraphs/hypertree_check.cpp ../src/fractional_edge_cover/fractional_edge_cover_solver.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

clean:
	-$(RM) hypertree_test$(EXE) fec_test$(EXE) tree_decomp_brute$(EXE)