import urllib.request
import time
import sys
import threading

def fetch(url, results, index):
    start = time.time()
    try:
        urllib.request.urlopen(url, timeout=5).read()
        results[index] = time.time() - start
    except Exception as e:
        results[index] = -1

url = sys.argv[1]
concurrency = int(sys.argv[2])
print(f"Testing {url} with concurrency {concurrency}...")

results = [None] * concurrency
threads = []
for i in range(concurrency):
    t = threading.Thread(target=fetch, args=(url, results, i))
    threads.append(t)

start_time = time.time()
for t in threads:
    t.start()
for t in threads:
    t.join()
total_time = time.time() - start_time

valid_results = [r for r in results if r >= 0]
if not valid_results:
    print("All requests failed")
    sys.exit(1)
    
print(f"Total time taken: {total_time:.3f} s")
print(f"Max latency: {max(valid_results)*1000:.1f} ms")
print(f"Min latency: {min(valid_results)*1000:.1f} ms")
print(f"Mean latency: {sum(valid_results)/len(valid_results)*1000:.1f} ms")
print(f"Failed: {len(results) - len(valid_results)}")
