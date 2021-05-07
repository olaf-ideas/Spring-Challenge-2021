#pragma once

#include <bits/stdc++.h>

using namespace std;

#include "types.h"

// remeber that tree_id is (0,49) on board, not this stupid permutation (0, 36)

// to_bitboard[index in game] = index on bitboard
constexpr int to_bitboard[37] = {
	24, 25, 18, 17, 23, 30, 31, 26, 19, 12, 11, 10, 16, 22, 29, 36, 37, 38, 32, 27, 20, 13, 6, 5, 4, 3, 9, 15, 21, 28, 35, 42, 43, 44, 45, 39, 33
};

constexpr int to_game_id[49] = {
	-1, -1, -1, 25, 24, 23, 22,
	-1, -1, 26, 11, 10,  9, 21,
	-1, 27, 12,  3,  2,  8, 20,
	28, 13,  4,  0,  1,  7, 19,
	29, 14,  5,  6, 18, 36, -1,
	30, 15, 16, 17, 35, -1, -1,
	31, 32, 33, 34, -1, -1, -1, 
};

enum ActionType {
	GROW,
	SEED_action,
	COMPLETE,
	WAIT
};

struct Action {
	virtual string get_name() = 0;
};

struct GrowAction : Action {
	Index tree_sz;
	Index tree_id;
	
	GrowAction() { }
	GrowAction(Index _tree_sz, Index _tree_id) : tree_sz(_tree_sz), tree_id(_tree_id) { }
	
	string get_name() override {
		return "GROW " + to_string(to_game_id[tree_id]);
	}
};

struct SeedAction : Action {
	Index tree_id;
	Index seed_id;
	
	SeedAction() { }
	SeedAction(Index _tree_id, Index _seed_id) : tree_id(_tree_id), seed_id(_seed_id) { }
	
	string get_name() override {
		return "SEED " + to_string(to_game_id[tree_id]) + " " + to_string(to_game_id[seed_id]);
	}
};

struct CompleteAction : Action {
	Index tree_id;
	
	CompleteAction() { }
	CompleteAction(Index _tree_id) : tree_id(_tree_id) { }
	
	inline string get_name() override {
		return "COMPLETE " + to_string(to_game_id[tree_id]);
	}
};

struct WaitAction : Action {
	
	WaitAction() { }
	inline string get_name() override {
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

BB usable = 0ULL;
int richness[BOARD_SZ];

int big_tree_index = -1;

struct State {
	BB us, op;
	BB tree[TREE_NB];
	BB dormant;
	short suns;
	short turns;
	short day;
	short day_mod6;
	short nutrients;
	short score;
	
	float eval_score;
	
	ActionType first_action_type;
	short first_action_index;
	
	inline void evaluate() {
		eval_score = 0;
		
		eval_score += score * 7;
		eval_score += min((short) 20, suns);
		
		eval_score += turns * 5; // more actions = more points usage / but not too much
		
		// eval_score += min(suns / 4, pop_cnt(tree[SIZE3] & us)) * nutrients; 
		
		static constexpr BB shadow_mask[49][3] = {
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000200008ULL, },
			{ 0x000000000000ULL, 0x000000008008ULL, 0x000000408018ULL, },
			{ 0x000000000208ULL, 0x000000010218ULL, 0x000000810238ULL, },
			{ 0x000000000610ULL, 0x000000028630ULL, 0x000001228670ULL, },
			{ 0x000000000c28ULL, 0x000000050c68ULL, 0x000002450c68ULL, },
			{ 0x000000001850ULL, 0x0000000a1858ULL, 0x0000048a1858ULL, },
			{ 0x000000003020ULL, 0x000000143030ULL, 0x000009143038ULL, },
			{ 0x000000000000ULL, 0x000000200200ULL, 0x000010200600ULL, },
			{ 0x000000008200ULL, 0x000000408600ULL, 0x000020408e00ULL, },
			{ 0x000000018408ULL, 0x000000a18c08ULL, 0x000040a19c08ULL, },
			{ 0x000000030a18ULL, 0x000001431a18ULL, 0x000091433a18ULL, },
			{ 0x000000061430ULL, 0x000002863630ULL, 0x000122863630ULL, },
			{ 0x0000000c2860ULL, 0x0000050c2c60ULL, 0x0002450c2e60ULL, },
			{ 0x000000181040ULL, 0x00000a181840ULL, 0x00008a181c40ULL, },
			{ 0x000000208000ULL, 0x000010218000ULL, 0x000810238000ULL, },
			{ 0x000000610200ULL, 0x000020630208ULL, 0x001020670208ULL, },
			{ 0x000000c28600ULL, 0x000050c68610ULL, 0x002050ce8610ULL, },
			{ 0x000001850c00ULL, 0x0000a18d8c28ULL, 0x0048a19d8c28ULL, },
			{ 0x0000030a1800ULL, 0x0001431b1850ULL, 0x0091431b9850ULL, },
			{ 0x000006143000ULL, 0x000286163020ULL, 0x002286173020ULL, },
			{ 0x00000c082000ULL, 0x00010c0c2040ULL, 0x00410c0e2040ULL, },
			{ 0x000010408000ULL, 0x000810c08200ULL, 0x040811c08208ULL, },
			{ 0x000030a18000ULL, 0x001031a18400ULL, 0x081033a18410ULL, },
			{ 0x000061430000ULL, 0x002863630a00ULL, 0x102867630a20ULL, },
			{ 0x0000c2860000ULL, 0x0050c6c61400ULL, 0x2450cee61448ULL, },
			{ 0x0001850c0000ULL, 0x00a18d8c2800ULL, 0x08a18dcc2810ULL, },
			{ 0x00030a180000ULL, 0x00430b181000ULL, 0x10430b981020ULL, },
			{ 0x000204100000ULL, 0x008206102000ULL, 0x208207102040ULL, },
			{ 0x000820600000ULL, 0x040860610000ULL, 0x0408e0610400ULL, },
			{ 0x001850c00000ULL, 0x0818d0c28000ULL, 0x0819d0c28800ULL, },
			{ 0x0030a1800000ULL, 0x1431b1850000ULL, 0x1433b1851200ULL, },
			{ 0x006143000000ULL, 0x2863630a0000ULL, 0x2863730a2400ULL, },
			{ 0x00c286000000ULL, 0x10c2c6140000ULL, 0x10c2e6140800ULL, },
			{ 0x00810c000000ULL, 0x20818c080000ULL, 0x2081cc081000ULL, },
			{ 0x000208000000ULL, 0x000308100000ULL, 0x000388102000ULL, },
			{ 0x041030000000ULL, 0x043030a00000ULL, 0x047030a20000ULL, },
			{ 0x0c2860000000ULL, 0x0c6861400000ULL, 0x0ce861448000ULL, },
			{ 0x1850c0000000ULL, 0x18d8c2800000ULL, 0x18d8c2890000ULL, },
			{ 0x30a180000000ULL, 0x30b185000000ULL, 0x30b985120000ULL, },
			{ 0x204300000000ULL, 0x20630a000000ULL, 0x20730a040000ULL, },
			{ 0x008200000000ULL, 0x00c204000000ULL, 0x00e204080000ULL, },
			{ 0x000000000000ULL, 0x008008000000ULL, 0x00c008100000ULL, },
			{ 0x081800000000ULL, 0x181850000000ULL, 0x381851200000ULL, },
			{ 0x143000000000ULL, 0x3430a0000000ULL, 0x3430a2400000ULL, },
			{ 0x286000000000ULL, 0x2c6140000000ULL, 0x2c6144800000ULL, },
			{ 0x10c000000000ULL, 0x18c280000000ULL, 0x1cc289000000ULL, },
			{ 0x208000000000ULL, 0x308100000000ULL, 0x388102000000ULL, },
			{ 0x000000000000ULL, 0x200200000000ULL, 0x300204000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x200008000000ULL, },
		};
		
		BB tree3 = tree[SIZE3] & us;
		
		while(tree3) {
			Index pos = pop_lsb(tree3);
			eval_score += pop_cnt(op & shadow_mask[pos][2]) * 2;
			eval_score -= pop_cnt(us & shadow_mask[pos][2]) * 1;
		}
		
		BB tree2 = tree[SIZE2] & us;
		
		while(tree2) {
			Index pos = pop_lsb(tree2);
			eval_score += pop_cnt(op & shadow_mask[pos][1]) * 2;
			eval_score -= pop_cnt(us & shadow_mask[pos][1]) * 1;
		}
		
		BB tree1 = tree[SIZE1] & us;
		
		while(tree1) {
			Index pos = pop_lsb(tree1);
			eval_score += pop_cnt(op & shadow_mask[pos][0]) * 2;
			eval_score -= pop_cnt(us & shadow_mask[pos][0]) * 1;
		}
		
		int size3 = pop_cnt(tree[SIZE3] & us);
		
		if(size3 <= 6)	eval_score += size3 * 3;
		else			eval_score -= size3 * 5;
		
		int size2 = pop_cnt(tree[SIZE2] & us);
		
		if(size2 <= 5)	eval_score += size2;
		else			eval_score -= size2 * 2;
		
		int size1 = pop_cnt(tree[SIZE1] & us);
		
		if(size1 <= 5)	eval_score += size1;
		else			eval_score -= size1 * 2;
		
		int size0 = pop_cnt(tree[SIZE2] & us);
		
		if(size0 <= 4)	eval_score += size0;
		else			eval_score -= size0 * 2; 
	}
	
	inline bool operator < (const State &other) const {
		return eval_score > other.eval_score;
	}
	
	inline int get_best_seed(int index, int size) const {
		static constexpr BB seeding_mask[49][3] = {
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000610ULL, 0x000000038e38ULL, 0x000001e79e78ULL, },
			{ 0x000000000c28ULL, 0x000000071e78ULL, 0x000003cfbe78ULL, },
			{ 0x000000001850ULL, 0x0000000e3c78ULL, 0x0000079f3e78ULL, },
			{ 0x000000003020ULL, 0x0000001c3870ULL, 0x00000f1e3c78ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000018408ULL, 0x000000e38e18ULL, 0x000071e79e38ULL, },
			{ 0x000000030a18ULL, 0x000001c79e38ULL, 0x0000f3efbe78ULL, },
			{ 0x000000061430ULL, 0x0000038f3e78ULL, 0x0001e7dfbe78ULL, },
			{ 0x0000000c2860ULL, 0x0000071e3c70ULL, 0x0003cf9f3e78ULL, },
			{ 0x000000181040ULL, 0x00000e1c3860ULL, 0x00038f1e3c70ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000610200ULL, 0x000030e38608ULL, 0x001871e78e18ULL, },
			{ 0x000000c28600ULL, 0x000071e78e18ULL, 0x0038f3ef9e38ULL, },
			{ 0x000001850c00ULL, 0x0000e3cf9e38ULL, 0x0079f7ffbe78ULL, },
			{ 0x0000030a1800ULL, 0x0001c79f3c70ULL, 0x00f3efdfbe78ULL, },
			{ 0x000006143000ULL, 0x00038f1e3860ULL, 0x00e3cf9f3c70ULL, },
			{ 0x00000c082000ULL, 0x00030e1c3040ULL, 0x00c38f1e3860ULL, },
			{ 0x000010408000ULL, 0x000830e18200ULL, 0x041871e38608ULL, },
			{ 0x000030a18000ULL, 0x001871e38600ULL, 0x0c38f3e78e18ULL, },
			{ 0x000061430000ULL, 0x0038f3e78e00ULL, 0x1c79f7ef9e38ULL, },
			{ 0x0000c2860000ULL, 0x0071e7cf1c00ULL, 0x3cfbffffbe78ULL, },
			{ 0x0001850c0000ULL, 0x00e3cf9e3800ULL, 0x38f3efdf3c70ULL, },
			{ 0x00030a180000ULL, 0x00c38f1c3000ULL, 0x30e3cf9e3860ULL, },
			{ 0x000204100000ULL, 0x00830e182000ULL, 0x20c38f1c3040ULL, },
			{ 0x000820600000ULL, 0x041870e18000ULL, 0x0c38f1e38600ULL, },
			{ 0x001850c00000ULL, 0x0c38f1e38000ULL, 0x1c79f3e78e00ULL, },
			{ 0x0030a1800000ULL, 0x1c79f3c70000ULL, 0x3cfbf7ef9e00ULL, },
			{ 0x006143000000ULL, 0x38f3e78e0000ULL, 0x3cfbffdf3c00ULL, },
			{ 0x00c286000000ULL, 0x30e3cf1c0000ULL, 0x38f3ef9e3800ULL, },
			{ 0x00810c000000ULL, 0x20c38e180000ULL, 0x30e3cf1c3000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x041030000000ULL, 0x0c3870e00000ULL, 0x1c78f1e38000ULL, },
			{ 0x0c2860000000ULL, 0x1c78f1c00000ULL, 0x3cf9f3e78000ULL, },
			{ 0x1850c0000000ULL, 0x3cf9e3800000ULL, 0x3cfbf7cf0000ULL, },
			{ 0x30a180000000ULL, 0x38f3c7000000ULL, 0x3cfbef9e0000ULL, },
			{ 0x204300000000ULL, 0x30e38e000000ULL, 0x38f3cf1c0000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x081800000000ULL, 0x1c3870000000ULL, 0x3c78f1e00000ULL, },
			{ 0x143000000000ULL, 0x3c78e0000000ULL, 0x3cf9f3c00000ULL, },
			{ 0x286000000000ULL, 0x3cf1c0000000ULL, 0x3cfbe7800000ULL, },
			{ 0x10c000000000ULL, 0x38e380000000ULL, 0x3cf3cf000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
		};
		
		int x = index / 7, y = index % 7;
		
		static const int dx[6] = {1, 1, 0,-1,-1, 0};
		static const int dy[6] = {0,-1,-1, 0, 1, 1};
		
		BB mask = seeding_mask[index][size - 1] & usable & (~us) & (~op);
		
		for(int dir = 0; dir < 6; dir++) {
			int i = x + dx[dir], j = y + dy[dir];
			while(0 <= i && i < 7 && 0 <= j && j < 7) {
				mask &= ~(1ULL << (i * 7 + j));
				i += dx[dir];
				j += dy[dir];
			}
		}
		
		cerr << "best seed: ";
		print(mask);
		cerr << '\n';
		
		return lsb(mask);
	}
	
	static inline void read_board() {
		int numberOfCells; 
		cin >> numberOfCells; cin.ignore();
		assert(numberOfCells == 37);
		
		for(int i = 0; i < numberOfCells; i++) {
			int index;
			int cell_richness;
			int neigh0;
			int neigh1;
			int neigh2;
			int neigh3;
			int neigh4;
			int neigh5;
			cin >> index >> cell_richness >> neigh0 >> neigh1 >> neigh2 >> neigh3 >> neigh4 >> neigh5; cin.ignore();
			
			if(cell_richness > 0) {
				usable |= 1ULL << to_bitboard[index];
			}
			richness[to_bitboard[index]] = cell_richness;
		}
		
	}
	
	inline void read_state() {
		cin >> day; cin.ignore();
		day_mod6 = day % 6;
		cin >> nutrients; cin.ignore();
		cin >> suns >> score; cin.ignore();
		int opp_sun, opp_score, op_is_waiting;
		cin >> opp_sun >> opp_score >> op_is_waiting; cin.ignore();
		
		int numberOfTrees; cin >> numberOfTrees; cin.ignore();
		us = 0ULL, op = 0ULL;
		tree[0] = tree[1] = tree[2] = tree[3] = 0ULL;
		dormant = 0ULL;
		
		for(int i = 0; i < numberOfTrees; i++) {
			int cell_id;
			int size;
			bool is_mine;
			bool is_dormant;
			cin >> cell_id >> size >> is_mine >> is_dormant; cin.ignore();
			
			tree[size] |= 1ULL << to_bitboard[cell_id];
			if(is_mine) {
				us |= 1ULL << to_bitboard[cell_id];
				if(big_tree_index == -1) {
					big_tree_index = cell_id;
				}
			} else {
				op |= 1ULL << to_bitboard[cell_id];
			}
			
			if(is_dormant) {
				dormant |= 1ULL << to_bitboard[cell_id];
			}
		}
		
		turns = 0;
	}
	
	inline void get_suns() {
		
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
		
		if(1 <= day_mod6 && day_mod6 <= 3) {
			
			if(tree[SIZE3]) {
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][0]) >> (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][1]) >> (2 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][2]) >> (3 * shift_dir[day_mod6]);
				suns += 3 * pop_cnt(tree[SIZE3] & (~shadow) & us);
			}
			
			if(tree[SIZE2]) {
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][0]) >> (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][1]) >> (2 * shift_dir[day_mod6]);
				suns += 2 * pop_cnt(tree[SIZE2] & (~shadow) & us);
			}
			
			if(tree[SIZE1]) {
				shadow |= (tree[SIZE1] & shift_mask[day_mod6][0]) >> (1 * shift_dir[day_mod6]);
				suns += 1 * pop_cnt(tree[SIZE1] & (~shadow) & us);
			}
			
		} else {
			
			if(tree[SIZE3]) {
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][0]) << (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][1]) << (2 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE3] & shift_mask[day_mod6][2]) << (3 * shift_dir[day_mod6]);
				suns += 3 * pop_cnt(tree[SIZE3] & (~shadow) & us);
			}
			
			if(tree[SIZE2]) {
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][0]) << (1 * shift_dir[day_mod6]);
				shadow |= (tree[SIZE2] & shift_mask[day_mod6][1]) << (2 * shift_dir[day_mod6]);
				suns += 2 * pop_cnt(tree[SIZE2] & (~shadow) & us);
			}
			
			if(tree[SIZE1]) {
				shadow |= (tree[SIZE1] & shift_mask[day_mod6][0]) << (1 * shift_dir[day_mod6]);
				suns += 1 * pop_cnt(tree[SIZE1] & (~shadow) & us);
			}
			
		}
	}
	
	inline void move(const GrowAction &action) {
		static const int grow_cost[3] = {1, 3, 7};
		
		#ifdef LOCAL
		assert(action.tree_sz < 3);
		#endif
		
		suns -= grow_cost[action.tree_sz] + pop_cnt(us & tree[action.tree_sz + 1]);
		
		#ifdef LOCAL
		assert(suns >= 0);
		#endif
		
		// delete old tree
		tree[action.tree_sz + 0] &= ~(1ULL << action.tree_id);
		
		// make new bigger
		tree[action.tree_sz + 1] |= 1ULL << action.tree_id;
		
		// tree is now dormant
		dormant |= 1ULL << action.tree_id;
		
		turns++;
	}
	
	inline void move(const SeedAction &action) {
		suns -= pop_cnt(us & tree[SEED]);
		
		#ifdef LOCAL
		assert(suns >= 0);
		assert((~us) & (1ULL << action.seed_id));
		assert((~op) & (1ULL << action.seed_id));
		#endif
		
		  dormant  |= 1ULL << action.seed_id;
		     us    |= 1ULL << action.seed_id;
		tree[SEED] |= 1ULL << action.seed_id;
		
		turns++;
	}
	
	inline void move(const CompleteAction &action) {
		suns -= 4;
		#ifdef LOCAL
		assert(suns >= 0);
		assert(tree[SIZE3] & (1ULL << action.tree_id));
		#endif
		
		tree[SIZE3] &= ~(1ULL << action.tree_id);
		    us      &= ~(1ULL << action.tree_id);
		
		score += nutrients + richness[action.tree_id];
		nutrients--;
		if(nutrients < 0)	nutrients = 0;
		
		turns++;
	}
	
	inline void move(const WaitAction &action) {
		dormant = 0ULL;
		day++;
		day_mod6++;
		if(day_mod6 == 6)	day_mod6 = 0;
		get_suns();
		
		turns = 0;
	}
	
	inline GrowAction* get_grow_actions(GrowAction *action_list) {
		static const int grow_cost[3] = {1, 3, 7};
		
		for(int tree_size = 0; tree_size < 3; tree_size++) {
			if(pop_cnt(us & tree[tree_size + 1]) > 5)	continue;
			
			if(grow_cost[tree_size] + pop_cnt(us & tree[tree_size + 1]) <= suns) {
				BB trees = tree[tree_size] & us & (~dormant);
				while(trees) {
					*action_list++ = GrowAction(tree_size, pop_lsb(trees));
				}
			}
		}
		
		return action_list;
	}
	
	inline SeedAction* get_seed_actions(SeedAction *action_list) {
		static constexpr BB seeding_mask[49][3] = {
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000010808ULL, 0x000000c51808ULL, },
			{ 0x000000000000ULL, 0x000000021210ULL, 0x0000018ab210ULL, },
			{ 0x000000000000ULL, 0x000000042420ULL, 0x000003152620ULL, },
			{ 0x000000000000ULL, 0x000000080840ULL, 0x0000060a0c40ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000420210ULL, 0x000031460230ULL, },
			{ 0x000000000000ULL, 0x000000848420ULL, 0x000062ac8460ULL, },
			{ 0x000000000000ULL, 0x000001090848ULL, 0x0000c5598848ULL, },
			{ 0x000000000000ULL, 0x000002121010ULL, 0x00018a931018ULL, },
			{ 0x000000000000ULL, 0x000004042020ULL, 0x000305062030ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000010808400ULL, 0x000851808c10ULL, },
			{ 0x000000000000ULL, 0x000021210808ULL, 0x0018a3211828ULL, },
			{ 0x000000000000ULL, 0x000042421210ULL, 0x003156623250ULL, },
			{ 0x000000000000ULL, 0x000084842420ULL, 0x0062acc42628ULL, },
			{ 0x000000000000ULL, 0x000109080840ULL, 0x00c149880c50ULL, },
			{ 0x000000000000ULL, 0x000202101000ULL, 0x008283101820ULL, },
			{ 0x000000000000ULL, 0x000020210000ULL, 0x001060230400ULL, },
			{ 0x000000000000ULL, 0x000840420200ULL, 0x0428c0460a08ULL, },
			{ 0x000000000000ULL, 0x001090848400ULL, 0x0c51908c9418ULL, },
			{ 0x000000000000ULL, 0x002121090800ULL, 0x18ab3119aa30ULL, },
			{ 0x000000000000ULL, 0x004242121000ULL, 0x305262131460ULL, },
			{ 0x000000000000ULL, 0x008084042000ULL, 0x20a0c4062840ULL, },
			{ 0x000000000000ULL, 0x000108080000ULL, 0x0041880c1000ULL, },
			{ 0x000000000000ULL, 0x001010808000ULL, 0x083011828200ULL, },
			{ 0x000000000000ULL, 0x042021210000ULL, 0x146023250600ULL, },
			{ 0x000000000000ULL, 0x084842420000ULL, 0x28c8466a8c00ULL, },
			{ 0x000000000000ULL, 0x109084840000ULL, 0x14988cd51800ULL, },
			{ 0x000000000000ULL, 0x202109080000ULL, 0x2831098a3000ULL, },
			{ 0x000000000000ULL, 0x004202100000ULL, 0x106203142000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x080840400000ULL, 0x1808c1418000ULL, },
			{ 0x000000000000ULL, 0x101090800000ULL, 0x301192a30000ULL, },
			{ 0x000000000000ULL, 0x242121000000ULL, 0x242335460000ULL, },
			{ 0x000000000000ULL, 0x084242000000ULL, 0x0c426a8c0000ULL, },
			{ 0x000000000000ULL, 0x108084000000ULL, 0x1880c5180000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x042020000000ULL, 0x0460a0c00000ULL, },
			{ 0x000000000000ULL, 0x084840000000ULL, 0x08c951800000ULL, },
			{ 0x000000000000ULL, 0x109080000000ULL, 0x109aa3000000ULL, },
			{ 0x000000000000ULL, 0x202100000000ULL, 0x203146000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
			{ 0x000000000000ULL, 0x000000000000ULL, 0x000000000000ULL, },
		};
		
		if(pop_cnt(us & tree[SEED]) <= suns) {
			
			for(int tree_size = 3; tree_size > 0; tree_size--) {
				
				BB trees = tree[tree_size] & us & ~(dormant);
				
				while(trees) {
					Index pos = pop_lsb(trees);
					
					BB seeds = seeding_mask[pos][tree_size - 1] & usable & (~us) & (~op);
					
					while(seeds) {
						Index seed = pop_lsb(seeds);
						
						*action_list++ = SeedAction(pos, seed);
					}
				}
				
			}
			
		}
		
		return action_list;
	}
	
	inline CompleteAction* get_complete_actions(CompleteAction *action_list) {
		if(suns >= 4) {
			BB trees = tree[SIZE3] & us & (~dormant);
			
			while(trees) {
				*action_list++ = CompleteAction(pop_lsb(trees));
			}
		}
		
		return action_list;
	}
	
	inline WaitAction* get_wait_actions(WaitAction *action_list) {
		//if(turns == 0 && suns > 15)	return action_list;
		
		*action_list++ = WaitAction();
		return action_list;
	}
	
	vector<string> all_actions() {
		vector<string> actions;
		
		{
			static GrowAction grow[50];
			GrowAction* end = get_grow_actions(grow);
			for(GrowAction *now = grow; now != end; now++) {
				actions.push_back(now->get_name());
			}
		}
		
		{
			static SeedAction grow[50 * 50];
			SeedAction* end = get_seed_actions(grow);
			for(SeedAction *now = grow; now != end; now++) {
				actions.push_back(now->get_name());
			}
		}
		
		{
			CompleteAction grow[50];
			CompleteAction* end = get_complete_actions(grow);
			for(CompleteAction *now = grow; now != end; now++) {
				actions.push_back(now->get_name());
			}
		}
		
		{
			WaitAction grow[50];
			WaitAction* end = get_wait_actions(grow);
			for(WaitAction *now = grow; now != end; now++) {
				actions.push_back(now->get_name());
			}
		}
		
		return actions;
	}
	
	inline bool is_terminal() const {
		return day == 24;
	}
	
	inline void debug() const {
		#define PRINT_ROW(x,y) for(int i = x; i <= y; i++) { \
									if((~usable) & (1ULL << i))			cerr << 'x'; \
									else if(tree[SEED] & (1ULL << i))	cerr << 's'; \
									else if(tree[SIZE1] & (1ULL << i))	cerr << 't'; \
									else if(tree[SIZE2] & (1ULL << i))	cerr << 'T'; \
									else if(tree[SIZE3] & (1ULL << i))	cerr << 'Y'; \
									else								cerr << '-'; \
									cerr << ' '; \
								} cerr << '\n'; \
								
		cerr << "   "; PRINT_ROW(3, 6);
		cerr << "  "; PRINT_ROW(9, 13);
		cerr << " "; PRINT_ROW(15, 20);
		cerr << ""; PRINT_ROW(21, 27);
		cerr << " "; PRINT_ROW(28, 33);
		cerr << "  "; PRINT_ROW(35, 39);
		cerr << "   "; PRINT_ROW(42, 45);
		
		#undef PRINT_ROW
		
		cerr << '\n';
	}
};

/*
   s x - t 
  T t s x - 
 x Y t - - x 
- - s - - - - 
 x - - - - x 
  s x - - t 
   t s x -
   
x = richness 0
- = empty
s = seed
t = size 1
T = size 2
Y = size 3
*/
