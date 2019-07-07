//
//  main.cpp
//  Game-Tree
//
//  Created by 张家豪 on 2018/11/14.
//  Copyright © 2018 张家豪. All rights reserved.
//

#include <iostream>
#include <vector>
#include <bitset>
#include <cmath>
#include <unistd.h>
#include <string>
#include <queue>
#define Block_size 8


//#include <iostream>
//
//using namespace std;
//
//int MyFunc1(int p,int q){return p%q;}
//int MyFunc2(int p,int q){return p/q;}
//int MyFunc3(int p,int q){return p-q;}
//
//typedef int (*PMF)(int,int);
//
//int main()
//{
//    PMF ArrPMF[3]={MyFunc1,MyFunc2,MyFunc3};
//
//    int nRes;
//    nRes=ArrPMF[1](7,2);    cout<<nRes<<endl;
//    nRes=1[ArrPMF](7,2);    cout<<nRes<<endl;
//
//    return 0;
//}

const int row_size = 8;
const int board_size = row_size * row_size;
typedef std::bitset<board_size> state_t;
typedef std::pair<int, int> pos_t;
state_t left_shift_base;
state_t right_shift_base;
state_t up_shift_base;
enum turn{black_turn = 1, white_turn};
enum direction{Up, Down, Left, Right, UpLeft, UpRight, DownLeft, DownRight};
int heur[board_size] = {90,-20,10,10,10,10,-20,90,
                        -20,-20,1, 1, 1, 1, -20, -20,
                        10, 1, 1, 1, 1, 1,1, 10,
                        10, 1, 1, 1, 1, 1,1, 10,
                        10, 1, 1, 1, 1, 1,1, 10,
                        10, 1, 1, 1, 1, 1,1, 10,
                        -20,-20,1, 1, 1, 1, -20, -20,
                        90,-20,10,10,10,10,-20,90
};

state_t extend(int, int );

void ini_base(){
    left_shift_base = state_t(0);
    right_shift_base = state_t(0);
    up_shift_base = state_t(0);
    
    state_t left_base = left_shift_base;
    state_t right_base = state_t(1);
    state_t up_base = up_shift_base;
    
    left_base[board_size-1] = 1;
    right_base[0] = 1;
    up_base[board_size-1] = 1;
    
    for(int i = 0; i < row_size; i++){
        left_shift_base |= left_base;
        right_shift_base |= right_base;
        up_shift_base |= up_base;
        left_base >>= row_size;
        right_base <<= row_size;
        up_base >>= 1;
    }
    left_shift_base = ~left_shift_base;
    right_shift_base = ~right_shift_base;
    up_shift_base = ~up_shift_base;
}

state_t shift(state_t state, direction dir){
    if(dir == Up){
        state <<= row_size;
    }
    else if(dir == Down){
        state >>= row_size;
    }
    else if(dir == Left){
        state &= left_shift_base;
        state <<= 1;
    }
    else if(dir == Right){
        state &= right_shift_base;
        state >>= 1;
    }
    else if(dir == UpLeft){
        state &= left_shift_base;
        state <<= 1;
        state <<= row_size;
    }
    else if(dir == UpRight){
        state &= right_shift_base;
        state >>= 1;
        state <<= row_size;
    }
    else if(dir == DownLeft){
        state &= left_shift_base;
        state <<= 1;
        state >>= row_size;
    }
    else if(dir == DownRight){
        state &= right_shift_base;
        state >>= 1;
        state >>= row_size;
    }
    return state;
}

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
    bool action(int x, int y);
    int score();
    int score_1();
    double score_2();
    bool isFinish();
    turn getPlayer();
    std::string getWinner();
};

std::ostream & operator << (std::ostream& out, state_t state){
    out << "  |";
    for(int i = 0; i < row_size; i++)
        out << char('1' + i) << " |";
    std::string white = "🌝";
    std::string black = "🌚";
    char row = '1';
    for(int i = board_size-1; i >= 0; i--){
        if(!((i+1) & row_size-1))
            out << std::endl << row++ << " |";
        if(state[i])
            out << " O";
        else
            out << "  ";
        out << "|";
    }
    return out;
}

std::ostream & operator << (std::ostream& out, State state){
    out << "  |";
    for(int i = 0; i < row_size; i++)
        out << char('1' + i) << " |";
    std::string white = "🌝";
    std::string black = "🌚";
    char row = '1';
    for(int i = board_size-1; i >= 0; i--){
        if(!((i+1) & row_size-1))
            out << std::endl << row++ << " |";
        if(state.black[i])
            out << black;
        else if(state.white[i])
            out << white;
        else if(state.valid_pos[i] == 1)
            out << "O ";
        else
            out << "  ";
        out << "|";
    }
    return out;
}

std::string State::getWinner(){
    std::string res;
    if(isFinish())
        res += black.count() > white.count()? "black win!" : "white win!";
    res += "比分是 " + std::to_string(black.count()) + " : " + std::to_string(white.count());
    return res;
}



double State::score_2(){
    double score = 0;
    double round = black.count() + white.count() - 3;
    
    //地势值
    double score1 = 0;
    for(int i = 0; i < board_size; i++)
        score1 += black[i] * heur[i];
    for(int i = 0; i < board_size; i++)
        score1 -= white[i] * heur[i];
    double a1 = (64.0 - round)/64.0;
    
    //行动力
    double score2 = 0;
    State tmp = *this;
    score2 += tmp.cur_turn == black_turn? tmp.valid_pos.count() : -double(tmp.valid_pos.count()) ;
    tmp.cur_turn = tmp.cur_turn == black_turn? white_turn : black_turn; //转换持方
    tmp.getValid(); // 更新合法位置
    score2 += tmp.cur_turn == black_turn? tmp.valid_pos.count():-double(tmp.valid_pos.count());
    double a2 = (64.0 - round)/64.0;
    
    //黑子数量白子数量
    double score3 = 0;
    score3 += double(black.count()) - double(white.count());
    double a3 =  round/64.0 * 10;
    
    //稳定子数目
    double score4 = 0;
    state_t onBoard = black | white;
    for(int i = row_size-1; i >= 0; i--){
        for(int j = row_size-1; j >= 0; j--){
            if(!onBoard[i*row_size+j]){}
            else{
                state_t base1 = extend(i, j);
                if((base1 & onBoard) == base1)//八个方向都满了是稳定子的必要条件
                    score += black[i*row_size+j] == 1? 1 : -1;
            }
        }
    }
    double a4 = 1 / ((round - 32) * (round - 32) + 0.1);
    
    score = a1*score1 + a2*score2 + a3*score3 + a4*score4;
    return score;
}

int State::score_1(){
    int score = 0;
    State tmp = *this;
    score += tmp.cur_turn == black_turn? tmp.valid_pos.count() : -tmp.valid_pos.count();
    tmp.cur_turn = tmp.cur_turn == black_turn? white_turn : black_turn;
    tmp.getValid();
    score += tmp.cur_turn == black_turn? tmp.valid_pos.count() : -tmp.valid_pos.count();
    return score;
}

int State::score(){
    int score = 0;
    for(int i = 0; i < board_size; i++)
        score += black[i] * heur[i];
    for(int i = 0; i < board_size; i++)
        score -= white[i] * heur[i];
    State tmp = *this;
    score += tmp.cur_turn == black_turn? tmp.valid_pos.count() * 10: -tmp.valid_pos.count() * 10;
    tmp.cur_turn = tmp.cur_turn == black_turn? white_turn : black_turn;
    tmp.getValid();
    score += tmp.cur_turn == black_turn? tmp.valid_pos.count()*10 : -tmp.valid_pos.count()*10;
    return score;
//    int action_power = (cur_turn == black_turn)? valid_pos.count() : - valid_pos.count();
//    return black.count() - white.count() + action_power;//黑减去白加行动力
}

turn State::getPlayer(){
    return cur_turn;
}

bool State::isFinish(){
    return valid_pos.none();
}

std::vector<pos_t> State::getValids(){
    std::vector<pos_t> res;
    for(int i = board_size-1; i >= 0; i--){
        if(valid_pos[i]){
            int tmp = board_size-1 - i;
            res.push_back(std::make_pair(tmp / row_size+1, tmp % row_size+1));
        }
    }
    return res;
}

state_t State::shift(state_t state, direction dir){
    if(dir == Up){
        state <<= row_size;
    }
    else if(dir == Down){
        state >>= row_size;
    }
    else if(dir == Left){
        state &= left_shift_base;
        state <<= 1;
    }
    else if(dir == Right){
        state &= right_shift_base;
        state >>= 1;
    }
    else if(dir == UpLeft){
        state &= left_shift_base;
        state <<= 1;
        state <<= row_size;
    }
    else if(dir == UpRight){
        state &= right_shift_base;
        state >>= 1;
        state <<= row_size;
    }
    else if(dir == DownLeft){
        state &= left_shift_base;
        state <<= 1;
        state >>= row_size;
    }
    else if(dir == DownRight){
        state &= right_shift_base;
        state >>= 1;
        state >>= row_size;
    }
    return state;
}

state_t extend(int x, int j){
    state_t res(0);
    res[x*row_size+j] = 1;
    for(direction dir = Up; dir <= DownRight; dir = (direction)(dir+1)){
        state_t tmp(0);
        tmp[x*row_size+j] = 1;
        while(tmp.any()){
            res |= tmp;
            tmp = shift(tmp, dir);
        }
    }
    return res;
}

state_t State::getValid(){
    state_t res(0);
    state_t state_1 = (cur_turn == black_turn)? black : white;
    state_t state_2 = (cur_turn == black_turn)? white : black;
    for(direction dir = Up; dir <= DownRight; dir = (direction)(dir+1)){
        state_t tmp = shift(state_1, dir);
        state_t tmp_2 = tmp & state_2;
        while(tmp_2.any()){
            tmp = shift(tmp_2, dir);
            tmp_2 = tmp & state_2;
            res |= (tmp_2 ^ tmp);
        }
    }
    res ^= res & state_1;
    return res;
}

State::State(){
    black = state_t(0);
    white = state_t(0);
    white[board_size/2-row_size/2-1] = 1;
    black[board_size/2+row_size/2-1] = 1;
    black[board_size/2-row_size/2] = 1;
    white[board_size/2+row_size/2] = 1;
    cur_turn = black_turn;
    valid_pos = getValid();
}

bool State::action(int x, int y){
    state_t act(0);
    act[board_size-(x-1)*row_size-y] = 1;
    if((act & valid_pos).none()){
        std::cout << "乱输尼玛呢!再输一次(x,y)：";
        return false;
    }
    
    state_t state_1 = (cur_turn == black_turn)? black : white;
    state_t state_2 = (cur_turn == black_turn)? white : black;
    for(direction dir = Up; dir <= DownRight; dir = (direction)(dir+1)){
        state_t res(0);
        state_t tmp = shift(act, dir);
        state_t tmp_2 = tmp & state_2;
        while(tmp_2.any()){
            res |= tmp_2;
            tmp = shift(tmp_2, dir);
            tmp_2 = tmp & state_2;
        }
        if(!(tmp & state_1).none()){
            state_1 |= res;
            state_2 ^= res;
        }
    }
    state_1 |= act;
    black = (cur_turn == black_turn)? state_1 : state_2;
    white = (cur_turn == black_turn)? state_2 : state_1;
    cur_turn = (cur_turn == black_turn)? white_turn : black_turn;
    valid_pos = getValid();
    if(valid_pos.none()){//没有得下了
        cur_turn = (cur_turn == black_turn)? white_turn : black_turn;
        valid_pos = getValid();
    }
    return true;
}

std::ostream & operator << (std::ostream& out, std::vector<pos_t> pos){
    for(auto iter = pos.begin(); iter != pos.end(); iter++)
        out << "(" << iter->first << ", " << iter->second << ")" << std::endl;
    return out;
}

std::ostream & operator << (std::ostream& out, pos_t pos){
    out << "(" << pos.first << ", " << pos.second << ")" << std::endl;
    return out;
}

//int minMax(State cur_state, int cur_height, int Alpha, int Beta, int max_height, pos_t& best_pos){
//    if(cur_height == max_height || cur_state.isFinish()){
//        return cur_state.score();
//    }
//    pos_t best = std::make_pair(0, 0);
//    pos_t t = best;
//    int cost = 0;
//    if(cur_state.getPlayer() == black_turn){
//        std::vector<pos_t> actions = cur_state.getValids();
//        cost = INT_MIN + 1;
//        for(auto iter = actions.begin(); iter != actions.end(); iter++){
//            State sub_state = cur_state;
//            sub_state.action(iter->first, iter->second);
//            int tmp = minMax(sub_state, cur_height + 1, Alpha, Beta, max_height, t);
//            best = tmp > cost? std::make_pair(iter->first, iter->second) : best;
//            cost = cost > tmp? cost : tmp;
//            Alpha = cost > Alpha? cost : Alpha;
//            if(Beta <= Alpha)
//                break;
//        }
//    }
//    else if(cur_state.getPlayer() == white_turn){
//        std::vector<pos_t> actions = cur_state.getValids();
//        cost = INT_MAX;
//        for(auto iter = actions.begin(); iter != actions.end(); iter++){
//            State sub_state = cur_state;
//            sub_state.action(iter->first, iter->second);
//            int tmp = minMax(sub_state, cur_height + 1, Alpha, Beta, max_height, t);
//            best = tmp < cost? std::make_pair(iter->first, iter->second) : best;
//            cost = cost < tmp? cost : tmp;
//            Beta = cost < Beta? cost : Beta;
//            if(Beta <= Alpha)
//                break;
//        }
//    }
//    best_pos = best;
//    return cost;
//}

std::priority_queue<double,std::vector<double>,std::greater<double>> a1; //小的在前面
std::priority_queue<double> a2;

double minMax(State cur_state, int cur_height, double Alpha, double Beta, int max_height, pos_t& best_pos){
    if(cur_height == max_height || cur_state.isFinish()){
        double s = cur_state.score_2();
        a1.push(s);
        a2.push(s);
//        std::cout << s << std::endl;
        return s;
    }
    pos_t best = std::make_pair(-1, -1);
    pos_t t = best;
    double cost = 0;
    if(cur_state.getPlayer() == black_turn){
        std::vector<pos_t> actions = cur_state.getValids();
        cost = INT_MIN + 1;
        for(auto iter = actions.begin(); iter != actions.end(); iter++){
            State sub_state = cur_state;
            sub_state.action(iter->first, iter->second);
            double tmp = minMax(sub_state, cur_height + 1, Alpha, Beta, max_height, t);
            best = tmp > cost? std::make_pair(iter->first, iter->second) : best;
            cost = cost > tmp? cost : tmp;
            Alpha = cost > Alpha? cost : Alpha;
            if(Beta <= Alpha)
                break;
        }
    }
    else if(cur_state.getPlayer() == white_turn){
        std::vector<pos_t> actions = cur_state.getValids();
        cost = INT_MAX;
        for(auto iter = actions.begin(); iter != actions.end(); iter++){
            State sub_state = cur_state;
            sub_state.action(iter->first, iter->second);
            double tmp = minMax(sub_state, cur_height + 1, Alpha, Beta, max_height, t);
            best = tmp < cost? std::make_pair(iter->first, iter->second) : best;
            cost = cost < tmp? cost : tmp;
            Beta = cost < Beta? cost : Beta;
            if(Beta <= Alpha)
                break;
        }
    }
//    if(best.first==-1){
//        std::vector<pos_t> actions = cur_state.getValids();
//        best = actions[rand()%actions.size()];
//    }
    best_pos = best;
    return cost;
}

int main(){
    ini_base();
    
    State t;
    int stand;
    std::cout << "请选择你要下的持方(1或0): " ;
    std::cin >> stand;
//    stand = 1;
    turn player_turn = stand == 1? black_turn : white_turn;
    
    std::cout << t << std::endl << std::endl;
    int depth = 5;
    int x,y;
    
    while(1){
        pos_t best_pos;
        if(t.getPlayer() == player_turn){
//            minMax(t, 0, INT_MIN, INT_MAX, depth, best_pos);
//            t.action(best_pos.first, best_pos.second);
//            std::cout << t << std::endl << std::endl;
//            std::cout << "AI_1下的地方是(" << best_pos.first << "," << best_pos.second << ")" << std::endl;
//            if(player_turn == black_turn)
//                std::cout << "heur: " << a2.top() << std::endl;
//            else
//                std::cout << "heur: " << a1.top() << std::endl;
            
//            sleep(2);
            std::cout << "请输入你要下的地方(x,y)：";
            std::cin >> x >> y;
            while(!t.action(x, y)){
                std::cin >> x >> y;
            }
            std::cout << t << std::endl;
            std::cout << "你选择的落子位置是:(" << x << "," << y << ")" << std::endl << std::endl;
            
        }
        else if(t.getPlayer() != player_turn){
            std::priority_queue<double,std::vector<double>,std::greater<double>> a3; //小的在前面
            std::priority_queue<double> a4;
            minMax(t, 0, INT_MIN, INT_MAX, depth, best_pos);
            t.action(best_pos.first, best_pos.second);
            std::cout << t << std::endl << std::endl;
            std::cout << "AI_2下的地方是(" << best_pos.first << "," << best_pos.second << ")" << std::endl;
//            sleep(2);
            if(player_turn == white_turn)
                std::cout << "heur: " << a2.top() << std::endl;
            else
                std::cout << "heur: " << a1.top() << std::endl;
            a1 = a3;
            a2 = a4;
        }
        if(t.isFinish()){
            std::cout << "游戏结束！！！" << std::endl;
            std::cout << t.getWinner() << std::endl;
            break;
        }
    }
}


