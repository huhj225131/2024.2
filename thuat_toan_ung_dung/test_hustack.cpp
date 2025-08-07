
#include <bits/stdc++.h> 
using namespace std;


int m , n, a, b;
int k;
int countR;
vector<vector<int>> block;
bool check_block(int a , int b){
    if(a  > m || b > n){
        return false;
    }
    if (block[a][b] == 0){
        return true;
    }
    return false;
}
bool check(){
    if(a <= m && b <= n ){
        return true;
    }
    return false;
}
void Try(){
    if(check()){
        if(a == m && b == n){
            countR  = countR + 1;
            countR = countR % 1000000007;
        }
        if (check_block(a + 1, b)){
            a = a + 1;
            // cout << "ch";
            cout << a <<" "<< b<< endl;
            Try();
            a = a - 1;
        }
        if(check_block(a, b+1)){
            b = b + 1;
            cout << a <<" "<< b<< endl;
            Try();
            b = b - 1;
        }
    }
}

int main(){
    cin >> m >> n;
    block.resize(m + 1, vector<int>(n + 1, 0));
    a = b = 1;
    int i, j;
    cin >> k;
    for(int h = 0 ; h < k; h++){
        cin >> i >> j;
        block[i][j] = 1;
    }
    // cout << block[2][2]<< block[3][3];
    countR = 0;
    Try();
    cout << countR;
}