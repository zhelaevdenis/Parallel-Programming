#include <iostream>
#include <iomanip>
#include <omp.h>
#include <ctime>


double calculatePi(int iterations) {
    double tempPi = 0.0;
#pragma omp parallel 
    for (int i = 0; i < iterations; ++i) {
        double sign = (i % 2 == 0) ? 1.0 : -1.0;
        tempPi += sign / (2.0 * i + 1.0);
        //std::cout << "<" << tempPi << std::endl;
    }

    return tempPi * 4.0;
}

int main()
{
    const int iterations = 100000000;
    clock_t start = clock();
    double pi = calculatePi(iterations);
    clock_t end = clock();
    double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Pi: " << std::setprecision(15) << pi << std::endl;
    std::cout << "Time: " << elapsed_secs << std::endl;

    return 0;
}


/*
тест для проверки паралельного запуска
// Функция, которую мы хотим выполнить в цикле
void process(int i) {
    std::cout << "Processing " << i << " on thread " << omp_get_thread_num() << std::endl;
    // Здесь могут быть любые операции, которые вы хотите выполнить в параллельном режиме
}

int main() {
    // Установка количества потоков, которые будут использоваться OpenMP
    omp_set_num_threads(4); // Например, мы устанавливаем 4 потока

    // Запуск цикла с параллельным выполнением функции process
#pragma omp parallel for
    for (int i = 0; i < 10; ++i) {
        process(i);
    }

    return 0;
}

*/