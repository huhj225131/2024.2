#include <bits/stdc++.h> 
using namespace std;

int n;
vector<double> money ;
vector<int> data;
vector<int> cut_d;
bool compare(int a, int b){
    if((money[a] / a) == (money[b]/ b)){
        return a < b;
    }
    return money[a] / a > money[b]/ b;
}
int main(){
    cin >> n;
    money.resize(n + 1, 0);
    data.resize(n  + 1);
    for (int i = 1 ; i <= n ; i++){
        cin >> money[i];
        cout << money[i] << endl;
        data[i] = i;
    }
    sort(data.begin(), data.end(), compare);

    cout << fixed << setprecision(6);
    // for(int i = 1; i <= n; i++){
    //     cout << data[i];
    // }
    double earn = 0;
    int remain = n;
    int cut = 0;
    while(remain > 0){
        for(int i = 1; i <= n; i++){
            if (remain >= data[i]){

                earn += money[data[i]];
                remain -= data[i];
                cut ++;
                cut_d.push_back(data[i]);
                break;
            }
        }
    }
    cout << earn<< endl;
    cout << cut << " ";
    sort(cut_d.begin(), cut_d.end());
    for (int i = 0 ; i < cut ; i++){
        cout << cut_d[i]<< " " ;
    }
}