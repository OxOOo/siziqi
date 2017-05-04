
#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "Point.h"
#include <algorithm>
#include <ostream>

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
    int MAX_M, MAX_N;
    int** pool_lines;
    int* pool_points;
    std::ostream *debug;

    int M, N, reloads;
    int** board;

    LL evaluate(int lastX, int lastY); // 评价
    bool isWin(int lastX, int lastY); // 检测该落子点是否胜利
    void straightNums(int X, int Y, int d, int&count, int&nnums); // d方向相同的数量
    bool isOK(int x, int y); // 检测是否在期盼内
    bool huo(int player, int nums); // 是否活棋
    bool chong(int player, int nums); // 是否活棋

    Action alpha1_action, beta1_action;
    LL alphabeta(int deep, int player, LL alpha, LL beta);
};

#endif // AI_ENGINE_H