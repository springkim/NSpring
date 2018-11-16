/*
*  utils.c
*  NSpring
*
*  Created by kimbomm on 2018. 11. 16...
*  Copyright 2018 kimbomm. All rights reserved.
*
*/
#include"utils.h"
ThreadManage* CreateUniqueThread(DWORD(WINAPI*f)(LPVOID), HWND hwnd) {
	DWORD tid = 0;
	ThreadManage* n = mspex_head;
	while (n) {
		if (n->hwnd == hwnd) {
			return NULL;
		}
		n = n->next;
	}
	ThreadManage* tm = (ThreadManage*)malloc(sizeof(ThreadManage));
	tm->hwnd = hwnd;
	tm->next = NULL;
	tm->exitcode = -1;
	tm->tid = CreateThread(NULL, 0, f, tm, 0, &tid);
#ifdef _DEBUG
	printf("%s : hwnd(%zd) created\n", __FUNCTION__, hwnd);
#endif
	return tm;
}
void UpdateThread(ThreadManage* tm) {
	//Add thread into pool
	if (tm) {
		tm->next = mspex_head;
		mspex_head = tm;
	}
	//Delete thread that has exit signal.
	ThreadManage** phead = &mspex_head;
	while (*phead) {
		ThreadManage* tmp = *phead;
		bool b = (**phead).exitcode != -1;
		if (b)
			*phead = (**phead).next;
		if (*phead)
			phead = &(**phead).next;
		if (b) {
			CloseHandle(tmp->tid);
			free(tmp);
		}
	}
}
HWND* WINAPI GetHwndsByClass(HWND hwnd, char* _class, int* n) {
	HWND* hwnds = NULL;
	size_t hwnds_sz = 0;
	HWND tmp = NULL;
	HWND lhwnd = NULL;
	HWND c = 0;
	*n = 0;
	do {
		lhwnd = tmp;
		tmp = FindWindowExA(hwnd, tmp, NULL, NULL);
		char str[MAX_PATH] = { 0 };
		RealGetWindowClassA(lhwnd, str, MAX_PATH);
		if (strcmp(str, _class) == 0) {
			hwnds = realloc(hwnds, ++hwnds_sz * sizeof(HWND));
			(*n)++;
			hwnds[hwnds_sz - 1] = lhwnd;
		}
	} while (tmp);
	return hwnds;
}