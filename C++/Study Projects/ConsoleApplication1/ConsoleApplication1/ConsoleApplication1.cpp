
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int calc(vector<vector<char>> &map, vector<vector<bool>> &hasPassed, int health, int x, int y, int step, int curMinStep)
{
    int n = map.size();
    int m = map[0].size();
    if(n == x+1 && m == y+1) return step;
    if(hasPassed[x][y] || map[x][y] == '*' || step >= curMinStep) return 2147480000;
    int attack = 0;
    if(map[x][y] >='1' &&  map[x][y] <='9')
    {
        attack = map[x][y] - '0';
    }
    if(health <= attack) return 2147480000;
    hasPassed[x][y] = true;
    health -= attack;
    int down = 2147480000;
    int right = 2147480000;
    int up = 2147480000;
    int left = 2147480000;
    int minStep = 2147480000;
    //dfs+回溯
    if(x<n-1)
    {
        down = calc(map, hasPassed, health, x+1, y, step+1, minStep);
    }
    minStep = min(minStep, down);
    if(y<m-1)
    {
        right = calc(map, hasPassed, health, x, y+1, step+1, minStep);
    }
    minStep = min(minStep, right);
    if(x>0)
    {
        up = calc(map, hasPassed, health, x-1, y, step+1, minStep);
    }
    minStep = min(minStep, up);
    if(y>0)
    {
        left = calc(map, hasPassed, health, x, y-1, step+1, minStep);
    }
    minStep = min(minStep, left);
    hasPassed[x][y] = false;
    //
    return minStep;
}

int main(int argc, char* argv[])
{
    int n,m,h;
    cin>>n>>m>>h;
    vector<vector<char>> map(n,vector<char>(m, '.'));
    vector<vector<bool>> hasPassed(n, vector<bool>(m, false));
    for(int i = 0; i<n; i++)
    {
        for(int j = 0; j<m; j++)
        {
            cin>>map[i][j];
        }
    }

    int res = calc(map, hasPassed, h, 0, 0, 0, 2147480000);
    if(res == 2147480000)
    {
        cout<<-1;
    }
    else
    {
        cout<<res;
    }
    return 0;
}
