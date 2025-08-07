#include<bits/stdc++.h>

using namespace std;


int n , k;
int current = 0, minRoute = INT_MAX;
int currentTotalRoute = 0;
int minTotalRoute = INT_MAX;
bool loc[22] = {false};
int route[22] = {0};
vector<int> sortedOrder(22);

 bool checkValid(int i){
    if (loc[i]) return false;
    return (i <= n) ? (current < k) : loc[i - n];
}
void Try(int k , vector<vector<int>>  & dis){
    
    for(int i : sortedOrder){
        if(checkValid(i)){
            loc[i] = true;
            route[k] = i;
            if (i <= n){
                current++;
            }
            else{
                current--;
            }
            currentTotalRoute += dis[route[k - 1]][i];
            // cout << currentTotalRoute << endl;
            if (k == 2* n){
                
                if (currentTotalRoute+ dis[i][0]< minTotalRoute){
                    minTotalRoute = currentTotalRoute + dis[i][0];
                }
            }
            else{
                if (currentTotalRoute + (2 * n + 1- k) * minRoute <= minTotalRoute){
                    Try(k +1, dis);
                }
            }
            loc[i] = false;
            route[k] = 0;
            if (i <= n){
                current--;
            }
            else{
                current++;
            }
            currentTotalRoute -= dis[route[k - 1]][i];
        }
    }
}
int main(){
    cin >> n >> k;
    vector<vector<int>> dis(2 * n + 1, vector<int>(2 *n + 1, 0) );
    for(int i = 0 ; i <= 2* n; i++){
        for(int j = 0 ; j <= 2*n ; j++){
            cin >> dis[i][j];
            if(dis[i][j] < minRoute && i !=j){
                minRoute = dis[i][j];
            }
        }
    }
    sortedOrder.resize(2 * n);
    iota(sortedOrder.begin(), sortedOrder.end(), 1);
    sort(sortedOrder.begin(), sortedOrder.end(), [&](int a, int b) {
        return dis[0][a] < dis[0][b];
    });
    route[0] = 0;
    Try(1, dis);
    cout << minTotalRoute << endl;
}
