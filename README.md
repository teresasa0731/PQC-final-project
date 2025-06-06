# PQC-final-project
Benchmark large integer multiplication on the Raspberry Pi and compare with mpz_mul provided by GMP.

## Brenchmark
using brenchmark tools [aarch64-bench](https://github.com/mkannwischer/aarch64-bench.git)

### How to build the test
```bash
git clone https://github.com/mkannwischer/aarch64-bench.git
cd aarch64-bench/
make CYCLES=PERF
sudo ./bench
```
and the terminal will show CPU cycle count as follows:
```bash
$ ./bench 
 
       ntt cycles = 992
 
           percentile      1     10     20     30     40     50     60     70     80     90     99
 
       ntt percentiles:    989    992    992    992    992    992    992    992   1017   1022   1202
```
