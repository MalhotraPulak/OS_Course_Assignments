// Lets Go!
#include <random>
#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <bitset>
#include <deque>
#include <stack>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstring>

#define int long long
#define pb push_back
#define mp make_pair
#define vvpii vector <vector<pii>>
#define endl "\n"
#define fast_io() std::ios::sync_with_stdio(false), cin.tie(nullptr), cout.tie(nullptr)
#define all(x) (x).begin(), (x).end()
#define input(x) for(int &y : (x)) cin >> y
using namespace std;

int nxt() {
    int x;
    cin >> x;
    return x;
}

int _gcd(int a, int b) {
    if (a == 0) return b;
    return _gcd(b % a, a);
}

void testcase() {
    int n = 100000;
    cout << n << endl;
    for(int i = 0; i < n; i++){
        int x = rand();
        cout << x << endl;
    }

}

signed main() {
    fast_io();
    //int t; cin >> t;while(t--)
    testcase();

}
