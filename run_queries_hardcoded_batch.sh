#!/usr/bin/env bash
set -uo pipefail

CXX="${CXX:-g++}"
QUERY_DIR="${QUERY_DIR:-queries_new}"
DATA_DIR="${DATA_DIR:-data3}"
BENCH_OUT="${BENCH_OUT:-results/hardcoded_decompositions.csv}"
FAIL_OUT="${FAIL_OUT:-results/hardcoded_decomposition_failures.csv}"
REPETITIONS="${REPETITIONS:-1}"
TIME_LIMIT="${TIME_LIMIT:-100000000s}"
# time_limit nulo pq sabemos que el mj corre...

CXXFLAGS=(
  -std=c++17
  -O3
  -DNDEBUG
  -Iincludes
  -Iexternal/sdsl-lite/include
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

# shapes con tree decompositions hardcodeadas
declare -A PLAN_COUNT=(
  [j3]=9
  [j4]=7
  [t3]=9
  [t4]=7
  [ti3]=9
  [ti4]=7
)

mkdir -p bin results tmp_queries results/logs

printf 'Compiling queries/query_processor_hardcoded.cpp...\n'
"$CXX" queries/query_processor_hardcoded.cpp "${COMMON_SRC[@]}" \
  "${CXXFLAGS[@]}" "${LDFLAGS[@]}" \
  -o bin/query_processor_hardcoded \
  "${LDLIBS[@]}"

: > "$BENCH_OUT"
echo "shape,query_id,decomposition_id,repetition,reason" > "$FAIL_OUT"

for shape in j3 j4 t3 t4 ti3 ti4; do
  query_list="$QUERY_DIR/$shape.txt"

  if [[ ! -f "$query_list" ]]; then
    echo "Skipping $shape: $query_list does not exist"
    continue
  fi

  echo "========================================"
  echo "Running shape: $shape"
  echo "Query file:    $query_list"
  echo "Plans:         ${PLAN_COUNT[$shape]}"
  echo "========================================"

  query_id=0

  while IFS= read -r query_line || [[ -n "$query_line" ]]; do
    query_line="${query_line%$'\r'}"

    [[ "$query_line" =~ ^[[:space:]]*$ ]] && continue
    [[ "$query_line" =~ ^[[:space:]]*# ]] && continue

    query_id=$((query_id + 1))
    tmp_query="tmp_queries/${shape}_${query_id}.query"
    printf '%s\n' "$query_line" > "$tmp_query"

    echo "[$shape query $query_id]"

    for ((decomposition=1; decomposition<=PLAN_COUNT[$shape]; decomposition++)); do
      for ((rep=1; rep<=REPETITIONS; rep++)); do
        log_file="results/logs/${shape}_${query_id}_d${decomposition}_r${rep}.log"

        echo "  decomposition=$decomposition repetition=$rep/$REPETITIONS"

        timeout "$TIME_LIMIT" \
          ./bin/query_processor_hardcoded \
            --query "$tmp_query" \
            --shape "$shape" \
            --decomposition "$decomposition" \
            --query-id "$query_id" \
            --repetition "$rep" \
            --data-dir "$DATA_DIR" \
            --bench "$BENCH_OUT" \
            > "$log_file" 2>&1

        status=$?
        if [[ $status -ne 0 ]]; then
          if [[ $status -eq 124 ]]; then
            reason="timeout"
          else
            reason="exit_$status"
          fi

          echo "$shape,$query_id,$decomposition,$rep,$reason" >> "$FAIL_OUT"
          echo "    failed: $reason (see $log_file)"
        fi
      done
    done
  done < "$query_list"
done

echo
echo "Done."
echo "Benchmarks: $BENCH_OUT"
echo "Failures:   $FAIL_OUT"
echo "Logs:       results/logs/"