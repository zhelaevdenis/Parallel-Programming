#include <iostream>
#include <windows.h>



// �������, ������� ��������� ������ �����
DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
	int threadNumber = (int)lpParam;

	std::cout << "Thread " << threadNumber << " works" << std::endl;

	ExitThread(0); // ������� ������������� ��� ���������� ������ � 0
}

int main()
{
	int n = 2;
	// �������� ���� �������
	HANDLE* handles = new HANDLE[n];

	for (int i = 0; i < n; i++)
	{
		handles[i] = CreateThread(NULL, 0, &ThreadFunction, (LPVOID)i, CREATE_SUSPENDED, NULL);
	}

	
	// ������ ���� �������
	for (int i = 0; i < n; i++)
	{
		ResumeThread(handles[i]);
	}

	// �������� ��������� ������ ���� �������
	WaitForMultipleObjects(n, handles, true, INFINITE);
	return 0;
}