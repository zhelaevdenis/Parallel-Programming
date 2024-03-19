#include <iostream>
#include <iomanip>
#include <omp.h>

int main() {
    const int N = 100000000; // Количество итераций для вычисления числа PI
    double pi = 0.0;
    double sign = 1.0;
    clock_t start = clock();
    #pragma omp parallel
    {
        double local_pi = 0.0; // Локальная переменная для каждого потока

        #pragma omp for
        for (int i = 0; i < N; ++i) {
            sign = (i % 2 == 0) ? 1.0 : -1.0;
            local_pi += sign / (2.0 * i + 1);
        }

        // Синхронизированное обновление общей переменной pi
        #pragma omp atomic
        pi += local_pi;
    }
    pi *= 4.0;
    clock_t end = clock();

    double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Pi: " << std::setprecision(15) << pi << std::endl;
    std::cout << "Time: " << elapsed_secs << std::endl;

    return 0;
}