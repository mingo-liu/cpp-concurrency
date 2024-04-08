#include <algorithm>
#include <chrono>
#include <execution>
#include <iostream>
#include <vector>
#include <random>

using namespace std;

// 随机数将在 [1.0, 100.0] 的范围内，且均匀分布
void generateRandomData(vector<double>& collection, int size) {
  random_device rd;
  mt19937 mt(rd());
  uniform_real_distribution<double> dist(1.0, 100.0);
  for (int i = 0; i < size; i++) {
    collection.push_back(dist(mt));
  }
}

int main() {
  vector<double> collection;
  generateRandomData(collection, 10e6);   // 生成10e6个随机数

  vector<double> copy1(collection);
  vector<double> copy2(collection);
  vector<double> copy3(collection);

  // 下面分别执行sort的并行版本
  auto time1 = chrono::steady_clock::now();
  sort(execution::seq, copy1.begin(), copy1.end());   // 要求并行算法的执行可以不并行化
  auto time2 = chrono::steady_clock::now();
  auto duration = chrono::duration_cast<chrono::milliseconds>(time2 - time1).count();
  cout << "Sequenced sort consuming " << duration << "ms." << endl;

  auto time3 = chrono::steady_clock::now();     
  sort(execution::par, copy2.begin(),copy2.end());  // 指示并行算法的执行可以并行化
  auto time4 = chrono::steady_clock::now();
  duration = chrono::duration_cast<chrono::milliseconds>(time4 - time3).count();
  cout << "Parallel sort consuming " << duration << "ms." << endl;

  auto time5 = chrono::steady_clock::now();
  sort(execution::par_unseq, copy2.begin(),copy2.end());   // 指示并行算法的执行可以并行化、向量化
  auto time6 = chrono::steady_clock::now();
  duration = chrono::duration_cast<chrono::milliseconds>(time6 - time5).count();
  cout << "Parallel unsequenced sort consuming " << duration << "ms." << endl;
}