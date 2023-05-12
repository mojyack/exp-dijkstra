import sys
import random

if len(sys.argv) != 2:
    print("usage: python3 gengraph.py vertices")
    exit(1)

size = int(sys.argv[1])
print(f"size {size}")
print("directed false")

a = 0
b = 0
for a in range(0, size):
    for b in range(a + 1,size):
        if random.randrange(2) == 0:
            weight = random.randrange(10) + 1
            print(f"edge {a} {b} {weight}")

