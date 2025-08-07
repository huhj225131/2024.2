#include <bits/stdc++.h> 
using namespace std;

int max_his(int m, vector<int> &a){
    stack<int> my_stack;
    int maxHis = 0;
    int tp, cur;
    for(int i = 0 ; i < m; i++){
        while(!my_stack.empty() && a[my_stack.top()]>= a[i]){
            tp= my_stack.top();
            my_stack.pop();
            int width;
            if(my_stack.empty()){
                width = i ;
            }
            else{
                width = i - my_stack.top() - 1 ;
            }
            maxHis =max(maxHis, a[tp] * width);
        }
        my_stack.push(i);
    }
    while(!my_stack.empty()){
        tp = my_stack.top(); my_stack.pop();
        cur = a[tp] * (my_stack.empty() ? m : m - my_stack.top() - 1);
        maxHis = max(maxHis, cur);
    }
    return maxHis;
}
int max_black(int n, int m, vector<vector<int>> &arr){
    int maxSub = 0; 
    vector<int> a(m, 0);
    for(int i = 0 ; i< n; i++){
        for(int j = 0 ; j< m; j++){
            if(arr[i][j] == 1){
                a[j] ++;
            }
            else{
                a[j] = 0;
            }
        }
    maxSub = max(maxSub, max_his(m, a)) ;
    }
    return maxSub;
}
int main(){
    int n, m; 
    cin >> n >> m;
    vector<vector<int>> arr(n,vector<int>(m, 0));
    for (int i = 0 ; i < n ; i++){
        for (int j = 0 ; j < m ;j++){
            cin >> arr[i][j];
        }
    }
    cout << max_black(n,m,arr);
}