/*
*  dllMain.c
*  NSpring
*
*  Created by kimbomm on 2018. 11. 15...
*  Copyright 2018 kimbomm. All rights reserved.
*
*/
#include<stdio.h>
#include<windows.h>
#include<Shlwapi.h>
#include<direct.h>
#include<stdbool.h>

HINSTANCE g_hInstance = NULL;
HHOOK g_hHook = NULL;
HWND g_hwnd = NULL;

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

BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lparam) {
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lparam) {
		g_hwnd = hwnd;
		return FALSE;
	}
	return TRUE;
}
HWND GetHWND() {
	DWORD pid = GetCurrentProcessId();
	if (pid == 0 || pid == -1)return NULL;
	EnumWindows(EnumProc, (LPARAM)pid);
	return g_hwnd;
}

#define CAPACITY 100
typedef struct  {
	char* text;
	DWORD position;
}NotepadElement;
char g_preallocated_text[CAPACITY][0xFFFF] = { 0 };
char g_preallocated_temp_text[0xFFFF] = { 0 };
char* g_temp_text = NULL;
typedef struct {
	NotepadElement notepad[CAPACITY];
	int loc;
	int top;
	int top2;
}CircularQueue;
void LOG(char* msg) {
	FILE* fp = fopen("C:\\Users\\spring\\Desktop\\hook.txt", "a+");
	fprintf(fp, msg);
	fprintf(fp, "\n");
	fclose(fp);
}
bool isNotepadApp() {
	char szPath[MAX_PATH] = { 0, };
	char *p = NULL;
	GetModuleFileNameA(NULL, szPath, MAX_PATH);
	LOG(szPath);
	p = strrchr(szPath, '\\');
	return !_stricmp(p + 1, "notepad.exe");
}
//윈도우 시작시 키 먹통

LRESULT CALLBACK KeyboardCapture(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode != HC_ACTION)return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	//if (g_hHook != GetCurrentProcessId())return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	LRESULT ret = 0;
#define KEYPRESSED(lparam)	(!(lParam & 0x80000000))
#define KEYRELEASED(lparam)	((lParam & 0x80000000))
	if (nCode >= 0 && isNotepadApp()) {	
		HWND hwnd = GetHWND();
		HWND hwnd_edit = FindWindowExA(hwnd, 0, "Edit", NULL);
		DWORD dummy;
		static CircularQueue queue;
		static bool init = true;
		if (init == true) {
			init = false;
			for (int i = 0; i < CAPACITY; i++) {
				queue.notepad[i].text = g_preallocated_text[i];
				queue.notepad[i].position = -1;
			}
			SendMessageA(hwnd_edit, WM_GETTEXT, (WPARAM)0xFFFF, (LPARAM)queue.notepad[0].text);
			SendMessageA(hwnd_edit, EM_GETSEL, (WPARAM)&dummy, (LPARAM)&queue.notepad[0].position);
			queue.loc = 0;
			queue.top = 0;
			g_temp_text = g_preallocated_temp_text;
		}
		if ((GetKeyState(VK_CONTROL) & 0x80) && (GetKeyState(VK_SHIFT) & 0x80) && ((GetAsyncKeyState('z') & 0x8000) || (GetAsyncKeyState('Z') & 0x8000))) {
			NotepadElement* pnotepad = &queue.notepad[(queue.top+1)%CAPACITY];
			if (queue.top != queue.top2) {
				SendMessageA(hwnd_edit, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);
				int h_pos = GetScrollPos(hwnd_edit, SB_HORZ);
				int v_pos = GetScrollPos(hwnd_edit, SB_VERT);
				SendMessageA(hwnd_edit, EM_SETSEL, 0, -1);
				SendMessageA(hwnd_edit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)pnotepad->text);
				SendMessageA(hwnd_edit, EM_SETSEL, pnotepad->position, pnotepad->position);	//restore caret position
				SetScrollPos(hwnd_edit, SB_HORZ, h_pos, FALSE);	//restore horizontal scroll position
				SendMessageA(hwnd_edit, WM_HSCROLL, MAKELPARAM(SB_THUMBPOSITION, h_pos), (LPARAM)NULL);
				SetScrollPos(hwnd_edit, SB_VERT, v_pos, FALSE);		//restore vertical scroll position
				SendMessageA(hwnd_edit, WM_VSCROLL, MAKELPARAM(SB_THUMBPOSITION, v_pos), (LPARAM)NULL);
				SendMessageA(hwnd_edit, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);
				queue.top = (queue.top + 1) % CAPACITY;
			}
			ret = 1;
		}
		else if ((GetKeyState(VK_CONTROL) & 0x80) && ((GetAsyncKeyState('z') & 0x8000) || (GetAsyncKeyState('Z') & 0x8000))) {
			NotepadElement* pnotepad = &queue.notepad[(queue.top+CAPACITY-1)%CAPACITY];
			if (queue.top!=queue.loc) {
				SendMessageA(hwnd_edit, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);
				int h_pos=GetScrollPos(hwnd_edit, SB_HORZ);
				int v_pos= GetScrollPos(hwnd_edit, SB_VERT);
				SendMessageA(hwnd_edit, EM_SETSEL, 0, -1);
				SendMessageA(hwnd_edit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)pnotepad->text);
				SendMessageA(hwnd_edit, EM_SETSEL, pnotepad->position, pnotepad->position);	//restore caret position
				SetScrollPos(hwnd_edit, SB_HORZ, h_pos, FALSE);	//restore horizontal scroll position
				SendMessageA(hwnd_edit,WM_HSCROLL, MAKELPARAM(SB_THUMBPOSITION, h_pos), (LPARAM)NULL);
				SetScrollPos(hwnd_edit, SB_VERT, v_pos, FALSE);		//restore vertical scroll position
				SendMessageA(hwnd_edit, WM_VSCROLL, MAKELPARAM(SB_THUMBPOSITION, v_pos), (LPARAM)NULL);
				SendMessageA(hwnd_edit, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);
				queue.top = (queue.top + CAPACITY - 1) % CAPACITY;
			}
			ret = 1;
		} else if (KEYRELEASED(lParam) && !(GetKeyState(VK_CONTROL) & 0x80)) {
			SendMessageA(hwnd_edit, WM_GETTEXT, (WPARAM)0xFFFF, (LPARAM)g_temp_text);
			if (strcmp(g_temp_text, queue.notepad[queue.top].text) != 0) {
				/*LOG(queue.notepad[queue.top].text);
				LOG(g_temp_text);
				LOG("==========");*/
				queue.top = (queue.top + 1) % CAPACITY;
				queue.loc = (queue.loc + (queue.top == queue.loc)) % CAPACITY; ;
				NotepadElement* pnotepad = &queue.notepad[queue.top];
				char* tmp = g_temp_text;
				g_temp_text = pnotepad->text;
				pnotepad->text = tmp;
				SendMessageA(hwnd_edit, EM_GETSEL, (WPARAM)&dummy, (LPARAM)&pnotepad->position);
				queue.top2 = queue.top;
			}
		}
	}
	return ret == 0 ? CallNextHookEx(g_hHook, nCode, wParam, lParam) : ret;
}

__declspec(dllexport) HHOOK HookStart(HWND hwnd) {
	g_hHook = SetWindowsHookExA(WH_KEYBOARD, KeyboardCapture, g_hInstance, GetWindowThreadProcessId(hwnd,NULL));
	return g_hHook;
}
__declspec(dllexport) void HookStop(HHOOK hook) {
	if (hook) {
		UnhookWindowsHookEx(hook);
		hook = NULL;
	}
}
