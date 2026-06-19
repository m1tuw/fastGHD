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

echo "Compiling t3_ghd_fast..."
"$CXX" queries/fast_version/t3_ghd_fast.cpp "${COMMON_SRC[@]}" \
  "${CXXFLAGS[@]}" "${LDFLAGS[@]}" \
  -o bin/t3_ghd_fast \
  "${LDLIBS[@]}"

echo "Compiling t3_ghd..."
"$CXX" queries/t3_ghd.cpp "${COMMON_SRC[@]}" \
  "${CXXFLAGS[@]}" "${LDFLAGS[@]}" \
  -o bin/t3_ghd \
  "${LDLIBS[@]}"

FAST_OUT="results/t3_ghd_fast.csv"
NORMAL_OUT="results/t3_ghd.csv"
SPEEDUP_OUT="results/t3_ghd_speedups.csv"

: > "$FAST_OUT"
: > "$NORMAL_OUT"
: > "$SPEEDUP_OUT"

echo "Running datasets from patterns/t3_ghd.txt..."

line_id=0

while read -r R S T REST; do
  [[ -z "${R:-}" ]] && continue
  [[ "${R:0:1}" == "#" ]] && continue

  if [[ -z "${S:-}" || -z "${T:-}" ]]; then
    echo "Skipping malformed line: expected at least 3 relation paths"
    continue
  fi

  line_id=$((line_id + 1))

  echo "processing dataset #$line_id"

  ./bin/t3_ghd_fast "$R" "$S" "$T" time yk "$FAST_OUT" 0
  ./bin/t3_ghd      "$R" "$S" "$T" time yk "$NORMAL_OUT" 1

done < patterns/t3_ghd.txt

echo
echo "Computing speedups..."

python3 "process_results.py" "$FAST_OUT" "$NORMAL_OUT" -o "$SPEEDUP_OUT"

echo
echo "Done."
echo "Fast times:      $FAST_OUT"
echo "Normal times:    $NORMAL_OUT"
echo "Speedups:        $SPEEDUP_OUT"