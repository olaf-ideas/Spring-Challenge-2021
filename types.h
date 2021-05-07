#pragma once

#include <bits/stdc++.h>

using namespace std;

typedef unsigned long long BB;
typedef unsigned char Index;

enum TreeType {
	SEED = 0,
	SIZE1 = 1,
	SIZE2 = 2,
	SIZE3 = 3,
	TREE_NB = 4
};

void print(BB bb) {
	static constexpr BB a = 0b0001111'0011111'0111111'1111111'1111110'1111100'1111000ULL;
	for(int i = 0; i < 7; i++) {
		for(int j = 0; j < 7; j++) {
			BB k = 1ULL << (i * 7 + j);
			if((~a) & k)	cerr << 'x';
			else if(bb & k)	cerr << '.';
			else		cerr << 'x';
		}
		cerr << '\n';
	}
}

void print_hex(BB bb) {
	printf("0x%.12llxULL, ", bb);
}

inline int pop_cnt(const BB &bb) {
	return __builtin_popcount(bb);
}

inline int lsb(const BB &b) {
	return __builtin_ctzll(b);
}

inline int pop_lsb(BB &b) {
	const int s = lsb(b);
	b &= b - 1;
	return s;
}

inline short pop_cnt(BB &b) {
	return __builtin_popcountll(b);
}
