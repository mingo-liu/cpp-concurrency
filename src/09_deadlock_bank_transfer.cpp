#include <iostream>
#include <mutex>
#include <set>
#include <thread>

using namespace std;

class Account {
public:
  Account(string name, double money): mName(name), mMoney(money) {};

public:
  void changeMoney(double amount) {
    mMoney += amount;
  }
  string getName() {
    return mName;
  }
  double getMoney() {
    return mMoney;
  }
  mutex* getLock() {
    return &mMoneyLock;
  }

private:
  string mName;
  double mMoney;
  mutex mMoneyLock;
};

class Bank {
public:
  void addAccount(Account* account) {
    mAccounts.insert(account);
  }

  bool transferMoney(Account* accountA, Account* accountB, double amount) { // accountA ---> accountB
    lock_guard<mutex> guardA(*accountA->getLock());  // 1.修改银行账户钱获得锁; 多个线程操作同一对象，使用同一把锁
    lock_guard guardB(*accountB->getLock());  // 编译器可以推断出lock_guard的模板参数类型：lock_guard<std::mutex>

    if (amount > accountA->getMoney()) {  // 2.判断钱是否足够
      return false;
    }

    accountA->changeMoney(-amount);   // 3.进行转账
    accountB->changeMoney(amount);
    return true;
  }

  double totalMoney() const {
    double sum = 0;
    for (auto a : mAccounts) {
      sum += a->getMoney();
    }
    return sum;
  }

private:
  set<Account*> mAccounts;
};

void randomTransfer(Bank* bank, Account* accountA, Account* accountB) {
  while(true) {
    double randomMoney = ((double)rand() / RAND_MAX) * 100;
    if (bank->transferMoney(accountA, accountB, randomMoney)) {
      cout << "Transfer " << randomMoney << " from " << accountA->getName()
           << " to " << accountB->getName()
           << ", Bank totalMoney: " << bank->totalMoney() << endl;
    } else {
      cout << "Transfer failed, "
           << accountA->getName() << " has only $" << accountA->getMoney() << ", but "
           << randomMoney << " required" << endl;
    }
  }
}

int main() {
  Account a("Paul", 100);
  Account b("Moira", 100);

  Bank aBank;
  aBank.addAccount(&a);
  aBank.addAccount(&b);

  thread t1(randomTransfer, &aBank, &a, &b);
  thread t2(randomTransfer, &aBank, &b, &a);

  t1.join();
  t2.join();

  return 0;
}