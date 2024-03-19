#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <ctime>
#include <string>
#include <cstdio> 

const std::string FILE_PATH = "D:\\reposC++\\PP\\lw3.1\\output.txt";

// Синхронизация доступа к файлу
CRITICAL_SECTION g_criticalSection;

// Функция сортировки пузырьком
void bubbleSort(std::vector<int>& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                // Меняем элементы местами
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Функция, которая создает массив и заполняет его случайными числами и сортирует его используя функцию bubbleSort
void SomeFunc()
{
    std::vector<int> array(5000);
    for (int i = 0; i < 5000; ++i)
    {
        array[i] = rand() % 101; // Заполнение случайными числами от 0 до 100
    }
    bubbleSort(array);
}

// Функция, которую выполняет каждый поток
DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
    // Получаем количество операций и номер потока из аргументов
    int threadNumber = *(int*)lpParam;
    int NumOfOperations = *((int*)lpParam + 1);

    // Создаем вектор для хранения времени выполнения каждой операции
    std::vector<double> VecTimeTemp(NumOfOperations);

    // Запуск цикла, выполняющего SomeFunc NumOfOperations раз
    for (int i = 0; i < NumOfOperations; ++i)
    {
        clock_t start = clock();
        SomeFunc();
        clock_t end = clock();
        double elapsed_secs = double(end - start) / CLOCKS_PER_SEC;
        VecTimeTemp[i] = elapsed_secs; // Записываем время выполнения в вектор
    }

    // Захват для синхронизации доступа к файлу
    EnterCriticalSection(&g_criticalSection);

    // Открытие файла в режиме добавления
    std::ofstream file(FILE_PATH, std::ios_base::app);
    if (file.is_open())
    {
        // Запись номера потока и времени работы в файл
        for (int i = 0; i < NumOfOperations; ++i)
        {
            file << threadNumber << " | " << VecTimeTemp[i] << std::endl;
        }
        file << std::endl;
        file.close(); // Закрытие файла
    }

    // Освобождение синхронизации
    LeaveCriticalSection(&g_criticalSection);

    // Освобождение памяти, выделенной для аргументов потока
    delete[](int*)lpParam;

    return 0; // Установка кода завершения потока в 0
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " < >" << std::endl;
        return 1;
    }

    //очищаем файл от предыдущих результатов
    std::ofstream file(FILE_PATH, std::ofstream::trunc); // Открываем файл для записи и очищаем его
    file.close(); // Закрываем файл

    // Ожидание ввода пользователя
    std::cout << "Press Enter to continue..." << std::endl;
    getchar();

    // получаем количество операций
    const int NumOfOperations = std::stoi(argv[1]);

    // Инициализация генератора случайных чисел
    srand(static_cast<unsigned int>(time(nullptr)));

    //задали количество ядер
    const int n = 2;

    // Инициализация синхронизации
    InitializeCriticalSection(&g_criticalSection);

    // Создание массива двух потоков
    HANDLE* handles = new HANDLE[n];

    // Создание двух потоков и передача аргументов
    for (int i = 0; i < n; i++)
    {
        // Создаем массив для аргументов потока
        int* threadArgs = new int[2];
        threadArgs[0] = i; // Номер потока
        threadArgs[1] = NumOfOperations; // Количество операций

        handles[i] = CreateThread(NULL, 0, &ThreadFunction, threadArgs, CREATE_SUSPENDED, NULL);
        if (i == 0) {
            SetThreadPriority(handles[i], THREAD_PRIORITY_ABOVE_NORMAL);
        }
    }

    // Запуск двух потоков
    for (int i = 0; i < n; i++)
    {
        ResumeThread(handles[i]);
    }

    // Ожидание окончания работы двух потоков
    WaitForMultipleObjects(n, handles, true, INFINITE);

    // Удаление синхронизации
    DeleteCriticalSection(&g_criticalSection);

    // Очистка ресурсов
    for (int i = 0; i < n; i++)
    {
        CloseHandle(handles[i]);
    }
    delete[] handles;

    std::cout << "done" << std::endl;

    return 0;
}
/*
D:\reposC++\PP\lw3.1\x64\Debug\lw3.1.exe 21
*/
