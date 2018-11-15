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
#include"resource.h"
#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON ( WM_USER + 1 )
UINT WM_TASKBARCREATED = 0;
HWND g_hwnd;
HMENU g_menu;
double g_inittime;
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
typedef void(*HookStopFunc)();
typedef HHOOK(*HookStartFunc)();
HMODULE   hDll = NULL;
HookStartFunc HookStart = NULL;
HookStopFunc HookStop = NULL;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int iCmdShow) {
	g_inittime = clock();
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
	hDll = LoadLibraryA("NSpringHook.dll");
	HookStart = (HookStartFunc)GetProcAddress(hDll, "HookStart");
	HookStop = (HookStopFunc)GetProcAddress(hDll, "HookStop");
	HookStart();
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static bool hide = false;
	if ((message == WM_TASKBARCREATED )) {
		Minimize();
		return 0;
	}
	switch (message) {
		case WM_CREATE:
			g_menu = CreatePopupMenu();
			AppendMenu(g_menu, MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, TEXT("Exit"));
			break;
		case WM_SYSCOMMAND:
			switch (wParam & 0xfff0) {
				case SC_MINIMIZE:
				case SC_CLOSE:
					Minimize();
					return 0;
					break;
			}
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
		case WM_NCHITTEST:
		{
			UINT uHitTest = (UINT)DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
			if (uHitTest == HTCLIENT)
				return HTCAPTION;
			else
				return uHitTest;
		}
		case WM_CLOSE:
			Minimize();
			return 0;
			break;
		case WM_DESTROY:
			HookStop();
			FreeLibrary(hDll);
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
typedef struct NSPElem {
	HWND hwnd;
	HHOOK hook;
}NSPElem;

int main() {
	hDll = LoadLibraryA("NSpringHook.dll");
	HookStart = (HookStartFunc)GetProcAddress(hDll, "HookStart");
	HookStop = (HookStopFunc)GetProcAddress(hDll, "HookStop");

	HWND hwnd = NULL;
	while ((hwnd = FindWindowExA(NULL, hwnd, "Notepad", NULL)) != NULL) {
		printf("%x\n", hwnd);
	}
	return 0;
	
	if (hwnd != NULL) {
		HHOOK hook=HookStart(hwnd);
		printf("press 'q' to quit!\n");
		while (getch() != 'q');

		HookStop(hook);
		FreeLibrary(hDll);
		DeleteFileA("C:\\Users\\spring\\Desktop\\hook.txt");
	}
	return 0;
}