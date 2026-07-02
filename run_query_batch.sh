#!/usr/bin/env bash
set -euo pipefail

QUERY_DIR="sample_queries"
DATA_DIR="data"
MODE="yk"

mkdir -p results tmp_queries

BENCH_OUT="results/batch_benchmarks.csv"
: > "$BENCH_OUT"

global_query_id=0

for QUERY_LIST in "$QUERY_DIR"/*.txt; do
  [[ -e "$QUERY_LIST" ]] || continue

  query_file_name="$(basename "$QUERY_LIST" .txt)"
  echo "========================================"
  echo "Running queries from: $QUERY_LIST"
  echo "========================================"

  local_query_id=0

  while IFS= read -r query_line; do
    # Skip empty lines and comments.
    [[ -z "${query_line// }" ]] && continue
    [[ "${query_line:0:1}" == "#" ]] && continue

    global_query_id=$((global_query_id + 1))
    local_query_id=$((local_query_id + 1))

    QUERY_FILE="tmp_queries/${query_file_name}_${local_query_id}.query"
    OUT_FILE="results/${query_file_name}_${local_query_id}.tsv"

    echo "$query_line" > "$QUERY_FILE"

    echo
    echo "Running $query_file_name query #$local_query_id"
    echo "$query_line"

    ./bin/query_processor \
      --query "$QUERY_FILE" \
      --data-dir "$DATA_DIR" \
      --mode "$MODE" \
      --no-dump \
      --unit-weights \
      --debug \
      --bench "$BENCH_OUT"

  done < "$QUERY_LIST"
done

echo
echo "Done."
echo "Benchmarks: $BENCH_OUT"
