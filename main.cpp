#include <bits/stdc++.h>

using namespace std;

#include "state.h"

int dx[6] = {1, 1, 0,-1,-1,0};
int dy[6] = {0,-1,-1, 0, 1,1};

int main() {
	State::read_board();
	
	for(int turn = 0; ; turn++) {
		State state;
		
		state.read_state();
		
		state.debug();
		
		return 0;
	}
	
	return 0;
}
