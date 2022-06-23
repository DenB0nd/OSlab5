#include <iostream>
#include <windows.h>
#include <fstream>
#include <codecvt>
#include <string>

struct employee
{
	int num;
	char name[10];
	double hours;
};

struct message
{
	char type;
	int id;
};

std::fstream file;
employee emp;
HANDLE* servers;
int processes, records;

void thread(HANDLE hNamedPipe);

void startProccesses()
{
	DWORD* idthread = new DWORD[processes];
	HANDLE* threads = new HANDLE[processes];
	for (int i = 0; i < processes; i++)
	{
		HANDLE hNamedPipe = CreateNamedPipe(
			L"\\\\.\\pipe\\named_pipe",
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_WAIT,
			processes,
			0,
			0,
			INFINITE,
			(LPSECURITY_ATTRIBUTES)NULL
		);
		threads[i] = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(thread), (void*)hNamedPipe, 0, &idthread[i]);
	}
	WaitForMultipleObjects(processes, threads, true, INFINITE);
}

void thread(HANDLE hNamedPipe)
{
	ConnectNamedPipe(hNamedPipe, NULL);

	while (WaitForMultipleObjects(processes, servers, true, 0))
	{
		message msg;
		DWORD dwRead;
		ReadFile(hNamedPipe, (char*)&msg, sizeof(msg), &dwRead, NULL);
		file.clear();
		file.seekg(0);

		int pos;
		int counter = 0;
		while (counter < records && emp.num == msg.id)
		{
			pos = file.tellg();
			file.read((char*)&emp, sizeof(emp));
			counter++;
		}

		WriteFile(hNamedPipe, &emp, sizeof(emp), &dwRead, NULL);
		if (msg.type != 'w')
		{
			return;
		}

		ReadFile(hNamedPipe, &emp, sizeof(emp), &dwRead, NULL);
		for (int i = 0; i < records; i++)
		{
			file.clear();
			file.seekg(pos);
			file.write((char*)&emp, sizeof(emp));
		}
	}
}

int main()
{
	std::string fileName;
	std::cout << "Input file name\n";
	std::cin >> fileName;
	std::cout << "Input employees number\n";
	std::cin >> records;
	file = std::fstream(fileName, std::ios::trunc | std::ios::binary);


	HANDLE* readAccess = new HANDLE[records];
	HANDLE* writeAccess = new HANDLE[records];

	for (int i = 0; i < records; i++)
	{
		std::cout << "Number:\n";
		std::cin >> emp.num;
		std::cout << "Name:\n";
		std::cin >> emp.name;
		std::cout << "Hours:\n";
		std::cin >> emp.hours;
		file.write((char*)&emp, sizeof(emp));
		readAccess[i] = CreateEvent(NULL,
			1,
			1,
			(LPWSTR)std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes("r" + std::to_string(emp.num)).c_str());
		writeAccess[i] = CreateEvent(NULL,
			1,
			1,
			(LPWSTR)std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes("w" + std::to_string(emp.num)).c_str());
	}

	std::cout << std::endl << fileName << std::endl;
	file.clear();
	file.seekg(0);

	for (int i = 0; i < records; i++)
	{
		file.read((char*)&emp, sizeof(emp));
		std::cout << emp.num << std::endl << emp.name << std::endl << emp.hours << std::endl;
	}

	std::cout << "Input processes number\n";
	std::cin >> processes;

	system("pause");
}