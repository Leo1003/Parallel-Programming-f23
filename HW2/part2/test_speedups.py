from decimal import Decimal
import subprocess
import matplotlib.pyplot as plt

def test_speedups_thread(threads: int, iterations: int = 5) -> Decimal:
    speedups = Decimal(0.0)

    for i in range(iterations):
        p = subprocess.Popen(["./mandelbrot", "-v", "2", "-t", str(threads)], stdout=subprocess.PIPE, close_fds=True, encoding="utf-8")
        retcode = p.wait()        

        if retcode == 0:
            output = p.stdout.readline()
            speedup = Decimal(output)
            speedups += speedup

    return round(speedups / iterations, 3)

def test_speedups(thread_list: list[int]) -> list[Decimal]:
    results = list()

    for t in thread_list:
        speedup = test_speedups_thread(t)
        results.append(speedup)
        
    return results

threads = list(range(2, 33))
speedups = test_speedups(threads)

print(f"Speedup results: {speedups}")

# Build the line graph.
plt.plot(threads, speedups, marker='o')

# Display the line graph.
plt.show()
