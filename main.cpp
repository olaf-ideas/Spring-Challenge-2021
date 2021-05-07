#ifndef LOCAL
#pragma GCC optimize("Ofast","unroll-loops","omit-frame-pointer","inline")
#pragma GCC option("arch=native","tune=native","no-zero-upper")
#pragma GCC target("avx2","popcnt","rdrnd","bmi2")
#endif

#include <bits/stdc++.h>

using namespace std;

#include "state.h"
#include "timer.h"

#define MAX_DEPTH 100
#define BEAM_WIDTH 3000
#define TIME_LIMIT 100

void shadow_mask_generation() {
	static const int dx[6] = {1, 1, 0,-1,-1, 0};
	static const int dy[6] = {0,-1,-1, 0, 1, 1};
	
	for(int p = 0; p < 49; p++) {
		int x = p / 7, y = p % 7;
		
		cerr << "{ ";
		for(int max_len = 1; max_len <= 3; max_len++) {
			BB mask = 0ULL;
			for(int dir = 0; dir < 6; dir++) {
				int i = x + dx[dir], j = y + dy[dir];
				for(int len = 0; len < max_len && (0 <= i && i < 7 && 0 <= j && j < 7); len++) {
					mask |= (1ULL << (i * 7 + j));
					i += dx[dir];
					j += dy[dir];
				}
			}
			cerr << '\n';
			print(mask & board);
			cerr << '\n';
			//print_hex(mask & board);
		}
		cerr << "},\n";
	}
}

int main() {
	//shadow_mask_generation();
	//return 0;
	State::read_board();
	
	vector<State> beam_states[MAX_DEPTH];
	
	Timer timer;
	
	for(int turn = 0; ; turn++) {
		State current_state;
		
		current_state.read_state();
		timer.start();
		
		current_state.debug();
		
		//vector<string> actions = current_state.all_actions();
		vector<string> engine_actions;
		
		int numberOfPossibleActions;
		cin >> numberOfPossibleActions; cin.ignore();
		
		for(int i = 0; i < numberOfPossibleActions; i++) {
			string action;
			getline(cin, action);
			engine_actions.push_back(action);
		}
		
		if(turn == 0) {
			cout << "WAIT\n";
			continue;
		}
		
		if(turn == 1) {
			cout << "GROW " << big_tree_index << "\n";
			continue;
		}
		
		if(turn == 2) {
			cout << "WAIT\n";
			continue;
		}
		
		if(turn == 3) {
			int bit_index = to_bitboard[big_tree_index];
			
			cout << "SEED " << big_tree_index << ' ' << to_game_id[current_state.get_best_seed(bit_index, 2)] << '\n';
			continue;
		}
		
		if(turn == 4) {
			cout << "WAIT\n";
			continue;
		}
		
		if(turn == 5) {
			cout << "GROW " << big_tree_index << '\n';
			continue;
		}
		
		if(turn == 6) {
			cout << "WAIT\n";
			continue;
		}
		
		if(turn < 9 && usable & (1ULL << to_bitboard[0])) {
			if((~current_state.us) & (1ULL << to_bitboard[0])) {
				cout << "SEED " << big_tree_index << ' ' << 0 << '\n';
				continue;
			}
		} else if(turn == 7) {
			int bit_index = to_bitboard[big_tree_index];
			
			cout << "SEED " << big_tree_index << ' ' << to_game_id[current_state.get_best_seed(bit_index, 3)] << '\n';
			continue;
		}
		
		if(current_state.day == 23) {
			
			if(current_state.suns < 4) {
				cout << "WAIT\n";
			} else {
				BB trees = current_state.tree[SIZE3] & current_state.us;
				
				int best_index = -1;
				while(trees) {
					Index pos = pop_lsb(trees);
					if(best_index == -1 || richness[best_index] < richness[pos]) {
						best_index = pos;
					}
				}
				
				if(best_index == -1) {
					cout << "WAIT\n";
				} else {
					cout << "COMPLETE " << to_game_id[best_index] << '\n';
				}
			}
			
			continue;
		}
		
		//sort(actions.begin(), actions.end());
		//sort(engine_actions.begin(), engine_actions.end());
		
		//cerr << actions.size() << ' ' << engine_actions.size() << ' ' << numberOfPossibleActions << '\n';
		
		//for(int i = 0; i < numberOfPossibleActions; i++) {
			//cerr << actions[i] << " | " << engine_actions[i] << '\n';
		//}
		
		//assert(actions == engine_actions);
		
		cerr << "starting...\n";
		for(int depth = 0; depth < MAX_DEPTH; depth++) {
			beam_states[depth].clear();
		}
		
		beam_states[0].push_back(current_state);
		
		vector<GrowAction> first_grow_actions;
		vector<SeedAction> first_seed_actions;
		vector<CompleteAction> first_complete_actions;
		vector<WaitAction> first_wait_actions;
		
		int depth = 0;
		for(; timer.get_elapsed() < TIME_LIMIT - 5 && depth < MAX_DEPTH - 1; depth++) {
			cerr << "depth: " << depth << ' ' << beam_states[depth].size() << '\n';
			
			for(State &state : beam_states[depth]) {
				state.evaluate();
			}
			
			if(beam_states[depth].size() > BEAM_WIDTH) {
				nth_element(beam_states[depth].begin(), beam_states[depth].begin() + BEAM_WIDTH, beam_states[depth].end());
				while(beam_states[depth].size() > BEAM_WIDTH) {
					beam_states[depth].pop_back();
				}
			}
			
			for(State &state : beam_states[depth]) {
				if(timer.get_elapsed() >= TIME_LIMIT - 5) {
					depth--;
					break;
				}
				
				if(state.is_terminal()) {
					beam_states[depth + 1].push_back(state);
					continue;
				}
				
				{ // GROW
					static GrowAction grow_actions[50];
					GrowAction *end = state.get_grow_actions(grow_actions);
					
					for(GrowAction *now = grow_actions; now != end; now++) {
						State new_state = state;
						
						if(depth == 0) {
							new_state.first_action_type = GROW;
							new_state.first_action_index = first_grow_actions.size();
							
							first_grow_actions.push_back(*now);
						}
						
						new_state.move(*now);
						beam_states[depth + 1].push_back(new_state);
					}
					
					if(state.day == 23 && end == grow_actions) {
						beam_states[depth + 1].push_back(state);
					}
				}
				
				if(state.day < 23) {
					{ // SEED
						static SeedAction seed_actions[50 * 50];
						SeedAction *end = state.get_seed_actions(seed_actions);
						
						for(SeedAction *now = seed_actions; now != end; now++) {
							State new_state = state;
							
							if(depth == 0) {
								new_state.first_action_type = SEED_action;
								new_state.first_action_index = first_seed_actions.size();
								
								first_seed_actions.push_back(*now);
							}
							
							new_state.move(*now);
							beam_states[depth + 1].push_back(new_state);
						}
					}
					
					{ // COMPLETE
						static CompleteAction complete_actions[50];
						CompleteAction *end = state.get_complete_actions(complete_actions);
						
						for(CompleteAction *now = complete_actions; now != end; now++) {
							State new_state = state;
							
							if(depth == 0) {
								new_state.first_action_type = COMPLETE;
								new_state.first_action_index = first_complete_actions.size();
								
								first_complete_actions.push_back(*now);
							}
							
							new_state.move(*now);
							beam_states[depth + 1].push_back(new_state);
						}
					}
					
					{ // WAIT
						static WaitAction wait_actions[2];
						WaitAction *end = state.get_wait_actions(wait_actions);
						
						for(WaitAction *now = wait_actions; now != end; now++) {
							State new_state = state;
							
							if(depth == 0) {
								new_state.first_action_type = WAIT;
								new_state.first_action_index = first_wait_actions.size();
								
								first_wait_actions.push_back(*now);
							}
							
							new_state.move(*now);
							
							beam_states[depth + 1].push_back(new_state);
						}
					}
				}
			}
		}
		cerr << depth << '\n';
		State best_state = *min_element(beam_states[depth].begin(), beam_states[depth].end());
		
		if(best_state.first_action_type == GROW) {
			cout << first_grow_actions[best_state.first_action_index].get_name() << '\n';
		}
		
		else if(best_state.first_action_type == SEED_action) {
			cout << first_seed_actions[best_state.first_action_index].get_name() << '\n';
		}
		
		else if(best_state.first_action_type == COMPLETE) {
			cout << first_complete_actions[best_state.first_action_index].get_name() << '\n';
		}
		
		else if(best_state.first_action_type == WAIT) {
			cout << first_wait_actions[best_state.first_action_index].get_name() << '\n';
		}
		
		#ifdef LOCAL
		return 0;
		#endif
		//cout << actions[rand() % actions.size()] << '\n';
	}
	
	return 0;
}
