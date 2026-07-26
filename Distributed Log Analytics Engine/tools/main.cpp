#include "log_generator.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // 1. Decide record count — either hardcode for now (e.g. 100) or read from argv[1]
    size_t count = (argc > 1) ? std::atoll(argv[1]) : 100;

    // 2. Open the output file
    std::ofstream out("generated_logs.txt");
    if (!out.is_open()) {
        std::cerr << "failed to open output file\n";
        return 1;
    }

    // 3. Seed one random engine, reused everywhere
    std::mt19937 rng(42);  // fixed seed = reproducible runs, useful while debugging

    // 4. Call your generator
    generate_logs(count, out, rng);

    return 0;
}