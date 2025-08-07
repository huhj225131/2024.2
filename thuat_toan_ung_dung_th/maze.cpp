#include<iostream>
#include<queue>


using namespace std;

int main(){
    int n, m , r, c;
    cin >> n >> m >> r>> c;
    int maze[n][m];
    int dis[n][m];
    bool visited[n][m];
    for(int i = 0 ; i < n ; i++){
        for(int j = 0 ; j < m; j++){
            cin >> maze[i][j];
            dis[i][j] = 0;
            visited[i][j] = false;
        }
    }
    pair<int, int> start;
    start.first = r - 1;
    start.second = c - 1;
    int move_x[4] = {0,1,-1,0};
    int move_y[4] = {1,0,0,-1};
    queue<pair<int, int>> my_queue;
    my_queue.push(start);
    while(!my_queue.empty()){
        pair<int, int> cur = my_queue.front();
        my_queue.pop();
        // visited[cur.first][cur.second] = true;
        if (cur.first == 0 || cur.first == n - 1 || cur.second== 0 || cur.second == m - 1){
            cout << dis[cur.first][cur.second] + 1;
            return 0;
        }
        for (int i = 0 ; i< 4; i++){
            if(cur.first + move_x[i] >= 0 && cur.first + move_x[i] < n &&cur.second + move_y[i] >= 0 && cur.second + move_y[i] < m ){
            if ((visited[cur.first + move_x[i]][cur.second + move_y[i]] == false)
            && maze[cur.first + move_x[i]][cur.second + move_y[i]] == 0){
                dis[cur.first + move_x[i]][cur.second + move_y[i]] = dis[cur.first][cur.second] + 1;
                pair<int, int> next;
                next.first = cur.first + move_x[i];
                next.second = cur.second + move_y[i];
                visited[next.first][next.second] = true;
                my_queue.push(next);
            }
        }
        }

    }
    cout << -1;
    return 0;
}
