from collections import Counter
import sys

def read_pairs(path):
    with open(path) as f:
        for line in f:
            if not line.strip():
                continue
            a, b = map(int, line.split()[:2])
            yield a, b

r1 = Counter()
r2 = Counter()
r3 = Counter()

# ?y 1292 ?x
for y, x in read_pairs(sys.argv[1]):
    r1[x] += 1

# ?z 529 ?x
for z, x in read_pairs(sys.argv[2]):
    r2[x] += 1

# ?x 71 ?v
for x, v in read_pairs(sys.argv[3]):
    r3[x] += 1

common = set(r1) & set(r2) & set(r3)

print("len r1:", len(r1))
print("len r2:", len(r2))
print("len r3:", len(r3))
print("common x:", len(common))
print("cardinality:", sum(r1[x] * r2[x] * r3[x] for x in common))

if common:
    x = next(iter(common))
    print("example x:", x, r1[x], r2[x], r3[x])