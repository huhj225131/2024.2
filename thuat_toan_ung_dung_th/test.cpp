
#include <bits/stdc++.h> 
using namespace std;


vector<int> num; 
vector<int> low;
vector<int> art_point;
int count_num;
int bridges;
vector< vector<int> > graph;
void dfs(int k, int parent){
	int children = 0;
	if(num[k] == -1){
		num[k] = low[k] =  count_num++;
//		cout << k << endl;
		int len= graph[k].size();
		for (int i = 0 ; i < len; i++){
			if(graph[k][i] == parent) continue;
			if(num[graph[k][i]] ==-1){
				
				dfs(graph[k][i], k);
				low[k] = min(low[k], low[graph[k][i]]);
				if(low[graph[k][i]] > num[k]){
					bridges++;
				}
				if(low[graph[k][i]] >= num[k] && (parent != -1)){
					art_point.push_back(k);
				} 
			}
			else{
				low[k] = min(low[k], num[graph[k][i]]);
			}
			children ++;
			
		}
		if (parent == -1 && children > 1){
			art_point.push_back(k);
		}
	}
} 
int main(){
    int  n = 0 ;
	int m = 0; 
	bridges = 0;
	cin >> n >> m;
	count_num = 0;
	graph.resize(n + 1);
	num.resize(n + 1, -1);
	low.resize(n + 1);
	for (int i = 0; i < m ; i++){
		int a, b;
		cin >> a >> b;
		graph[a].push_back(b);
		graph[b].push_back(a);
		
	}
	dfs(1, -1);
//	cout << bridges << endl;
	for(int i = 0; i< art_point.size(); i++){
		cout << art_point[i]<< endl;
	}
	
	
}
