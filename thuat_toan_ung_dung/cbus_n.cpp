#include <bits/stdc++.h>
using namespace std;

int n, k;
int current = 0, minRoute = INT_MAX;
int currentTotalRoute = 0, minTotalRoute = INT_MAX;
bitset<22> loc;  // Dùng bitset thay vì bool[]
int route[22] = {0};
vector<int> sortedOrder;

inline bool checkValid(int i) {
    if (loc[i]) return false;
    return (i <= n) ? (current < k) : loc[i - n];
}

void Try(int step, vector<vector<int>> &dis) {
    if (currentTotalRoute + (2 * n + 1 - step) * minRoute >= minTotalRoute) return;

    int prev = route[step - 1];
    for (int i : sortedOrder) {
        if (!checkValid(i)) continue;

        loc[i] = true;
        route[step] = i;
        (i <= n) ? current++ : current--;
        currentTotalRoute += dis[prev][i];

        if (step == 2 * n) {
            minTotalRoute = min(minTotalRoute, currentTotalRoute + dis[i][0]);
        } else {
            Try(step + 1, dis);
        }

        loc[i] = false;
        (i <= n) ? current-- : current++;
        currentTotalRoute -= dis[prev][i];
    }
}

int main() {
    cin >> n >> k;
    vector<vector<int>> dis(2 * n + 1, vector<int>(2 * n + 1));

    for (int i = 0; i <= 2 * n; i++) {
        for (int j = 0; j <= 2 * n; j++) {
            cin >> dis[i][j];
            if (i != j) minRoute = min(minRoute, dis[i][j]);
        }
    }

    // Sắp xếp các điểm theo khoảng cách từ điểm xuất phát
    sortedOrder.resize(2 * n);
    iota(sortedOrder.begin(), sortedOrder.end(), 1);
    sort(sortedOrder.begin(), sortedOrder.end(), [&](int a, int b) {
        return dis[0][a] < dis[0][b];
    });

    route[0] = 0;
    Try(1, dis);
    cout << minTotalRoute << endl;
}
