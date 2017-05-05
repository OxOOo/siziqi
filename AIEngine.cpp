#include "AIEngine.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <sstream>
#include <queue>

using namespace std;

const int DX[] = {0, 1, 1, 1};
const int DY[] = {1, 0, 1, -1};
const int TIMES = 10000;
const double EXPAND_P = 0.5;
const double C = 1.5;

AIEngine::AIEngine(int MAX_M, int MAX_N): MAX_M(MAX_M), MAX_N(MAX_N)
{
    pool_lines = new int*[MAX_M];
    pool_points = new int[MAX_M*MAX_N];
    simulation_xs = new int[MAX_M*MAX_N];
    simulation_ys = new int[MAX_M*MAX_N];
    simulation_x_list = new int[MAX_N];
    simulation_y_list = new int[MAX_N];
    simulation_top = new int[MAX_N];
    reloads = 0;

    node_pool = new Node[TIMES*MAX_N];
    for(int i = 0; i < TIMES*MAX_N; i ++)
    {
        node_pool[i].xs.resize(MAX_N);
        node_pool[i].nodes.resize(MAX_N);
    }
}

AIEngine::~AIEngine()
{
    delete[] pool_lines;
    delete[] pool_points;
    delete[] simulation_xs;
    delete[] simulation_ys;
    delete[] simulation_x_list;
    delete[] simulation_y_list;
    delete[] simulation_top;
    delete[] node_pool;
}

bool AIEngine::isWin(int lastX, int lastY)
{
    for(int d = 0; d < 4; d ++)
    {
        int dx = DX[d], dy = DY[d];
        int count = 1;
        for(int x = lastX+dx, y = lastY+dy; isOK(x, y); x += dx, y += dy)
            if (board[x][y] == board[lastX][lastY])
                count ++;
            else break;
        for(int x = lastX-dx, y = lastY-dy; isOK(x, y); x -= dx, y -= dy)
            if (board[x][y] == board[lastX][lastY])
                count ++;
            else break;
        if (count >= 4) return true;
    }
    return false;
}

inline bool AIEngine::isOK(int x, int y)
{
    return 0 <= x && x < M && 0 <= y && y < N;
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

    if (reloads == 0)
    {
        system("del debug*.txt");
    }

    reloads ++;
    node_pool_cnt = 0;
}

Point AIEngine::getAction()
{
    char buf[1024];
    sprintf(buf, "debug%d.txt", reloads);
    debug = new ofstream(buf);
    // debug = new ostringstream();

    Point rst = MCST();
    
    debug->flush();
    delete debug;

    return rst;
}

Point AIEngine::MCST()
{
    Node* root = newNode(BOARD_MINE);

    for(int x = 0; x < M; x ++)
    {
        for(int y = 0; y < N; y ++)
            *debug << board[x][y] << " ";
        *debug << endl;
    }

    for(int t = 0; t < TIMES; t ++)
    {
        // *debug << "==============" << endl;
        // for(int x = 0; x < M; x ++)
        // {
        //     for(int y = 0; y < N; y ++)
        //         *debug << board[x][y] << " ";
        //     *debug << endl;
        // }

        Node* pos = root;
        vector<Node*> way;
        vector<int> way_cy;
        way.push_back(root);

        double slogN = sqrt(log(root->b));

        while(!pos->is_leaf)
        {
            double p = -1e16;
            Node* next = NULL;
            int cy = 0;

            for(int y = 0; y < N; y ++)
                if (pos->nodes[y] != NULL && p < pos->nodes[y]->UCB_x + pos->nodes[y]->UCB_factor*slogN)
                {
                    p = pos->nodes[y]->UCB_x + pos->nodes[y]->UCB_factor*slogN;
                    cy = y;
                    next = pos->nodes[y];
                }
            board[pos->xs[cy]][cy] = pos->player;
            way_cy.push_back(cy);
            way.push_back(next);
            pos = next;
        }

        bool can_expand = false;
        for(int y = 0; y < N; y ++)
            can_expand |= isOK(pos->xs[y], y);
        can_expand &= pos->winner == BOARD_NONE;
        LL this_a = 0, this_b = 0;
        if (can_expand && (rand()%100 < EXPAND_P*100 || pos == root)) // 扩展
        {
            pos->is_leaf = false;
            for(int y = 0; y < N; y ++)
            {
                int x = pos->xs[y];
                if (isOK(x, y))
                {
                    board[x][y] = pos->player;
                    pos->nodes[y] = newNode(BOARD_MINE + BOARD_ENEMY - pos->player);
                    pos->nodes[y]->b = 2;
                    if (isWin(x, y)) {
                        pos->nodes[y]->winner = pos->player;
                        pos->nodes[y]->a = 2*int(pos->player == BOARD_MINE);
                    } else {
                        pos->nodes[y]->a = simulation(pos->nodes[y]->player);
                    }
                    recalcUCB(pos->nodes[y]);
                    this_b += pos->nodes[y]->b;
                    this_a += pos->nodes[y]->a;
                    board[x][y] = BOARD_NONE;
                }
            }
        } else if (pos->winner != BOARD_NONE) { // 终态
            this_b = 2;
            this_a = LL(pos->winner == BOARD_MINE)*2;
        } else {
            this_b = 2;
            this_a = simulation(pos->player);
        }

        for(int i = 0; i < (int)way_cy.size(); i ++)
            board[way[i]->xs[way_cy[i]]][way_cy[i]] = BOARD_NONE;
        for(int i = 0; i < (int)way.size(); i ++)
        {
            way[i]->a += this_a;
            way[i]->b += this_b;
            recalcUCB(way[i]);
        }
    }

    Point action(-1, -1);
    double p = -1e16;
    for(int y = 0; y < N; y ++)
    {
        int x = root->xs[y];
        if (isOK(x, y) && p < root->nodes[y]->UCB_x)
        {
            p = root->nodes[y]->UCB_x;
            action = Point(x, y);
        }
    }

    *debug << "!!!!!!!!!!!!!!!!!!!" << endl;
    for(int y = 0; y < N; y ++)
    {
        int x = root->xs[y];
        if (isOK(x, y))
        {
            *debug << x << " " << y << " " << root->nodes[y]->a << " " << root->nodes[y]->b << endl;
        }
    }
    *debug << action.x << " " << action.y << endl;

    return action;
}

AIEngine::Node* AIEngine::newNode(int player)
{
    Node *node = &node_pool[node_pool_cnt++];
    node->a = node->b = 0;
    node->is_leaf = true;
    node->winner = BOARD_NONE;
    node->player = player;
    node->UCB_x = node->UCB_factor = 0.0;

    for(int y = 0; y < N; y ++)
    {
        int x = M - 1;
        while(isOK(x, y) && board[x][y] != BOARD_NONE) x --;
        node->xs[y] = x;
        node->nodes[y] = NULL;
    }

    return node;
}

void AIEngine::recalcUCB(AIEngine::Node *node)
{
    if (node->player != BOARD_MINE) {
        node->UCB_x = double(node->a) / double(node->b);
        node->UCB_factor = C*sqrt(1.0/node->b);
    } else {
        node->UCB_x = double(node->b - node->a) / double(node->b);
        node->UCB_factor = C*sqrt(1.0/node->b);
    }
}

int AIEngine::simulation(int player)
{
    int rst = 1;
    int cnt = 0;

    for(int y = 0; y < N; y ++)
    {
        int x = M - 1;
        while(isOK(x, y) && board[x][y] != BOARD_NONE) x --;
        simulation_top[y] = x;
    }

    while(true)
    {
        int list_cnt = 0;
        for(int y = 0; y < N; y ++)
        {
            int x = simulation_top[y];
            if (isOK(x, y))
            {
                simulation_x_list[list_cnt] = x;
                simulation_y_list[list_cnt] = y;
                list_cnt ++;
            }
        }
        if (list_cnt)
        {
            int x = -1, y = -1;
            for(int i = 0; i < list_cnt && x == -1; i ++)
            {
                board[simulation_x_list[i]][simulation_y_list[i]] = player;
                if (isWin(simulation_x_list[i], simulation_y_list[i])) {
                    x = simulation_x_list[i];
                    y = simulation_y_list[i];
                }
                board[simulation_x_list[i]][simulation_y_list[i]] = BOARD_NONE;
            }
            for(int i = 0; i < list_cnt && x == -1; i ++)
            {
                board[simulation_x_list[i]][simulation_y_list[i]] = BOARD_MINE + BOARD_ENEMY - player;
                if (isWin(simulation_x_list[i], simulation_y_list[i])) {
                    x = simulation_x_list[i];
                    y = simulation_y_list[i];
                }
                board[simulation_x_list[i]][simulation_y_list[i]] = BOARD_NONE;
            }
            if (x == -1) {
                int r = rand() % list_cnt;
                x = simulation_x_list[r];
                y = simulation_y_list[r];
            }

            simulation_xs[cnt] = x;
            simulation_ys[cnt] = y;
            cnt ++;
            board[x][y] = player;
            if (isWin(x, y)) {
                rst = 2*int(player == BOARD_MINE);
                break;
            }
            player = BOARD_MINE + BOARD_ENEMY - player;
            while(isOK(x, y) && board[x][y] != BOARD_NONE) x --;
            simulation_top[y] = x;
        } else {
            rst = 1;
            break;
        }
    }

    for(int i = 0; i < cnt; i ++)
        board[simulation_xs[i]][simulation_ys[i]] = BOARD_NONE;

    return rst;
}