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

echo "Compiling bowtie_fast..."
"$CXX" queries/fast_version/bowtie_fast.cpp "${COMMON_SRC[@]}" \
  "${CXXFLAGS[@]}" "${LDFLAGS[@]}" \
  -o bin/bowtie_fast \
  "${LDLIBS[@]}"

echo "Compiling bowtie..."
"$CXX" queries/bowtie.cpp "${COMMON_SRC[@]}" \
  "${CXXFLAGS[@]}" "${LDFLAGS[@]}" \
  -o bin/bowtie \
  "${LDLIBS[@]}"

FAST_OUT="results/bowtie_fast.csv"
NORMAL_OUT="results/bowtie.csv"
SPEEDUP_OUT="results/bowtie_speedups.csv"

: > "$FAST_OUT"
: > "$NORMAL_OUT"

echo "Running datasets from patterns/bowtie.txt..."

line_id=0

while read -r R S T RP SP TP; do
  [[ -z "${R:-}" ]] && continue
  [[ "${R:0:1}" == "#" ]] && continue

  line_id=$((line_id + 1))

  echo "processing dataset #$line_id"

  ./bin/bowtie_fast "$R" "$S" "$T" "$RP" "$SP" "$TP" time yk "$FAST_OUT" 0
  ./bin/bowtie "$R" "$S" "$T" "$RP" "$SP" "$TP" time yk "$NORMAL_OUT" 0

done < patterns/bowtie.txt

echo
echo "Computing speedups..."

python3 "process_results.py" "$FAST_OUT" "$NORMAL_OUT" -o "$SPEEDUP_OUT"

echo
echo "Done."
echo "Fast times:      $FAST_OUT"
echo "Normal times:    $NORMAL_OUT"
echo "Speedups:        $SPEEDUP_OUT"