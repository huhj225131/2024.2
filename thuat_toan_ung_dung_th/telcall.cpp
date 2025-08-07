#include <bits/stdc++.h> 
using namespace std;


int testcases;
int n;
int c;
vector<int> a;
bool isValid(int d){
    int count = 1;
    int cur = a[0];
    for(int i = 1 ; i < n ; i++){
        if (a[i] - cur >= d){
            count ++;
            cur = a[i];
        }
        if (count == c){
            return true;
        }
    }
        return false;
}
int main(){
    cin >>testcases;
    for(int testcase = 0 ;testcase < testcases;testcase ++){
        cin >> n >> c;
        a.resize(n);
        for(int i =  0; i < n; i++){
            cin >> a[i];
        }
        sort(a.begin(), a.end());
        int max_dis = a[n - 1] - a[0];
        int min_dis = 1;
        
        int best = 0;
        while(max_dis > min_dis){
            int  mid_dis= (max_dis + min_dis) / 2;
            if (isValid(mid_dis)){
                best = mid_dis;
                min_dis = mid_dis + 1;
            }
            else{
                max_dis = mid_dis --;
            }
            
        }
        cout << best<< endl;
    }
 
}