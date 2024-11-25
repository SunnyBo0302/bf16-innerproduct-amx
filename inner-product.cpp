#include <iostream>
#include <random>
#include <chrono>
#include <immintrin.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>

using BF16 = __bf16;

#define TEST_SIZE (16 * 100)
#define DATA_SIZE (16 * 10000)
#define DIMENSION 32

#define ARCH_GET_XCOMP_PERM 0x1022
#define ARCH_REQ_XCOMP_PERM 0x1023
#define XFEATURE_XTILECFG 17
#define XFEATURE_XTILEDATA 18

// Define tile config data structure
typedef struct __tile_config {
    uint8_t palette_id;
    uint8_t start_row;
    uint8_t reserved_0[14];
    uint16_t colsb[16];
    uint8_t rows[16];
} __tilecfg;

bool enable_amx() {
    if (syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA)) {
        std::cerr << "Failed to enable AMX" << std::endl;
        return false;
    }

    __tilecfg config = {0}; {
        config.palette_id = 1;
        config.start_row = 0;

        config.colsb[0] = 16 * 4;
        config.rows[0] = 16;

        config.colsb[1] = 4 * 16;
        config.rows[1] = 16;

        config.colsb[2] = 4 * 16;
        config.rows[2] = 16;
    }
    _tile_loadconfig(&config);

    return true;
}


// Generate random BF16 numbers in [0, 1]
void generate_random_data(BF16 *array, size_t rows, size_t cols) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution dist(0.0, 1.0);

    for (size_t i = 0; i < rows * cols; ++i) {
        float value = dist(gen);
        array[i] = (BF16) value;
    }
}

// Compute inner product 1 pair of 32-dimensional vectors
BF16 result = 0;

void inner_product(const BF16 *a, const BF16 *b) {
    for (int i = 0; i < DIMENSION; ++i) {
        result += a[i] * b[i];
    }
}

// Compute inner product of 1 pairs of 32-dimensional vectors using avx-512
void inner_product_avx512(const BF16 *a, const BF16 *b) {
    __m512 sum = _mm512_setzero_ps();

    __m512 v1 = _mm512_loadu_ps(a);
    __m512 v2 = _mm512_loadu_ps(b);

    sum = _mm512_dpbf16_ps(v1, v2, sum);

    result = _mm512_reduce_add_ps(sum);
}

// Compute inner product of 16 pairs 32-dimensional vectors using amx
float results[1024 / 4] = {};

void inner_product_amx(const BF16 *a, const BF16 *b) {
    _tile_loadd(1, a, 16 * 4);
    _tile_loadd(2, b, 16 * 4);
    _tile_loadd(0, results, 16 * 4);

    // Compute dot-product of bytes in tiles
    _tile_dpbf16ps(0, 1, 2);

    // Store the tile data to memory
    _tile_stored(0, results, 16 * 4);
}

double run_test(BF16 *test, BF16 *data) {
    auto start = std::chrono::high_resolution_clock::now();

    // Compute inner products
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        for (size_t j = 0; j < DATA_SIZE; ++j) {
            inner_product(test + i * DIMENSION, data + j * DIMENSION);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Inner product calculations completed in " << elapsed.count() << " seconds.\n\n";

    return elapsed.count();
}

double run_test_avx512(BF16 *test, BF16 *data) {
    auto start = std::chrono::high_resolution_clock::now();

    // Compute inner products
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        for (size_t j = 0; j < DATA_SIZE; ++j) {
            inner_product_avx512(test + i * DIMENSION, data + j * DIMENSION);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "AVX-512 calculations completed in " << elapsed.count() << " seconds.\n\n";

    return elapsed.count();
}

double run_test_amx(BF16 *test, BF16 *data) {
    auto start = std::chrono::high_resolution_clock::now();

    // Compute inner products
    for (size_t i = 0; i < TEST_SIZE; i += 16) {
        for (size_t j = 0; j < DATA_SIZE; j += 16) {
            inner_product_amx(test + i * DIMENSION, data + j * DIMENSION);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "AMX calculations completed in " << elapsed.count() << " seconds.\n\n";

    return elapsed.count();
}


int main() {
    // Setup for amx
    if (!enable_amx()) exit(-1);

    // Allocate memory for test and data

    std::cout << "Runnging test with dimesion:" << DIMENSION
            << ", data size: " << DATA_SIZE
            << ", test size: " << TEST_SIZE
            << std::endl;

    BF16 *test = (BF16 *) malloc(TEST_SIZE * DIMENSION * sizeof(BF16));
    BF16 *data = (BF16 *) malloc(DATA_SIZE * DIMENSION * sizeof(BF16));

    // Generate random data
    generate_random_data(test, TEST_SIZE, DIMENSION);
    generate_random_data(data, DATA_SIZE, DIMENSION);
    std::cout << "Data generated.\n";

    // Run tests for standard, avx-512, amx
    std::cout << "Start test.\n\n";
    double standard = run_test(test, data);
    double avx512 = run_test_avx512(test, data);
    double amx = run_test_amx(test, data);

    std::cout << std::endl << "Using AVX-512 is " << (standard / avx512) << " times faster than standard." << std::endl;

    std::cout << "Using AMX is " << (standard / amx) << " times faster than standard and "
            << (avx512 / amx) << " times faster than AVX-512." << std::endl;

    return 0;
}
