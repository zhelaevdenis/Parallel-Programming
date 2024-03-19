#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>
#include <windows.h>
//выходной файл для аутпут
const std::string FILE_PATH = "C:\\Users\\91com\\OneDrive\\Desktop\\temp\\output.txt";

//вектор для хранения объектов сделал глобальным для работы в потоках
std::vector<std::vector<char>> segments;

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

    // Предполагается, что размер заголовка BMP составляет 54 байта, а размер изображения — 128x128 для демонстрационных целей.
    /*
     width = 128;
     height = 128;
    */
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
void applyBlurToSegment(std::vector<char>& segment, int width, int height) {
    // Размер размытия поля
    const int blurSize = 128;

    // Коэффициент размытия для нормализации
    const float blurFactor = 10.0f / static_cast<float>((2 * blurSize + 1) * (2 * blurSize + 1));

    // Проверяем, достаточен ли размер сегмента для размытия поля
    if (segment.size() < (2 * blurSize + 1) * (2 * blurSize + 1) * 3) {
        std::cerr << "Segment size is too small for Box Blur." << std::endl;
        return;
    }

    // Временный буфер для данных пикселей с заполнением для окна размытия
    std::vector<char> tempSegment(segment.size() + (2 * blurSize + 1) * (2 * blurSize + 1) * 3);

    // Копируем сегмент во временный буфер с заполнением
    std::copy(segment.begin(), segment.end(), tempSegment.begin() + (blurSize * (2 * blurSize + 1) * 3 + blurSize * 3));

    // Применяем размытие по рамке к каждому пикселю
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
        }
    }
}

bool funcCopyFile(const std::string& sourceFile, const std::string& destinationFile) {
    std::ifstream source(sourceFile, std::ios::binary);
    std::ofstream destination(destinationFile, std::ios::binary);

    if (!source.is_open() || !destination.is_open()) {
        std::cerr << "Error with coping files." << std::endl;
        return false;
    }

    // Копирование файла побайтово
    destination << source.rdbuf();

    source.close();
    destination.close();

    return true;
}

// Функция, которая будет выполняться в отдельном потоке
DWORD WINAPI ThreadFunction(LPVOID param) {
    int i = reinterpret_cast<int>(param);
    // Предполагается, что каждый сегмент имеет одинаковые размеры
    int width = 100; // Замените на реальные размеры сегмента
    int height = 100; // Замените на реальные размеры сегмента
    applyBlurToSegment(segments[i], width, height);
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
    std::string outputFilePath = "output.bmp";

    // получаем количество сегментов
    const int NUM_OF_STRIPS = std::stoi(argv[2]);

    // получаем количество процессоров
    const int NUM_OF_PROCESSORS = std::stoi(argv[3]);

    // Запускаем таймер
    auto start_time = std::chrono::high_resolution_clock::now();

    // Разделяем исходный файл на сегменты и сохраняем в вектор
    int width = 128;
    int height = 128;
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

    // Находим время работы программы
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    while (duration < 500000)
    {
        funcCopyFile(outputFilePath,inputFilePath);
        std::cerr << "again, dur: " << duration << std::endl;
        // Разделяем исходный файл на сегменты и сохраняем в вектор
        int width, height;
        splitFileIntoSegments(inputFilePath, NUM_OF_STRIPS, segments, width, height);

        // Создаем массив хэндлов потоков
        HANDLE* threadHandles = new HANDLE[NUM_OF_STRIPS];
        DWORD* threadIds = new DWORD[NUM_OF_STRIPS];

        // Создаем массив масок процессоров
        DWORD_PTR* processorMasks = new DWORD_PTR[NUM_OF_STRIPS];

        // Заполняем массив масок процессоров
        for (int i = 0; i < NUM_OF_STRIPS; ++i)
        {
            processorMasks[i] = 1 << (i % NUM_OF_PROCESSORS);
        }

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

        // Находим время работы программы
        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        std::cerr << "new dur: " << duration << std::endl;
    }

    //Открываем выходной файл для проверки
    
    std::string openCommand = "start " + outputFilePath; // Команда открытия файла в зависимости от операционной системы
    system(openCommand.c_str());
    

    std::ofstream file(FILE_PATH, std::ios_base::app);
    if (file.is_open())
    {
        file << "threads: " << NUM_OF_STRIPS << " processors: "  << NUM_OF_PROCESSORS << std::endl;
        file << duration << std::endl;
        file << std::endl;
        file.close(); // Закрытие файла
    }

    // Выводим число и время работы программы
    std::cerr << "Time: " << duration << " microseconds" << std::endl;

    return 0;
}
