#include <Windows.h>
#include <winternl.h>
#include "memory.h"

HANDLE hProcess;
uint64 Base;

bool Read(void* address, void* buffer, uint64 size)
{
	uint64 read;
	return ReadProcessMemory(hProcess, address, buffer, size, &read) && read == size;
}

bool ReaderInit(uint32 pid)
{
	PROCESS_BASIC_INFORMATION pbi;
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, NULL, pid);

	if (!hProcess) return false;
	if (0 > NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL)) goto failed;
	Base = Read<uint64>(reinterpret_cast<uint8*>(pbi.PebBaseAddress) + 0x10);

	if (!Base) goto failed;
	return TRUE;

failed:
	CloseHandle(hProcess);
	return FALSE;
}

uint64 GetImageSize()
{
	char buffer[0x400];
	if (!Read(reinterpret_cast<void*>(Base), buffer, 0x400)) return NULL;

	PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(buffer + reinterpret_cast<PIMAGE_DOS_HEADER>(buffer)->e_lfanew);
	return nt->OptionalHeader.SizeOfImage;
}
