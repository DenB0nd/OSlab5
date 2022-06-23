#include <windows.h>
#include <iostream>
#include <string>
#include <codecvt>
#include <locale>
#include <conio.h>


HANDLE hPipe;
DWORD cbRead, dwMode;

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

void readModification()
{
	boolean ok = false;
	while (!ok)
	{

		std::cout << "\nInput ID\n";
		int id;
		std::cin >> id;
		HANDLE* event = new HANDLE[2];
		event[0] = OpenEvent(EVENT_ALL_ACCESS, true, (LPWSTR)std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes("r" + std::to_string(id)).c_str());
		if (event[0] == NULL)
		{
			std::cout << "\nThere is no employee with this ID\n";
			continue;
		}

		ok = true;
		event[1] = OpenEvent(EVENT_ALL_ACCESS, false, (LPWSTR)std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes("w" + std::to_string(id)).c_str());
		std::cout << "\nRecord blocked. Wait...\n";

		WaitForSingleObject(event[1], INFINITE);
		ResetEvent(event[0]);
		message msg;
		msg.id = id;
		msg.type = 'r';
		employee emp;
		TransactNamedPipe(
			hPipe,
			(char*)&msg,
			sizeof(msg),
			(char*)&emp,
			sizeof(emp),
			&cbRead,
			NULL);
		std::cout << "Name: " << emp.name << "\nHours: " << emp.hours << "\nPress any button...\n";
		_getch();
		SetEvent(event[0]);
	}
}

void recordModification()
{
	boolean ok = false;
	while (!ok)
	{
		std::cout << "Input ID\n";
		int id;
		std::cin >> id;
		HANDLE* event = new HANDLE[2];
		event[0] = OpenEvent(EVENT_ALL_ACCESS, true, (LPWSTR)std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes("r" + std::to_string(id)).c_str());
		if (event[0] == NULL)
		{
			std::cout << "\nThere is no employee with this ID\n";
			continue;
		}

		ok = true;
		event[1] = OpenEvent(EVENT_ALL_ACCESS, false, (LPWSTR)std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes("w" + std::to_string(id)).c_str());
		std::cout << "Record blocked. Wait...\n";
		WaitForMultipleObjects(2, event, true, INFINITE);
		ResetEvent(event[1]);
		message msg;
		msg.id = id;
		msg.type = 'w';
		employee emp;
		TransactNamedPipe(
			hPipe,
			(char*)&msg,
			sizeof(msg),
			(char*)&emp,
			sizeof(emp),
			&cbRead,
			NULL);
		std::cout << "Name: " << emp.name << "\nHours: " << emp.hours << "\nNew name:\n";
		std::cin >> emp.name;
		std::cout << "\nNew hours\n";
		std::cin >> emp.hours;
		DWORD dwRead;
		WriteFile(hPipe, (char*)&emp, sizeof(emp), &dwRead, NULL);
		std::cout << "\nPress any button...\n";
		_getch();
		SetEvent(event[1]);
	}
}

bool waitForPing()
{
	while (true)
	{
		WaitNamedPipe(L"\\\\.\\pipe\\named_pipe", INFINITE);
		hPipe = CreateFile(
			L"\\\\.\\pipe\\named_pipe",
			GENERIC_READ |
			GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);


		if (hPipe != INVALID_HANDLE_VALUE)
		{
			return true;
		}

		if (GetLastError() != ERROR_PIPE_BUSY ||
			!WaitNamedPipe(L"\\\\.\\pipe\\named_pipe", 20000))
		{
			return false;
		}
	}
}

int main()
{
	if (!waitForPing())
	{
		return 0;
		printf("Could not open pipe\n");
	}

	dwMode = PIPE_READMODE_MESSAGE;
	boolean fSuccess = SetNamedPipeHandleState(
		hPipe,
		&dwMode,
		NULL,
		NULL);

	if (!fSuccess)
	{
		printf("SetNamedPipeHandleState failed.\n");
		return 0;
	}

	while (true)
	{
		std::cout << "1 - модифицировать запись\n2 - прочитать запись\n3 - выйти\n";
		int ans;
		std::cin >> ans;

		if (ans == 1)
		{
			recordModification();
		}
		else if (ans == 2)
		{
			readModification();
		}
		else 
		{
			return 0;
		}
	}
}