#include <windows.h>
#include <iostream>
#include <string>

/*
//путь до утилиты
D:\reposC++\PP\lw5\lw5Utilita\x64\Debug\lw5Utilita.exe
*/


// Функция для преобразования строки в формат LPCWSTR
std::wstring s2ws(const std::string& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

int main(int argc, char* argv[])
{
    // Пути к исполняемому файлу приложения

    /*
    D:\reposC++\PP\lw5\lw5MutexApp\x64\Debug\lw5MutexApp.exe"; //приложение с мьютексом
    D:\reposC++\PP\\lw5\lw5AppCriticalSection\x64\\Debug\lw5AppCriticalSection.exe; //приложение с критикал секшн
    D:\reposC++\PP\\lw5\lw5OrigApp\x64\Debug\lw5OrigApp.exe"; //приложение оригинальное
    */
    std::string app1Path = "D:\\reposC++\\PP\\lw5\\lw5MutexApp\\x64\\Debug\\lw5MutexApp.exe"; //приложение с мьютексом
    //std::string app1Path = "D:\\reposC++\\PP\\lw5\\lw5AppCriticalSection\\x64\\Debug\\lw5AppCriticalSection.exe"; //приложение с критикал секшн
    //std::string app1Path = "D:\\reposC++\\PP\\lw5\\lw5OrigApp\\x64\\Debug\\lw5OrigApp.exe"; //приложение оригинальное


    // Преобразование строк в формат LPCWSTR
    std::wstring wApp1Path = s2ws(app1Path);
    std::wstring wApp2Path = s2ws(app1Path);


    // Запуск первого приложения
    STARTUPINFO si1 = { sizeof(si1) };
    PROCESS_INFORMATION pi1;
    if (!CreateProcess(wApp1Path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si1, &pi1)) {
        std::cerr << "Failed to start the first application." << std::endl;
        return 1;
    }

    // Запуск второго приложения без ожидания первого
    STARTUPINFO si2 = { sizeof(si2) };
    PROCESS_INFORMATION pi2;
    if (!CreateProcess(wApp2Path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si2, &pi2)) {
        std::cerr << "Failed to start the second application." << std::endl;
        // Закрытие первого процесса, если второй не запустится
        CloseHandle(pi1.hProcess);
        CloseHandle(pi1.hThread);
        return 1;
    }

    // Ожидание завершения обоих процессов
    HANDLE handles[2] = { pi1.hProcess, pi2.hProcess };
    WaitForMultipleObjects(2, handles, TRUE, INFINITE);
    // Закрытие дескрипторов процесса и потока
    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);

    return 0;
}