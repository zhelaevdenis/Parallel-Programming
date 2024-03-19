#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>
#include <chrono>

HANDLE hMutex;
const std::string FILE_PARH_OUTPUT = "output.txt";

int ReadFromFile() {
    WaitForSingleObject(hMutex, INFINITE);
    std::fstream myfile("balance.txt", std::ios_base::in);
    int result;
    myfile >> result;
    myfile.close();
    ReleaseMutex(hMutex);

    return result;
}

void WriteToFile(int data) {
    WaitForSingleObject(hMutex, INFINITE);
    std::fstream myfile("balance.txt", std::ios_base::out);
    myfile << data << std::endl;
    myfile.close();
    ReleaseMutex(hMutex);
}

void Deposit(int money) {
    WaitForSingleObject(hMutex, INFINITE);
    int balance = ReadFromFile();
    balance += money;

    WriteToFile(balance);

    printf("Balance after deposit: %d\n", balance);
    std::ofstream file(FILE_PARH_OUTPUT, std::ios_base::app);
    file << "Balance after deposit: " << balance << std::endl;
    file.close();
    ReleaseMutex(hMutex);
}

void Withdraw(int money) {
    WaitForSingleObject(hMutex, INFINITE);
    int balance = ReadFromFile();
    if (balance < money) {
        printf("Cannot withdraw money, balance lower than %d\n", money);
        std::ofstream file(FILE_PARH_OUTPUT, std::ios_base::app);
        file << "Cannot withdraw money, balance lower than  " << money << std::endl;
        file.close();
    }
    else {
        balance -= money;
        WriteToFile(balance);
        printf("Balance after withdraw: %d\n", balance);
        std::ofstream file(FILE_PARH_OUTPUT, std::ios_base::app);
        file << "Balance after withdraw:  " << balance << std::endl;
        file.close();
    }
    ReleaseMutex(hMutex);
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter)
{
    Deposit((int)lpParameter);
    ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
    Withdraw((int)lpParameter);
    ExitThread(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
    //очищаем файл от предыдущих результатов
    std::ofstream file(FILE_PARH_OUTPUT, std::ofstream::trunc); // Открываем файл для записи и очищаем его
    file.close(); // Закрываем файл

    HANDLE* handles = new HANDLE[50];

    hMutex = CreateMutex(NULL, FALSE, NULL);
    auto start_time = std::chrono::high_resolution_clock::now();
    WriteToFile(0);

    SetProcessAffinityMask(GetCurrentProcess(), 1);
    for (int i = 0; i < 50; i++) {
        handles[i] = (i % 2 == 0)
            ? CreateThread(NULL, 0, &DoDeposit, (LPVOID)230, CREATE_SUSPENDED, NULL)
            : CreateThread(NULL, 0, &DoWithdraw, (LPVOID)1000, CREATE_SUSPENDED, NULL);
        ResumeThread(handles[i]);
    }

    // ожидание окончания работы двух потоков
    WaitForMultipleObjects(50, handles, true, INFINITE);
    // Находим время работы программы
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    printf("Final Balance: %d\n", ReadFromFile());
    std::cout << "Time: " << duration << std::endl;
   
    getchar();

    CloseHandle(hMutex);

    return 0;
}