/*
*  main.c
*  NSpring
*
*  Created by kimbomm on 2018. 11. 15...
*  Copyright 2018 kimbomm. All rights reserved.
*
*/
#include<stdio.h>
#include<Windows.h>
#include<shellapi.h>
#include<time.h>
#include<stdbool.h>
#include"hash.h"
#include"utils.h"
#include"resource.h"
#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON ( WM_USER + 1 )
UINT WM_TASKBARCREATED = 0;
HWND g_hwnd;
HMENU g_menu;
NOTIFYICONDATAA g_notifyIconData;
void Minimize() {
	Shell_NotifyIconA(NIM_ADD, &g_notifyIconData);
	ShowWindow(g_hwnd, SW_HIDE);
}
void Restore() {
	Shell_NotifyIconA(NIM_DELETE, &g_notifyIconData);
	ShowWindow(g_hwnd, SW_SHOW);
}
void InitNotifyIconData() {
	ZeroMemory(&g_notifyIconData, sizeof(NOTIFYICONDATA));
	g_notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	g_notifyIconData.hWnd = g_hwnd;
	g_notifyIconData.uID = ID_TRAY_APP_ICON;
	g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	g_notifyIconData.uCallbackMessage = WM_TRAYICON;
	g_notifyIconData.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	strcpy_s(g_notifyIconData.szTip, 128, "NSpring ~_~");
}
typedef void(*HookStopFunc)(HHOOK);
typedef HHOOK(*HookStartFunc)(HWND);
HMODULE hDll = NULL;
HookStartFunc HookStart = NULL;
HookStopFunc HookStop = NULL;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool isModalOfMsNotepad(HWND hwnd, char** nullend_modalname) {
	char modal_name[MAX_PATH] = { 0 };
	GetWindowTextA(hwnd, modal_name, MAX_PATH);
	bool b = false;
	for (char** p = nullend_modalname; *p != NULL; p++) {
		b |= strcmp(*p, modal_name) == 0;
	}
	if (b) {
		HWND parent = GetParent(hwnd);
		char parent_class_name[MAX_PATH] = { 0 };
		GetClassNameA(parent, parent_class_name, MAX_PATH);
		if (!strcmp(parent_class_name, "Notepad")) {
			return true;
		}
	}
	return false;
}
DWORD WINAPI NSpringAboutNotepad(LPVOID param) {
	ThreadManage* tm = (ThreadManage*)param;
	RECT dlg_crect;
	GetClientRect(tm->hwnd, &dlg_crect);
	HDC hdc = GetWindowDC(tm->hwnd);
	HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SelectObject(hdc, font);
	SetTextColor(hdc, RGB(77,77,77));
	SetBkMode(hdc, TRANSPARENT);
	HICON icon= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));
	while (IsWindow(tm->hwnd)) {
		char* str1 = "Powered by NSpring";
		char* str2 = "https://www.github.com/springkim/NSpring";
		TextOutA(hdc, 70, 250, str1, strlen(str1));
		TextOutA(hdc, 70, 270, str2, strlen(str2));
		DrawIcon(hdc, 25, 250, icon);
		Sleep(50);
	}
	DestroyIcon(icon);
	return tm->exitcode = 0;
}
DWORD WINAPI HelpThread(LPVOID param) {
	char* about_notepad[] = { "메모장 정보","About Notepad",NULL };
	HWND hwnd;
	while (1) {
		hwnd = GetForegroundWindow();
		ThreadManage* tm = NULL;
		if (isModalOfMsNotepad(hwnd, about_notepad)) {
			tm = CreateUniqueThread(NSpringAboutNotepad, hwnd);
		} 
		UpdateThread(tm);
		Sleep(300);
	}
	return 1;
}
DWORD WINAPI HookThread(LPVOID param) {
	hDll = LoadLibraryA("NSpringHook.dll");
	HookStart = (HookStartFunc)GetProcAddress(hDll, "HookStart");
	HookStop = (HookStopFunc)GetProcAddress(hDll, "HookStop");
	Hash hash = Create();
	while (1) {	
		HWND hwnd = NULL;
		while ((hwnd = FindWindowExA(NULL, hwnd, "Notepad", NULL)) != NULL) {
			NSPElem elem;
			elem.hwnd = hwnd;
			HHOOK* phook = Insert(&hash, elem);
			if (phook != NULL) {
				*phook = HookStart(hwnd);
			}
		}
		NSPElem* temp = (NSPElem*)calloc(hash.capacity, sizeof(NSPElem));
		unsigned char* bloom = (unsigned char*)calloc(hash.capacity, sizeof(unsigned char));
		memcpy(temp, hash.base, hash.capacity * sizeof(NSPElem));
		memcpy(bloom, hash.bloom, hash.capacity);
		int capacity = hash.capacity;
		for (int i = 0; i < capacity; i++) {
			if (bloom[i]==INSERTED && IsWindow(temp[i].hwnd) == FALSE) {
				HookStop(temp[i].hook);
				Erase(&hash, temp[i]);
			}
		}
		free(temp);
		Sleep(200);
	}
	return 1;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow) {
	HANDLE hEvent = CreateEventA(NULL, FALSE, TRUE, "NSpring");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		return 1;
	}
	TCHAR className[] = TEXT("NSpring");
	WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");
	WNDCLASSEX wnd = { 0 };
	wnd.hInstance = hInstance;
	wnd.lpszClassName = className;
	wnd.lpfnWndProc = WndProc;
	wnd.style = CS_HREDRAW | CS_VREDRAW;
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;
	if (!RegisterClassEx(&wnd)) {
		FatalAppExit(0, TEXT("Couldn't register window class!"));
	}
	g_hwnd = CreateWindowA("NSpring", "NSpring", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	UpdateWindow(g_hwnd);
	InitNotifyIconData();

	


	Minimize();
	int tid = 0;
	CreateThread(NULL, 0, HookThread, NULL, 0, &tid);
	CreateThread(NULL, 0, HelpThread, NULL, 0, &tid);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static bool hide = false;
	if ((message == WM_TASKBARCREATED)) {
		Minimize();
		return 0;
	}
	switch (message) {
		case WM_CREATE:
			g_menu = CreatePopupMenu();
			AppendMenu(g_menu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, TEXT("Exit"));
			break;
		case WM_TRAYICON:
		{
			if (lParam == WM_RBUTTONDOWN) {
				POINT curPoint;
				GetCursorPos(&curPoint);
				SetForegroundWindow(hwnd);
				UINT clicked = TrackPopupMenu(g_menu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);
				if (clicked == ID_TRAY_EXIT_CONTEXT_MENU_ITEM) {
					PostQuitMessage(0);
				}
			}
		}
		break;
		case WM_CLOSE:
			Minimize();
			return 0;
			break;
		case WM_DESTROY:
			FreeLibrary(hDll);
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
int main() {
	
	return 0;
}