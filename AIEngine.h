
#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "Point.h"
#include <algorithm>
#include <ostream>
#include <vector>

#define BOARD_NONE 0
#define BOARD_MINE 2
#define BOARD_ENEMY 1
#define BOARD_BLOCK 3
#define INF 0xfffffffLL

typedef long long LL;
typedef std::pair<int, int> Action;

// AI引擎
class AIEngine
{
public:
    AIEngine(int MAX_M, int MAX_N);
    ~AIEngine();

    void reload(const int M, const int N,  const int* _board, const int noX, const int noY);
    Point getAction();

private:
    struct Node
    {
        LL a, b;
        bool is_leaf; // 是否叶子
        int winner; // 胜者
        int player; // 先手
        double UCB_x, UCB_factor;
        std::vector<int> xs;
        std::vector<AIEngine::Node*> nodes;
    };

    int MAX_M, MAX_N;
    int** pool_lines;
    int* pool_points;
    int *simulation_xs, *simulation_ys;
    int *simulation_x_list, *simulation_y_list, *simulation_top;
    AIEngine::Node* node_pool;
    int node_pool_cnt;
    std::ostream *debug;

    int M, N, reloads;
    int** board;

    bool isWin(int lastX, int lastY); // 检测该落子点是否胜利
    inline bool isOK(int x, int y); // 检测是否在期盼内

    Point MCST(); // 蒙特卡洛搜索
    AIEngine::Node* newNode(int player);
    void recalcUCB(AIEngine::Node *node);
    int simulation(int player); // 随机模拟，返回0/1/2
};

#endif // AI_ENGINE_H