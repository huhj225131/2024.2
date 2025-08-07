#include <bits/stdc++.h> 
using namespace std;

int main(){
    int n ;
    cin >> n;
    int arr[n];
    for(int i = 0 ; i < n ; i++){
        cin >> arr[i];
    }
    int m;
    cin >> m;
    int log_n = log2(n) + 1;
    int spare_table[log_n][n];
    for(int i = 0 ; i < n; i++){
        spare_table[0][i] = i;
    }
    for(int i = 1; i < log_n; i++){
        for(int j = 0 ; j <= n - (1 << i) + 1; j++){
            if(arr[spare_table[i- 1][j]] < arr[spare_table[i - 1][j + (1 << (i - 1))]]){
                spare_table[i][j] = spare_table[i- 1][j];
            }
            else{
                spare_table[i][j] = spare_table[i- 1][j + (1 << (i-1))];
            }
        }
    }
    int a, b;
    int sum = 0;
    for(int i = 0 ; i < m; i++){
        cin >> a >> b;
        int logDis = log2(b - a);
        int minNumber = min(arr[spare_table[logDis][a]], arr[spare_table[logDis][ b - (1 << logDis) + 1]]);
        sum += minNumber;
    }
    cout << sum << endl;
}