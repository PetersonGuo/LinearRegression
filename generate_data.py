import random

NUM_POINTS = 100

with open("testdata.txt", mode="w", encoding="UTF-16") as f:
    for _ in range(NUM_POINTS):
        x = random.uniform(-10, 10)
        y = 2 * x + 3 + random.gauss(0, 1)  # Linear relation with some noise
        f.write(f"{x} {y}\n")

