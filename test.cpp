#include <iostream>
#include <fstream>
#include "AIEngine.h"

using namespace std;

int main()
{
    AIEngine AI(13, 13);

    int M, N;
    int* board;
    int noX, noY;

    ifstream fin("test/test6.txt");
    fin >> M >> N;
    board = new int[M*N];
    for(int x = 0; x < M; x ++)
    {
        for(int y = 0; y < N; y ++)
        {
            int t;
            fin >> t;
            board[x*N+y] = 0;
            if (t == 3) {noX = x; noY = y;}
            else board[x*N+y] = t;
        }
    }

    AI.reload(M, N, board, noX, noY);
    Point x = AI.getAction();

    cout << x.x << " " << x.y << endl;

    return 0;
}