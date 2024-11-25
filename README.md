# Inner Product Comparison with AVX-512, and AMX using BF16

This repository demonstrates the use of various hardware optimization techniques for computing the inner product of BF16 vectors using:
1. **Standard CPU instructions**
2. **AVX-512** (Advanced Vector Extensions)
3. **AMX** (Advanced Matrix Extensions)

The code benchmarks the performance of these techniques and provides insights into their speed-ups for computational tasks.

---

## Requirements

### Hardware
- **Processor**: Must support AVX-512 and AMX instruction sets.
- **Operating System**: Linux-based systems with kernel support for AMX extensions.

### Software
- **Compiler**: icpx

---

## Features
- **Data Generation**: Randomly generate `BF16` vectors for testing.
- **Inner Product Implementations**:
  - **Standard Scalar Implementation**: Performs inner product computation in a straightforward way.
  - **AVX-512 Optimized**: Leverages SIMD intrinsics for efficient computation.
  - **AMX Optimized**: Uses tile-based operations for high-performance dot products.

---

## Build And Run

```
make
./out/inner-product
```

### Output
```
Runnging test with dimesion:32, data size: 1600000, test size: 1600
Data generated.
Start test.

Inner product calculations completed in 366.002 seconds.

AVX-512 calculations completed in 20.9063 seconds.

AMX calculations completed in 0.482679 seconds.


Using AVX-512 is 17.5068 times faster than standard.
Using AMX is 758.272 times faster than standard and 43.313 times faster than AVX-512.


```


