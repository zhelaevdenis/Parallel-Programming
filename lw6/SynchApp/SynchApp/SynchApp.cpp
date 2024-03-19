#include <iostream>
#include <cmath>
#include <iomanip>

double calculatePi(int iterations) {
    double pi = 0.0;
    double sign = 1.0;
    for (int i = 0; i < iterations; ++i) {
        pi += sign / (2.0 * i + 1.0);
        sign *= -1.0;
    }
    return pi * 4.0;
}

int main() {
    const int iterations = 100000000;
    clock_t start = clock();
    double pi = calculatePi(iterations);
    clock_t end = clock();
    double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Pi: " << std::setprecision(15) << pi << std::endl;
    std::cout << "Time: " << elapsed_secs << std::endl;

    return 0;
}