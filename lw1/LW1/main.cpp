#include <iostream>
#include <windows.h>



// Функция, которую выполняет каждый поток
DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
	int threadNumber = (int)lpParam;

	std::cout << "Thread " << threadNumber << " works" << std::endl;

	ExitThread(0); // функция устанавливает код завершения потока в 0
}

int main()
{
	int n = 2;
	// создание двух потоков
	HANDLE* handles = new HANDLE[n];

	for (int i = 0; i < n; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadFunction, (LPVOID)i, CREATE_SUSPENDED, NULL);
	}

	
	// запуск двух потоков
	for (int i = 0; i < n; i++)
	{
		ResumeThread(handles[i]);
	}

	// ожидание окончания работы двух потоков
	WaitForMultipleObjects(n, handles, true, INFINITE);
	return 0;
}