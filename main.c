#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <sysinfoapi.h>

void monitor(double allowed_memory);

int main()
{
	HWND hwnd = GetConsoleWindow();
	MEMORYSTATUSEX mem_status = {0};
	mem_status.dwLength = sizeof(mem_status);

	if (!GlobalMemoryStatusEx(&mem_status))
	{
		printf("GlobalMemoryStatusEx(&mem_status) returned %d\n", GetLastError());
		return -1;
	}

	unsigned long long physical_mem = mem_status.ullTotalPhys;
	double physical_mem_gb = physical_mem / 1073741824;

	printf("You have %lf GB of physical memory in this system.\n", physical_mem_gb);

	float percentage = 0;

	printf("What percentage of your memory should every application be allowed? >> ");
	scanf("%f", &percentage);

	double p_decimal = percentage / 100;

	double allowed_memory = physical_mem * p_decimal;

	printf("You are only allowing %lf GBs of memory for each application. If this is OK, this window will hide in 5 seconds. If this is not OK, please exit this program with Ctrl+C.\n", allowed_memory / 1073741824);
	Sleep(5000);

	ShowWindow(hwnd, SW_HIDE);
	monitor(allowed_memory);

	return 0;
}

void monitor(double allowed_memory)
{
	for (;;)
	{

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (snapshot && snapshot != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 process_entry = {0};
			process_entry.dwSize = sizeof(process_entry);

			if (Process32First(snapshot, &process_entry))
			{
				do
				{
					PROCESS_MEMORY_COUNTERS pmc = { 0 };
					HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_entry.th32ProcessID);
					if (handle != INVALID_HANDLE_VALUE)
					{
						K32GetProcessMemoryInfo(handle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
						SIZE_T mem_used = pmc.WorkingSetSize;
						if ((double)mem_used >= allowed_memory)
						{
							TerminateProcess(handle, -1);

							char msg_buffer[512];
							sprintf(msg_buffer, "Mema has detected that %s has been using too much memory.\nMemory used: %lf GBs.", process_entry.szExeFile, ((double)mem_used / (double)1073741824));
							MessageBox(NULL, msg_buffer, "Mema Termination", 0);
						}
						CloseHandle(handle);
					}
				} 
				while (Process32Next(snapshot, &process_entry));
			}
		}
		Sleep(5);
	}
}