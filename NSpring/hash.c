/*
*  hash.c
*  NSpring
*
*  Created by kimbomm on 2018. 11. 16...
*  Copyright 2018 kimbomm. All rights reserved.
*
*/
#include"hash.h"
int is_prime(int n) {
	int i = 5;
	if (!(n & 1))
		return n == 2;
	if (n % 3 == 0)
		return n == 3;
	while (i*i <= n)
		if (n%i++ == 0)return 0;
	return n != 1;
}
size_t hash4(size_t key) {
	key = ~key + (key << 15); // key = (key << 15) - key - 1;
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057; // key = (key + (key << 3)) + (key << 11);
	key = key ^ (key >> 16);
	return key;
}
size_t hash(void* _key, size_t n) {
	char* key = (char*)_key;
	char* e = key + n;
	size_t ret = 0;
	while (key + sizeof(size_t) <= e) {
		size_t tmp = hash4(*(size_t*)key);
		ret ^= tmp;
		key += sizeof(size_t);
	}
	return ret;
}
size_t HashFunc1(HWND key, int size) {
	return hash(&key, sizeof(HWND)) % size;
}
size_t HashFunc2(HWND key, int size) {
	size_t h1 = HashFunc1(key, size);
	return h1 % (size - 1) + 1;
}
HHOOK* _Insert(NSPElem* base, unsigned char* bloom, int capacity, NSPElem key) {
	size_t h1 = HashFunc1(key.hwnd, capacity);
	size_t h2 = HashFunc2(key.hwnd, capacity);
	size_t pos = h1;
	while (bloom[pos] == INSERTED) {
		if (base[pos].hwnd == key.hwnd)return NULL;
		pos = (pos + h2) % capacity;
	}
	bloom[pos] = INSERTED;
	base[pos] = key;
	return &base[pos].hook;
}
HHOOK* Insert(Hash* hash, NSPElem key) {
	if (hash->length > hash->capacity*0.7) {
		int capacity = hash->capacity * 2;
		while (is_prime(capacity) == 0)capacity++;
		NSPElem* base = (NSPElem*)calloc(capacity, sizeof(NSPElem));
		unsigned char* bloom = (unsigned char*)calloc(capacity, sizeof(unsigned char));
		for (int i = 0; i < hash->capacity; i++) {
			if (hash->bloom[i] == true) {
				_Insert(base, bloom, capacity, hash->base[i]);
			}
		}
		free(hash->base);
		free(hash->bloom);
		hash->base = base;
		hash->bloom = bloom;
		hash->capacity = capacity;
	}
	HHOOK* ret = NULL;
	if ((ret=_Insert(hash->base, hash->bloom, hash->capacity, key))) {
		hash->length++;
	}
	return ret;
}
void Erase(Hash* hash, NSPElem key) {
	size_t h1 = HashFunc1(key.hwnd, hash->capacity);
	size_t h2 = HashFunc2(key.hwnd, hash->capacity);
	size_t pos = h1;
	while (hash->bloom[pos] != EMPTY && memcmp(&hash->base[pos], &key,sizeof(NSPElem))) {
		pos = (pos + h2) % hash->capacity;
	}
	if (memcmp(&hash->base[pos], &key,sizeof(NSPElem))) {
		return;
	}
	size_t sw = pos;
	size_t mv = pos;
	while (hash->bloom[mv] != EMPTY) {
		if (hash->bloom[mv] != DELETED && HashFunc1(hash->base[mv].hwnd, hash->capacity) == h1) {
			sw = mv;
		}
		mv = (mv + h2) % hash->capacity;
	}

	hash->base[pos] = hash->base[sw];
	hash->bloom[pos] = hash->bloom[sw];

	hash->bloom[sw] = DELETED;
	hash->length--;
}
Hash Create() {
	Hash hash;
	hash.base = (NSPElem*)calloc(7, sizeof(NSPElem));
	hash.bloom = (unsigned char*)calloc(7, sizeof(unsigned char));
	hash.length = 0;
	hash.capacity = 7;
	return hash;
}