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

struct Action {};

struct GrowAction : Action {
	Index tree_sz;
	Index tree_id;
	
	GrowAction(Index _tree_sz, Index _tree_id) : tree_sz(_tree_sz), tree_id(_tree_id) { }
	
	inline string get_name() const {
		return "GROW " + to_string(to_game_id[tree_id]);
	}
};

struct SeedAction : Action {
	Index tree_id;
	Index seed_id;
	
	SeedAction(Index _tree_id, Index _seed_id) : tree_id(_tree_id), seed_id(_seed_id) { }
	
	inline string get_name() const {
		return "SEED " + to_string(to_game_id[tree_id]) + " " + to_string(to_game_id[seed_id]);
	}
};

struct CompleteAction : Action {
	Index tree_id;
	
	CompleteAction(Index _tree_id) : tree_id(_tree_id) { }
	
	inline string get_name() const {
		return "COMPLETE " + to_string(to_game_id[tree_id]);
	}
};

struct WaitAction : Action {
	
	inline string get_name() const {
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

struct State {
	BB us, op;
	BB tree[TREE_NB];
	BB dormant;
	short suns;
	short day;
	short day_mod6;
	short nutrients;
	short score;
	
	Action first_move;
	
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
				usable |= 1ULL << to_bitboard[index];`
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
			} else {
				op |= 1ULL << to_bitboard[cell_id];
			}
			
			if(is_dormant) {
				dormant |= 1ULL << to_bitboard[cell_id];
			}
		}
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
		
		assert(action.tree_sz < 3);
		
		suns -= grow_cost[action.tree_sz] + pop_cnt(tree[action.tree_sz + 1]);
		
		assert(suns >= 0);
		
		// delete old tree
		tree[action.tree_sz + 0] &= ~(1ULL << action.tree_id);
		
		// make new bigger
		tree[action.tree_sz + 1] |= 1ULL << action.tree_id;
		
		// tree is now dormant
		dormant |= 1ULL << action.tree_id;
	}
	
	inline void move(const SeedAction &action) {
		suns -= pop_cnt(tree[SEED]);
		
		assert(suns >= 0);
		assert((~us) & (1ULL << action.seed_id));
		assert((~op) & (1ULL << action.seed_id));
		
		  dormant  |= 1ULL << action.seed_id;
		     us    |= 1ULL << action.seed_id;
		tree[SEED] |= 1ULL << action.seed_id;
	}
	
	inline void move(const CompleteAction &action) {
		suns -= 4;
		assert(suns >= 0);
		assert(tree[SIZE3] & (1ULL << action.tree_id));
		
		tree[SIZE3] &= ~(1ULL << action.tree_id);
		score += nutrients + richness[action.tree_id];
		nutrients--;
		if(nutrients < 0)	nutrients = 0;
	}
	
	inline void move(const WaitAction &action) {
		dormant = 0ULL;
		day++;
		day_mod6++;
		if(day_mod6 == 6)	day_mod6 = 0;
		get_suns();
	}
	
	inline Action* get_grow_actions(Action *action_list) {
		static const int grow_cost[3] = {1, 3, 7};

		for(int tree_size = 0; tree_size < 3; tree_size++) {
			if(grow_cost[tree_size] + pop_cnt(tree[tree_size + 1]) < sun) {	
				BB trees = tree[tree_size] & us & (~dormant);
				while(trees) {
					action_list++ = GrowAction(tree_size, pop_lsb(trees));
				}
			}
		}
		
		return action_list;
	}
	
	inline Action* get_seed_actions(Action *action_list) {
		if(pop_cnt(tree[SEED]) < sun) {
			
			for(int tree_size = 3; tree_size > 0; tree_size--) {
				
				BB trees = tree[tree_size] & us & ~(dormant);
				
				while(trees) {
					Index pos = pop_lsb(trees);
					
					BB seeds = seeding_mask[pos] & usable & (~us) & (~op);
					
					while(seeds) {
						Index seed = pop_lsb(seeds);
						
						action_list++ = SeedAction(pos, seed);
					}
				}
				
			}
			
		}
		
		return action_list;
	}
	
	inline Action* get_complete_actions(Action *action_list) {
		if(suns >= 4) {
			BB trees = tree[SIZE3] & us & (~dormant);
			
			while(trees) {
				action_list++ = CompleteAction(pop_lsb(trees));
			}
		}
		
		return action_list;
	}
	
	inline Action* get_wait_actions(Action *action_list) {
		action_list++ = WaitAction;
	}
	
	inline Action* get_all_actions(Action* action_list) {
		return get_grow_actions(get_seed_actions(get_complete_actions(get_wait_actions(action_list))));
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
