/*
*  utils.h
*  NSpring
*
*  Created by kimbomm on 2018. 11. 16...
*  Copyright 2018 kimbomm. All rights reserved.
*
*/
#pragma once
#include<stdio.h>
#include<stdbool.h>
#include<Windows.h>
typedef struct ThreadManage ThreadManage;
struct ThreadManage {
	HWND hwnd;
	HANDLE tid;
	DWORD exitcode;
	ThreadManage* next;
};
static ThreadManage* mspex_head = NULL;
ThreadManage* CreateUniqueThread(DWORD(WINAPI*f)(LPVOID), HWND hwnd);
void UpdateThread(ThreadManage* tm);
HWND* WINAPI GetHwndsByClass(HWND hwnd, char* _class, int* n);