#include<stdio.h>
#include<windows.h>
#include <Shlwapi.h>
#include<direct.h>
#include<stdbool.h>
#pragma comment(lib, "Shlwapi.lib")
HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;
HWND g_hWnd = NULL;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved) {
	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			g_hInstance = hinstDLL;
			break;
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;

}
HWND g_HWND = NULL;
BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lparam) {
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lparam) {
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
}
HWND GetHWND() {
	DWORD pid = GetCurrentProcessId();
	if (pid == 0 || pid == -1)return NULL;
	EnumWindows(EnumProc, (LPARAM)pid);
	return g_HWND;
}
void PushFileQueue(char* dir, char* text, char ch, int size) {
	char curr_path[MAX_PATH] = { 0 };
	char next_path[MAX_PATH] = { 0 };
	for (int i = size - 1; i >= 0; i--) {
		sprintf(curr_path, "%s%d.txt\0", dir, i);
		sprintf(next_path, "%s%d.txt\0", dir, i + 1);
		if (PathFileExistsA(curr_path) == TRUE) {
			MoveFileA(curr_path, next_path);
		}
	}
	FILE* fp = fopen(curr_path, "w");
	fprintf(fp, text);
	fputc(ch, fp);
	fclose(fp);
}

typedef struct Stack Stack;
#define CAPACITY 20
char g_notepad[CAPACITY][0xFFFF] = { 0 };
struct Stack {
	char* notepad[CAPACITY];
	int top;
};
void LOG(char* msg) {
	FILE* fp = fopen("..\\..\\hook.txt", "a+");
	fprintf(fp, msg);
	fclose(fp);
}
bool isNotepadApp() {
	char szPath[MAX_PATH] = { 0, };
	char *p = NULL;
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	p = strrchr(szPath, '\\');
	return !_stricmp(p + 1, "notepad.exe");
}
void ClickAndRestore(POINT dst) {
	POINT point;
	GetCursorPos(&point);
	SetCursorPos(dst.x, dst.y);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	Sleep(1);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); 
	SetCursorPos(point.x, point.y);
}
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
#define KEYPRESSED(lparam)	(!(lParam & 0x80000000))
#define KEYRELEASED(lparam)	((lParam & 0x80000000))
	static Stack stack;
	static bool init = true;
	static char* prev = "";
	if (init == true) {
		init = false;
		for (int i = 0; i < CAPACITY; i++)
			stack.notepad[i] =  g_notepad[i];
		stack.top = 0;
	}
	if (nCode >= 0 && isNotepadApp()) {
		HWND hwnd = GetHWND();
		HWND hwnd_edit = FindWindowExA(hwnd, 0, "Edit", NULL);
		char* notepad = NULL;
		if ((GetKeyState(VK_CONTROL) & 0x80) && ((GetAsyncKeyState('z') & 0x8000) || (GetAsyncKeyState('Z') & 0x8000))) {
			LOG("======Ctrl+Z======\n");
			if (stack.top > 0) {
				notepad = stack.notepad[stack.top-1];
				POINT point;
				GetCaretPos(&point);
				RECT rect;
				GetWindowRect(hwnd_edit, &rect);
				point.x += rect.left;
				point.y += rect.top;
				char str[128] = { 0 };
				sprintf(str, "(%d,%d)\n", point.x, point.y);
				LOG(str);
				if (SendMessageA(hwnd_edit, WM_SETTEXT, 0, notepad) == FALSE) {
					MessageBoxA(NULL, "WM_SETTEXT failure", "fatal", MB_OK);
				}
				ClickAndRestore(point);
				//저 좌표에다가 직접 마우스 한번 찍고 다시 돌아오면 됨.
				stack.top--;
			}
			return 1;
		}
		else if (KEYRELEASED(lParam) && !(GetKeyState(VK_CONTROL) & 0x80)) {
			if (stack.top == CAPACITY) {
				char* tmp = stack.notepad[0];
				memset(tmp, 0, 0xFFFF);
				for (int i = 1; i < CAPACITY; i++)
					stack.notepad[i - 1] = stack.notepad[i];
				notepad=stack.notepad[CAPACITY-1] = tmp;
			} else {
				notepad = stack.notepad[stack.top];
			}
			SendMessageA(hwnd_edit, WM_GETTEXT, 0xFFFF, notepad);
			if(stack.top<CAPACITY) {
				stack.top++;
			}
		}
		for (int i = 0; i < stack.top; i++) {
			LOG(stack.notepad[i]);
			LOG("\n");
		}
		LOG("================\n");
	}
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

__declspec(dllexport) void HookStart() {
	g_hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, g_hInstance, 0);
}
__declspec(dllexport) void HookStop() {
	if (g_hHook) {
		UnhookWindowsHookEx(g_hHook);
		g_hHook = NULL;
	}
}
