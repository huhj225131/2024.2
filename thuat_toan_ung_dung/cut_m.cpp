#include <bits/stdc++.h> 
using namespace std;
int H, W;
vector<int> h, w;
int n;
vector<vector<bool>> mat;
bool canCut(int y, int x, int h, int w){
    if (x + w > W || y + h > H){
        return false;
    }
    for(int i = x; i< x + w; i++){
        for(int j = y; j < y + h; j++){
            if (mat[j][i] == true){
                return false;
            }
        }
    }
    return true;

}
void cut(int y, int x, int h, int w, bool cut){
    if (x + w > W || y + h > H){
        return;
    }
    for(int i = x; i< x + w; i++){
        for(int j = y; j < y + h; j++){
            mat[j][i] = cut;
        }
    }
    
    
}
bool Try(int k){
    for(int y = 0 ; y< H; y++){
        for(int x = 0; x < W; x++){
            int r= 0;
            if(canCut(y,x,h[k], w[k])){
                cut(y, x, h[k],w[k],true);
                r = 1;
            }
            else if(canCut(y, x, w[k], h[k])){
                cut(y, x, w[k], h[k], true);
                r = 2;
            }
            if(k == n - 1 && r!=0){
                return true;
            }
            if(r != 0){
                if(Try(k + 1)) return true;
            }
            if(r == 1){
                cut(y, x, h[k],w[k],false);
            }
            if(r== 2){
                cut(y, x, w[k], h[k], false);
            }
            r = 0;
        }
    }
    return false;
}
int main(){
    cin >> H >> W;
    cin >> n;
    h.resize(n);
    w.resize(n);
    for (int i =  0; i < n; i++){
        cin >> h[i] >> w[i];
    }
    mat.resize(H, vector<bool>(W, false));
    if(Try(0)) cout << 1;
    else cout << 0;
}