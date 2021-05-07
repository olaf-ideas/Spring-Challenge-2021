#pragma once

#include <bits/stdc++.h>

using namespace std;

#include "types.h"

struct Action {};

// remeber that tree_id is (0,49) on board not the this stupid permutation

struct GrowAction : Action {
	Index tree_sz;
	Index tree_id;
	
	string get_name() const {
		return "GROW " + to_string(tree_id);
	}
};

struct SeedAction : Action {
	Index tree_id;
	Index seed_id;
	
	string get_name() const {
		return "SEED " + to_string(tree_id) + " " + to_string(seed_id);
	}
};

struct CompleteAction : Action {
	Index tree_id;
	
	string get_name() const {
		return "COMPLETE " + to_string(tree_id);
	}
};

struct WaitAction : Action { 
	
	string get_name() const {
		return "WAIT";
	}
};

/*
			  2 1    
			3 . 0    
			4 5      

	         25 24 23 22
	      26 11 10 09 21
	   27 12 03 02 08 20
	28 13 04 00 01 07 19
	29 14 05 06 18 36   
	30 15 16 17 35      
	31 32 33 34         

	         03 04 05 06
	      09 10 11 12 13
	   15 16 17 18 19 20
	21 22 23 24 25 26 27
	28 29 30 31 32 33   
	35 36 37 38 39      
	42 43 44 45         
*/

constexpr BB board = 0b0001111'0011111'0111111'1111111'1111110'1111100'1111000ULL;

int richness[BOARD_SZ];

class State {
	BB us, op;
	BB tree[TREE_NB];
	BB dormant;
	short suns;
	short day;
	short day_mod6;
	short nutrient;
	short score;
	Action first_move;
	
	inline short get_suns() {
		
		constexpr static int shift_dir[6] = {
			1,
			6,
			7,
			1,
			6,
			7,
		};
		
		constexpr static BB shift_mask[6][3] = {
			{ 0x1c79f7ef9e38ULL, 0x0c38f3e78e18ULL, 0x041871e38608ULL },
			{ 0x3cfbf7ef9e00ULL, 0x3cf9f3e78000ULL, 0x3c78f1e00000ULL },
			{ 0x3cfbffdf3c00ULL, 0x3cfbef9e0000ULL, 0x3cf3cf000000ULL },
			{ 0x38f3efdf3c70ULL, 0x30e3cf9e3860ULL, 0x20c38f1c3040ULL },
			{ 0x00f3efdfbe78ULL, 0x0003cf9f3e78ULL, 0x00000f1e3c78ULL },
			{ 0x0079f7ffbe78ULL, 0x0000f3efbe78ULL, 0x000001e79e78ULL },
		};
		
		BB shadow = 0ULL;
		int new_suns = 0;
		
		if(1 <= day_mod6 && day_mod6 <= 3) {
			
			if(tree[SIZE3]) {
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][0]) >> (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][1]) >> (2 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][2]) >> (3 * shift_dir[day_mod6]);
				new_suns += 3 * pop_cnt(tree[SIZE3] & (~shadow) & us);
			}
			
			if(tree[SIZE2]) {
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][0]) >> (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][1]) >> (2 * shift_dir[day_mod6]);
				new_suns += 2 * pop_cnt(tree[SIZE2] & (~shadow) & us);
			}
			
			if(tree[SIZE1]) {
				shadow |= (tree[SIZE1] & shift_mask[day_mod6][0]) >> (1 * shift_dir[day_mod6]);
				new_suns += 1 * pop_cnt(tree[SIZE1] & (~shadow) & us);
			}
		} else {
			
			if(tree[SIZE3]) {
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][0]) << (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][1]) << (2 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][2]) << (3 * shift_dir[day_mod6]);
				
				active |= tree[SIZE3] & (~shadow);
			}
			
			if(tree[SIZE2]) {
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][0]) << (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][1]) << (2 * shift_dir[day_mod6]);
				
				active |= tree[SIZE2] & (~shadow);
			}
			
			shadow |= (tree[SIZE1] & shift_mask[day_mod6][0]) << (1 * shift_dir[day_mod6]);
			
			active |= tree[SIZE1] & (~shadow);
		}
		
		active &= us;
		
		return pop_cnt(tree[SIZE3] & active) * 3 +
			   pop_cnt(tree[SIZE2] & active) * 2 +
			   pop_cnt(tree[SIZE1] & active) * 1;
	}
	
	inline void move(const GrowAction &action) {
		static const int grow_cost[3] = {1, 3, 7};
		
		assert(action.tree_sz < 3);
		
		suns -= grow_cost[action.tree_sz] + pop_cnt(tree[action.tree_sz + 1]);
		
		assert(suns >= 0);
		
		// delete old tree
		tree[action.tree_sz + 0] &= ~(1ULL << action.tree_id);
		
		// make new bigger
		tree[action.tree_sz + 1] |= 1ULL << action.tree_id;
	}
	
	inline void move(const SeedAction &action) {
		suns -= pop_cnt(tree[SEED]);
		
		assert(suns >= 0);
		assert((~us) & (1ULL << action.seed_id));
		assert((~op) & (1ULL << action.seed_id));
		
		     us    |= 1ULL << action.seed_id;
		tree[SEED] |= 1ULL << action.seed_id;
	}
	
	inline void move(const CompleteAction &action) {
		suns -= 4;
		assert(suns >= 4);
		assert(tree[SIZE3] & (1ULL << action.tree_id));
		
		score += nutrient + richness[action.tree_id];
		nutrient--;
		if(nutrient < 0)	nutrient = 0;
	}
	
	inline void move(const WaitAction &action) {
		day++;
		day_mod6++;
		if(day_mod6 == 6)	day_mod6 = 0;
		suns += get_suns();
	}
	
	inline bool is_terminal() const {
		return day == 24;
	}
};
