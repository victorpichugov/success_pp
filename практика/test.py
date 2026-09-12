

def dream(n, x):
    if n <= 1:
        return x
    for i in range(1, n):
        x += dream(i, x)
    return x


def main():
    return dream(4, 2)