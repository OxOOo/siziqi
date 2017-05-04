#include "AIEngine.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>

using namespace std;

const int DX[] = {0, 1, 1, 1};
const int DY[] = {1, 0, 1, -1};

AIEngine::AIEngine(int MAX_M, int MAX_N): MAX_M(MAX_M), MAX_N(MAX_N)
{
    pool_lines = new int*[MAX_M];
    pool_points = new int[MAX_M*MAX_N];
    reloads = 0;
}

AIEngine::~AIEngine()
{
    delete[] pool_lines;
    delete[] pool_points;
}

void AIEngine::reload(const int M, const int N,  const int* _board, const int noX, const int noY)
{
    this->M = M;
    this->N = N;
    board = pool_lines;
	for(int i = 0; i < M; i++){
		board[i] = pool_points + i*N;
		for(int j = 0; j < N; j++){
			board[i][j] = _board[i * N + j];
		}
	}
    board[noX][noY] = BOARD_BLOCK;

    reloads ++;
}

Point AIEngine::getAction()
{
    // char buf[1024];
    // sprintf(buf, "debug%d.txt", reloads);
    // debug = new ofstream(buf);
    debug = new ostringstream();
    *debug << M << " " << N << endl;
    for(int i = 0; i < M; i ++)
    {
        for(int j = 0; j < N; j ++)
            *debug << board[i][j] << " ";
        *debug << endl;
    }

    LL v = alphabeta(1, BOARD_MINE, -INF*2, INF*2);
    *debug << "v : " << v << endl;
    *debug << "choose : " << alpha1_action.first << " " << alpha1_action.second << endl;
    
    debug->flush();
    delete debug;
    return Point(alpha1_action.first, alpha1_action.second);
}

LL AIEngine::evaluate(int lastX, int lastY) // FIXME 待细细考量 =============================
{
    if (isWin(lastX, lastY))
        return board[lastX][lastY] == BOARD_MINE ? INF : -INF;
    
    LL mine = 0, enemy = 0;

    if(huo(BOARD_MINE, 3)) mine = 800;
    else if(chong(BOARD_MINE, 3)) mine = 600;
    else if(huo(BOARD_MINE, 2)) mine = 300;
    else if(chong(BOARD_MINE, 2)) mine = 250;
    else if(huo(BOARD_MINE, 1)) mine = 110;
    else if(chong(BOARD_MINE, 1)) mine = 100;

    if(huo(BOARD_ENEMY, 3)) enemy = 650;
    else if(chong(BOARD_ENEMY, 3)) enemy = 550;
    else if(huo(BOARD_ENEMY, 2)) enemy = 300;
    else if(chong(BOARD_ENEMY, 2)) enemy = 200;
    else if(huo(BOARD_ENEMY, 1)) enemy = 100;
    else if(chong(BOARD_ENEMY, 1)) enemy = 0;
 
    return mine - enemy;
}

bool AIEngine::isWin(int lastX, int lastY)
{
    for(int d = 0; d < 4; d ++)
    {
        int count, nnums;
        straightNums(lastX, lastY, d, count, nnums);
        if (count >= 4) return true;
    }
    return false;
}

void AIEngine::straightNums(int X, int Y, int d, int&count, int&nnums)
{
    count = 1;
    nnums = 0;
    if (!isOK(X, Y) || (board[X][Y] != BOARD_MINE && board[X][Y] != BOARD_ENEMY))
        return;

    int dx = DX[d], dy = DY[d];
    for(int x = X+dx, y = Y+dy; isOK(x, y); x += dx, y += dy)
        if (board[x][y] == board[X][Y])
            count ++;
        else {
            if (board[x][y] == BOARD_NONE)
                nnums ++;
            break;
        }
    for(int x = X-dx, y = Y-dy; isOK(x, y); x -= dx, y -= dy)
        if (board[x][y] == board[X][Y])
            count ++;
        else {
            if (board[x][y] == BOARD_NONE)
                nnums ++;
            break;
        }
}

bool AIEngine::isOK(int x, int y)
{
    return 0 <= x && x < M && 0 <= y && y < N;
}

bool AIEngine::huo(int player, int nums)
{
    for(int i = 0; i < M; i ++)
        for(int j = 0; j < N; j ++)
            if (board[i][j] == player)
            {
                for(int d = 0; d < 4; d ++)
                {
                    int count, nnums;
                    straightNums(i, j, d, count, nnums);
                    if (nnums == 2 && count == nums) return true;
                }
            }
    return false;
}

bool AIEngine::chong(int player, int nums)
{
    for(int i = 0; i < M; i ++)
        for(int j = 0; j < N; j ++)
            if (board[i][j] == player)
            {
                for(int d = 0; d < 4; d ++)
                {
                    int count, nnums;
                    straightNums(i, j, d, count, nnums);
                    if (nnums == 1 && count == nums) return true;
                }
            }
    return false;
}

// alpha-beta
struct Next
{
    Action action;
    LL score;
};
bool maxFirst(const Next&A, const Next&B) { return A.score > B.score; }
bool minFirst(const Next&A, const Next&B) { return A.score < B.score; }
const int MAX_DEEP = 7;
const int DFS_WIDTH[] = {0, 6, 5, 4, 3, 2, 2, 2, 2, 2};

LL AIEngine::alphabeta(int deep, int player, LL alpha, LL beta)
{
    *debug << "search : " << deep << " " << player << " " << alpha << " " << beta << " " << M << " " << N << endl;

    vector<Next> nexts;
    for(int y = 0; y < N; y ++)
    {
        int x = M-1;
        while(isOK(x, y) && board[x][y] != BOARD_NONE) x --;
        if (isOK(x, y))
        {
            Next n;
            n.score = -INF;
            LL v;

            board[x][y] = player;
            v = evaluate(x, y);
            if (player == BOARD_MINE) v += N - abs(y - N/2);
            else v -= N - abs(y - N/2);
            if (player == BOARD_MINE) n.score = max(n.score, v);
            else n.score = min(n.score, v);

            board[x][y] = BOARD_MINE + BOARD_ENEMY - player;
            v = - evaluate(x, y) - 20;
            if (player == BOARD_MINE) v -= N - abs(y - N/2);
            else v += N - abs(y - N/2);
            if (player == BOARD_MINE) n.score = max(n.score, v);
            else n.score = min(n.score, v);
            
            board[x][y] = BOARD_NONE;

            n.action = Action(x, y);
            nexts.push_back(n);
        }
    }
    sort(nexts.begin(), nexts.end(), player == BOARD_MINE ? maxFirst : minFirst);
    for(int i = 0; i < M; i ++)
    {
        for(int j = 0; j < N; j ++)
            *debug << board[i][j] << " ";
        *debug << endl;
    }
    for(int i = 0; i < (int)nexts.size(); i ++)
        *debug << nexts[i].action.first << " " << nexts[i].action.second << " " << nexts[i].score << endl;

    Action alpha_action, beta_action;
    for(int i = 0; i < (int)nexts.size() && i < DFS_WIDTH[deep]; i ++)
    {
        board[nexts[i].action.first][nexts[i].action.second] = player;
        LL v = 0;
        if (isWin(nexts[i].action.first, nexts[i].action.second)) v = nexts[i].score;
        else v = deep == MAX_DEEP ? nexts[i].score : alphabeta(deep + 1, BOARD_MINE + BOARD_ENEMY - player, alpha, beta);
        board[nexts[i].action.first][nexts[i].action.second] = BOARD_NONE;

        if (player == BOARD_MINE) {
            if (alpha < v) {
                alpha = v;
                alpha_action = nexts[i].action;
            }
        } else {
            if (beta > v) {
                beta = v;
                beta_action = nexts[i].action;
            }
        }

        if (beta <= alpha) break;
    }

    if (deep == 1) {
        alpha1_action = alpha_action;
        beta1_action = beta_action;
    }

    return player == BOARD_MINE ? alpha : beta;
}