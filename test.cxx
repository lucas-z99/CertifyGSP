
#include <atomic>
#include <vector>
#include <iostream>
#include <fstream>
#include <stack>
#include <chrono>
#include <iomanip>
#include <string>

using std::pair;
using std::vector;

int main()
{
    using namespace std;

    auto fun = vector<int>{};

    // fun.reserve(10);
    fun[5] = 100;

    cout << fun[5] << endl;
    cout << fun[9] << endl;
    cout << fun[11] << endl;
    // cout << fun.capacity() << fun.size() << endl;

    return 0;
}
