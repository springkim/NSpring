#include <stdio.h>
#include <windows.h>
#include<conio.h>

typedef void(*HOOKFUNC)();

int main() {
	HMODULE   hDll = NULL;
	HOOKFUNC HookStart = NULL;
	HOOKFUNC HookStop = NULL;
	char   ch = 0;
	hDll = LoadLibraryA("NSpringHook.dll");
	HookStart = (HOOKFUNC)GetProcAddress(hDll, "HookStart");
	HookStop = (HOOKFUNC)GetProcAddress(hDll, "HookStop");
	HookStart();

	printf("press 'q' to quit!\n");
	while (getch() != 'q');

	HookStop();
	FreeLibrary(hDll);
	return 0;
}