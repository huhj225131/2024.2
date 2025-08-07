#include <bits/stdc++.h> 
using namespace std;

int n, K, Q;
int minTotalRoute = INT_MAX;
int totalRoute = 0;
// so buoc di
int segments;
//so xe duoc huy dong
int nbR;
int minRoute =INT_MAX;
//Diem dau cua oto co the la 0
vector<int> y;
//So hang tren xe
vector<int> load;
//diem tiep theo
vector<int> x;
//Diem da den
vector<int> visited;
//Yeu cau
vector<int> d;
//Ma tran kc
vector<vector<int>> dis;

bool checkX(int v, int k){
    if (v > 0 && visited[v]) return false;
    if (load[k] + d[v] > Q) return false;
    return true;
}
void TRY_X(int s,int k){
    if(s == 0){
        if (k < K - 1){
            TRY_X(y[k+ 1], k + 1);
        }
        return;
    }
    for (int v = 0; v <= n; v++){
        if (checkX(v, k)){
            x[s] = v;
            visited[v] = true;
            totalRoute += dis[s][v];
            load[k] += d[v];
            segments++;
            if (v > 0){
                if (totalRoute + (n + nbR - segments) * minRoute < minTotalRoute){
                    TRY_X(v, k);
                }
            }
            else{
                if (k == K - 1){
                    if (segments == (n + nbR)){
                        if (totalRoute < minTotalRoute){
                            minTotalRoute = totalRoute;
                        }
                    }
                }
                else{
                    if (totalRoute + (n + nbR - segments) * minRoute < minTotalRoute){
                        TRY_X(y[k+ 1], k + 1);
                    }
                }
            }
            visited[v]= false; load[k] -= d[v];
            totalRoute -= dis[s][v];
            segments -= 1;
        }
    }
}
 bool checkY(int v,int k){
    if (v == 0) return true;
    if (load[k] + d[v] > Q) return false;
    if (visited[v]) return false;
    return true;
}
void TRY_Y(int k){
    int s = 0;
    if(k != 0){
        if (y[k - 1] != 0) s = y[k-1] + 1;
    }
    for(int v = s; v <= n; v++){
        if (checkY(v, k)){
            y[k] = v;
            visited[v] = true;
            if (v > 0)segments++;
            totalRoute += dis[0][v];
            load[k] += d[v];
            if(k < K - 1) TRY_Y(k + 1);
            else{nbR= segments; TRY_X(y[0], 0);}
            load[k] -= d[v];
            visited[v] = false;
            totalRoute -= dis[0][v];
            if (v > 0) segments --;
        }
    }
}

int main(){
    cin >> n >> K >> Q;
    d.resize(n+ 1, 0);
    dis.resize(n+1, vector<int>(n + 1, 0));
    y.resize(K, 0);
    load.resize(K, 0);
    x.resize(n + 1, 0);
    visited.resize(n + 1, false);
    for(int i = 1 ; i <= n ; i++){
        cin >> d[i];
    }
    for(int i = 0; i <= n; i++){
        for(int j = 0 ; j <= n; j++){
            cin >> dis[i][j];
            if(i != j && minRoute > dis[i][j]){
                minRoute = dis[i][j];
            }
        }
    }
    TRY_Y(0);
    cout << minTotalRoute;
}