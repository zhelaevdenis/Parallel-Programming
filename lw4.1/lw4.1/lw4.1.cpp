#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <ctime>
#include <string>
#include <cstdio>
#include <algorithm>
#include <chrono>

/*
D:\reposC++\PP\lw4.1\x64\Debug\lw4.1.exe D:\reposC++\PP\lw4.1\inputFolder11\input100x100.bmp 3 3
размеры моего bmp 100x100, я занизил чтобы было адекватное количество работы над пикселями. 

Иначе получались зашкаливающие результаты под 21.000 пикселей с временем
*/


const std::string FILE_PATH = "D:\\reposC++\\PP\\lw4.1\\output.txt";
const std::string HELP_MSG = "/?";

//вектор для хранения объектов сделал глобальным для работы в потоках
std::vector<std::vector<char>> segments;

// Глобальная переменная для хранения времени начала работы программы
std::chrono::steady_clock::time_point programStartTime;

//Разделяем исходный файл на сегменты и сохраняет в вектор
void splitFileIntoSegments(const std::string& inputFilePath, int N, std::vector<std::vector<char>>& segments, int& width, int& height) {
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Failed to open the file for reading." << std::endl;
        return;
    }

    // Определяем размер файла
    inputFile.seekg(0, std::ios::end);
    std::streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    // Вычисляем размер каждого сегмента
    std::streampos segmentSize = fileSize / N;
    std::streampos remainder = fileSize % N;

    // Читаем файл и разбиваем его на сегменты
    for (int i = 0; i < N; ++i) {
        std::streampos currentSegmentSize = segmentSize;
        if (i < remainder) {
            currentSegmentSize += 1; // Увеличиваем размер последних сегментов, если размер файла не делится на N
        }

        std::vector<char> segment(currentSegmentSize);
        inputFile.read(segment.data(), currentSegmentSize);
        segments.push_back(std::move(segment));
    }

    inputFile.close();
}

// Функция для объединения сегментов и записи в выходной файл
void mergeSegmentsAndWriteToFile(const std::vector<std::vector<char>>& segments, const std::string& outputFilePath) {
    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Failed to open the file for writing." << std::endl;
        return;
    }

    // Объединяем и записываем сегменты в выходной файл
    for (const auto& segment : segments) {
        outputFile.write(segment.data(), segment.size());
    }

    outputFile.close();
}

// Функция для применения эффекта размытия к сегменту
void applyBlurToSegment(std::vector<char>& segment, int width, int height, int threadId) {
    // Размер размытия поля
    const int blurSize = 10;

    // Коэффициент размытия для нормализации
    const float blurFactor = 1.0f / static_cast<float>((2 * blurSize + 1) * (2 * blurSize + 1));

    // Проверяем, достаточен ли размер сегмента для размытия поля
    if (segment.size() < (2 * blurSize + 1) * (2 * blurSize + 1) * 3) {
        std::cerr << "Segment size is too small for Box Blur." << std::endl;
        return;
    }

    // Временный буфер для данных пикселей с заполнением для окна размытия
    std::vector<char> tempSegment(segment.size() + (2 * blurSize + 1) * (2 * blurSize + 1) * 3);

    // Копируем сегмент во временный буфер с заполнением
    std::copy(segment.begin(), segment.end(), tempSegment.begin() + (blurSize * (2 * blurSize + 1) * 3 + blurSize * 3));

    // Получаем время начала обработки текущего сегмента
    //auto segmentStartTime = std::chrono::steady_clock::now();

    // Применяем размытие по рамке к каждому пикселю
    // Применяем эффект размытия к каждому пикселю
    for (int y = blurSize; y < height - blurSize; ++y) {
        for (int x = blurSize; x < width - blurSize; ++x) {
            int blurR = 0, blurG = 0, blurB = 0;
            for (int dy = -blurSize; dy <= blurSize; ++dy) {
                for (int dx = -blurSize; dx <= blurSize; ++dx) {
                    int index = (y * width + x) * 3 + (dy * width + dx) * 3;
                    blurR += tempSegment[index];
                    blurG += tempSegment[index + 1];
                    blurB += tempSegment[index + 2];
                }
            }
            // Применяем коэффициент размытия и сохраняем результат
            segment[(y * width + x) * 3] = static_cast<char>(blurR * blurFactor);
            segment[(y * width + x) * 3 + 1] = static_cast<char>(blurG * blurFactor);
            segment[(y * width + x) * 3 + 2] = static_cast<char>(blurB * blurFactor);

            // Вычисляем время, затраченное на обработку этого пикселя
            auto pixelEndTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(pixelEndTime - programStartTime).count();
            // Записываем номер потока, время, и координаты пикселя в файл output.txt
            std::ofstream outputFile(FILE_PATH, std::ios::app);
            if (outputFile) {
                outputFile << "Thread " << threadId << ": Pix (" << x << ", " << y << ") time: " << elapsedTime << std::endl;
                outputFile.close();
            }
        }
    }
}

// Функция, которая будет выполняться в отдельном потоке
DWORD WINAPI ThreadFunction(LPVOID param) {
    int i = reinterpret_cast<int>(param);
    //std::cout << "thread " << i << " works" << std::endl;
    // 
    // Предполагается, что каждый сегмент имеет одинаковые размеры
    int width = 25; // Замените на реальные размеры сегмента
    int height = 25; // Замените на реальные размеры сегмента
    applyBlurToSegment(segments[i], width, height, i);
    return 0;
}

int main(int argc, char* argv[])
{
    // Проверяем, что были переданы аргументы
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <bmp_file_path> <integer> <number_of_processors>" << std::endl;
        return 1;
    }

    // получаем входной файл и инициализируем выходной
    std::string inputFilePath = argv[1];
    std::string outputFilePath = "D:\\reposC++\\PP\\lw4.1\\output.bmp";

    // получаем количество сегментов
    const int NUM_OF_STRIPS = std::stoi(argv[2]);

    // получаем количество процессоров
    const int NUM_OF_PROCESSORS = std::stoi(argv[3]);

    //очищаем файл
    std::ofstream file(FILE_PATH, std::ofstream::trunc); // Открываем файл для записи и очищаем его
    file.close(); // Закрываем файл

    

    //создаём вектор приоритетов
    std::vector <int>VecOfPrioriters(NUM_OF_STRIPS);

    std::cout << "Enter proiritet for every thread" << std::endl
        << "THREAD_PRIORITY_BELOW_NORMAL - 0" << std::endl
        << "THREAD_PRIORITY_NORMAL - 1" << std::endl
        << "THREAD_PRIORITY_ABOVE_NORMAL - 2" << std::endl;
    std::string tempInp;

    for (int i = 0; i < NUM_OF_STRIPS; i++)
    {
        std::cout << "Enter proiritet for " << i << " thread" << std::endl;
        std::cin >> tempInp;
        if (tempInp == HELP_MSG)
        {
            std::cout << "Enter proiritet for every thread" << std::endl
                << "THREAD_PRIORITY_BELOW_NORMAL - 0" << std::endl
                << "THREAD_PRIORITY_NORMAL - 1" << std::endl
                << "THREAD_PRIORITY_ABOVE_NORMAL - 2" << std::endl
                << "other ints will count as 1" << std::endl;
            i--;
        }
        else
        {
            try
            {
                VecOfPrioriters[i] = std::stoi(tempInp);
            }
            catch (const std::invalid_argument& e)
            {
                std::cerr << "Error: u put wrong symbol" << std::endl;
                i--;
            }
        }
    }

    //записываем сколько процессоров и потоков
    std::ofstream outputFile(FILE_PATH, std::ios::app);
    if (outputFile) {
        outputFile << "Threads:" << NUM_OF_STRIPS << " Strips: " << NUM_OF_STRIPS << std::endl;
        for (int i = 0; i < NUM_OF_STRIPS; i++)
        {
            outputFile << "Prior for" << i << "process is " << VecOfPrioriters[i] << std::endl;
        }
        outputFile.close();
    }

    // Разделяем исходный файл на сегменты и сохраняем в вектор
    int width = 25;
    int height = 25;
    splitFileIntoSegments(inputFilePath, NUM_OF_STRIPS, segments, width, height);

    // Создаем массив хэндлов потоков
    HANDLE* threadHandles = new HANDLE[NUM_OF_STRIPS];
    DWORD* threadIds = new DWORD[NUM_OF_STRIPS];

    // Создаем массив масок процессоров
    DWORD_PTR* processorMasks = new DWORD_PTR[NUM_OF_STRIPS];

    // Заполняем массив масок процессоров
    for (int i = 0; i < NUM_OF_STRIPS; ++i) {
        processorMasks[i] = 1 << (i % NUM_OF_PROCESSORS);
    }

    

    // Инициализируем время начала работы программы
    programStartTime = std::chrono::steady_clock::now();

    // Запускаем потоки
    for (int i = 0; i < NUM_OF_STRIPS; ++i) {
        threadHandles[i] = CreateThread(NULL, 0, ThreadFunction, reinterpret_cast<LPVOID>(i), 0, &threadIds[i]);
        if (threadHandles[i] == NULL) {
            std::cerr << "Failed to create thread." << std::endl;
            return 1;
        }
        // Устанавливаем маску процессоров для потока
        SetThreadAffinityMask(threadHandles[i], processorMasks[i]);
    }

    //устанавливаем приоритеты
    for (int i = 0; i < NUM_OF_STRIPS; ++i)
    {
        switch (VecOfPrioriters[i])
        {
        case 0:
            if (SetThreadPriority(threadHandles[i], THREAD_PRIORITY_BELOW_NORMAL))
            {
                std::cout << "THREAD_PRIORITY_BELOW_NORMAL done." << std::endl;
            }
            else {
                std::cerr << "THREAD_PRIORITY_BELOW_NORMAL error" << std::endl;
            }
            break;
        case 1:
            if (SetThreadPriority(threadHandles[i], THREAD_PRIORITY_NORMAL))
            {
                std::cout << "THREAD_PRIORITY_NORMAL done." << std::endl;
            }
            else {
                std::cerr << "THREAD_PRIORITY_NORMAL error." << std::endl;
            }
            break;
        case 2:
            if (SetThreadPriority(threadHandles[i], THREAD_PRIORITY_ABOVE_NORMAL))
            {
                std::cout << "THREAD_PRIORITY_ABOVE_NORMAL done." << std::endl;
            }
            else {
                std::cerr << "THREAD_PRIORITY_ABOVE_NORMAL error." << std::endl;
            }
            break;
        default:
            if (SetThreadPriority(threadHandles[i], THREAD_PRIORITY_NORMAL))
            {
                std::cout << "THREAD_PRIORITY_NORMAL DEFAULT done." << std::endl;
            }
            else {
                std::cerr << "THREAD_PRIORITY_NORMAL error." << std::endl;
            }
            break;
        }

    }

    // Ждем завершения всех потоков
    WaitForMultipleObjects(NUM_OF_STRIPS, threadHandles, TRUE, INFINITE);

    // Освобождаем ресурсы
    for (int i = 0; i < NUM_OF_STRIPS; ++i) {
        CloseHandle(threadHandles[i]);
    }

    delete[] threadHandles;
    delete[] threadIds;
    delete[] processorMasks;

    // Объединяем сегменты и записываем в выходной файл
    mergeSegmentsAndWriteToFile(segments, outputFilePath);

    
   /*
    //Открываем входной и выходной файлы для визуальной проверки
    std::string openCommand = "start " + outputFilePath; // Команда открытия файла в зависимости от операционной системы
    system(openCommand.c_str());
    std::string openCommandAdd = "start " + inputFilePath; // Команда открытия файла в зависимости от операционной системы
    system(openCommandAdd.c_str());
   */
    

    std::cerr << "done" << std::endl;

    return 0;
}

