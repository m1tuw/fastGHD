#!/usr/bin/env bash
set -euo pipefail

CXX="g++"

CXXFLAGS=(
  -std=c++17
  -Iincludes
  -Iexternal/sdsl-lite/include
  -g
  -O3
)

LDFLAGS=(
  -Lexternal/sdsl-lite/lib
)

LDLIBS=(
  -lsdsl
  -ldivsufsort
  -ldivsufsort64
)

COMMON_SRC=(
  src/hypergraphs/hypertree_check.cpp
  src/fractional_edge_cover/fractional_edge_cover_solver.cpp
)

mkdir -p bin results

echo "Compiling square_barbell_fast..."
"$CXX" queries/fast_version/square_barbell_fast.cpp "${COMMON_SRC[@]}" \
  "${CXXFLAGS[@]}" "${LDFLAGS[@]}" \
  -o bin/square_barbell_fast \
  "${LDLIBS[@]}"

echo "Compiling square_barbell..."
"$CXX" queries/square_barbell.cpp "${COMMON_SRC[@]}" \
  "${CXXFLAGS[@]}" "${LDFLAGS[@]}" \
  -o bin/square_barbell \
  "${LDLIBS[@]}"

FAST_OUT="results/square_barbell_fast.csv"
NORMAL_OUT="results/square_barbell.csv"
SPEEDUP_OUT="results/square_barbell_speedups.csv"

: > "$FAST_OUT"
: > "$NORMAL_OUT"

echo "Running datasets from patterns/square_barbell.txt..."

line_id=0

while read -r A B C D J R S T U; do
  [[ -z "${A:-}" ]] && continue
  [[ "${A:0:1}" == "#" ]] && continue

  line_id=$((line_id + 1))

  echo "processing dataset #$line_id"

  ./bin/square_barbell_fast "$A" "$B" "$C" "$D" "$J" "$R" "$S" "$T" "$U" time yk "$FAST_OUT" 0
  ./bin/square_barbell "$A" "$B" "$C" "$D" "$J" "$R" "$S" "$T" "$U" time yk "$NORMAL_OUT" 0

done < patterns/square_barbell.txt

echo
echo "Computing speedups..."

python3 "process_results.py" "$FAST_OUT" "$NORMAL_OUT" -o "$SPEEDUP_OUT"

echo
echo "Done."
echo "Fast times:      $FAST_OUT"
echo "Normal times:    $NORMAL_OUT"
echo "Speedups:        $SPEEDUP_OUT"