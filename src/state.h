//
//  state.h
//  Game-Tree
//
//  Created by 张家豪 on 2018/11/18.
//  Copyright © 2018 张家豪. All rights reserved.
//

#ifndef state_h
#define state_h
#include <bitset>
#include <iostream>
#include <vector>

const int row_size = 8;
const int board_size = row_size * row_size;
typedef std::bitset<board_size> state_t;
typedef std::pair<int, int> pos_t;

enum turn{black_turn = 1, white_turn};
enum direction{Up, Down, Left, Right, UpLeft, UpRight, DownLeft, DownRight};

class State{
private:
    state_t black;
    state_t white;
    state_t valid_pos;
    turn cur_turn;
    
public:
    State();
    friend std::ostream& operator << (std::ostream& , State);
    friend std::ostream& operator << (std::ostream& , state_t);
    state_t shift(state_t , direction);
    state_t getValid();
    std::vector<pos_t> getValids();
    void action(int x, int y);
    
};

void ini_base();
#endif /* state_h */
