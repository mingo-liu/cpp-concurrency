#include <condition_variable>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>

using namespace std;

mutex totalMutex;
class Account {
public:
  Account(string name, double money): mName(name), mMoney(money) {};

public:
  void changeMoney(double amount) {
    unique_lock<mutex> lock(mMoneyLock);             // unique_lock可以与条件变量搭配起来使用
    mConditionVar.wait(lock, [this, amount] {   // 满足则继续，不满足则解锁并等待
      return mMoney + amount > 0;
    });
    mMoney += amount;
    mConditionVar.notify_all();     // 唤醒该条件变量对应的等待队列中的线程
  }

  string getName() {
    return mName;
  }

  double getMoney() {
    return mMoney;
  }

private:
  string mName;
  double mMoney;
  mutex mMoneyLock;
  condition_variable mConditionVar;
};

class Bank {
public:
  void addAccount(Account* account) {
    mAccounts.insert(account);
  }

  void transferMoney(Account* accountA, Account* accountB, double amount) {
    accountA->changeMoney(-amount);   // 如果A的账户余额不够，则解锁后等待;
                                      // 这是因为条件变量的等待操作会在等待期间阻塞当前线程，直到被唤醒。在等待期间，当前线程不会继续执行代码。
    accountB->changeMoney(amount);
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

mutex sCoutLock;
void randomTransfer(Bank* bank, Account* accountA, Account* accountB) {
  while(true) {
    double randomMoney = ((double)rand() / RAND_MAX) * 100;
    {
      lock_guard<mutex> guard(sCoutLock);
      cout << "Try to Transfer " << randomMoney
           << " from " << accountA->getName() << "(" << accountA->getMoney()
           << ") to " << accountB->getName() << "(" << accountB->getMoney()
           << "), Bank totalMoney: " << bank->totalMoney() << endl;
    }
    bank->transferMoney(accountA, accountB, randomMoney);
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