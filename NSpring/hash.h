/*
*  hash.h
*  NSpring
*
*  Created by kimbomm on 2018. 11. 16...
*  Copyright 2018 kimbomm. All rights reserved.
*
*/
#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<Windows.h>
typedef struct NSPElem {
	HWND hwnd;
	HHOOK hook;
}NSPElem;
typedef struct Hash {
	NSPElem* base;
	unsigned char* bloom;
#define EMPTY 0
#define INSERTED 1
#define DELETED 2
	int length;
	int capacity;
}Hash;
int is_prime(int n);
size_t hash4(size_t key);
size_t hash(void* _key, size_t n);
size_t HashFunc1(HWND key, int size);
size_t HashFunc2(HWND key, int size);
HHOOK* _Insert(NSPElem* base, unsigned char* bloom, int capacity, NSPElem key);
HHOOK* Insert(Hash* hash, NSPElem key);
void Erase(Hash* hash, NSPElem key);
Hash Create();