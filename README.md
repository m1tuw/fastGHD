## Generic Query Processor

### Query format

Queries use RDF/SPARQL-style triples:

```text
?x1 1652 ?x2 .
?x2 2043 ?x3 .
?x3 658 ?x4 .
?x4 939 ?x5 .
```

Each triple has the form:

```text
subject predicate object .
```

Predicates are mapped to files as:

```text
1652 -> data/prop-direct-P1652
2043 -> data/prop-direct-P2043
```

Each data file must contain binary tuples:

```text
u v
u v
u v
```

### Compile

From the project root:

```bash
mkdir -p bin

g++ queries/query_processor.cpp \
  src/hypergraphs/hypertree_check.cpp \
  src/fractional_edge_cover/fractional_edge_cover_solver.cpp \
  -std=c++17 \
  -Iincludes \
  -Iexternal/sdsl-lite/include \
  -Lexternal/sdsl-lite/lib \
  -g -O3 \
  -o bin/query_processor \
  -lsdsl -ldivsufsort -ldivsufsort64
```

### Run

Using GHD + Yannakakis:

```bash
./bin/query_processor \
  --query queries/path.query \
  --data-dir data \
  --mode yk \
  --output results/path.tsv \
  --debug \
  --bench results/query_benchmarks.csv
```

Using direct `multiJoin`:

```bash
./bin/query_processor \
  --query queries/path.query \
  --data-dir data \
  --mode mj \
  --output results/path_mj.tsv \
  --debug \
  --bench results/query_benchmarks.csv
```

Benchmark only, without dumping results:

```bash
./bin/query_processor \
  --query queries/path.query \
  --data-dir data \
  --mode yk \
  --no-dump \
  --debug \
  --bench results/query_benchmarks.csv
```

### Options

```text
--query <file>       Query file.
--data-dir <dir>     Directory with relation files. Default: data.
--output <file>      Output TSV file for materialized results.
--mode yk|mj         yk = GHD + Yannakakis, mj = direct multiJoin.
--debug              Print timing and result cardinality.
--no-dump            Do not write result tuples.
--bench <file>       Append benchmark data to CSV.
```
