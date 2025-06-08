#include <gtest/gtest.h>
#include <stdexcept>
#include "Account.h"
#include "Transaction.h"
#include <gmock/gmock.h>

class MockAccount : public Account {
 private:
  int id;
  int balance;
 public:
  MockAccount(int id, int balance) : Account(id, balance) {}
  MOCK_METHOD(int, GetBalance, (), (const, override));
  MOCK_METHOD(void, ChangeBalance, (int), (override));
  MOCK_METHOD(void, Lock, (), (override));
  MOCK_METHOD(void, Unlock, (), (override));
};

class MockTransaction : public Transaction {
 public:
  MockTransaction() : Transaction() {}
  MOCK_METHOD(void, SaveToDataBase, (Account& from, Account& to, int sum), (override));
};

using ::testing::AtLeast;

TEST(Account, Mock) {
  MockAccount ac1(1, 1000);
  EXPECT_CALL(ac1, GetBalance()).Times(AtLeast(1));
  std::cout <<  ac1.GetBalance() << std::endl;
  EXPECT_CALL(ac1, Lock()).Times(AtLeast(1));
  ac1.Lock();
  EXPECT_CALL(ac1, ChangeBalance(1)).Times(AtLeast(1));
  ac1.ChangeBalance(1);
  EXPECT_CALL(ac1, Unlock()).Times(AtLeast(1));
  ac1.Unlock();
  
}

TEST(Transaction, Mock) {
  Account ac1(1, 10000);
  Account ac2(2, 10000);
  MockTransaction t1;
  EXPECT_CALL(t1, SaveToDataBase(ac1, ac2, 1999)).Times(AtLeast(1));
  t1.Make(ac1, ac2, 1999);
  
  
}

TEST(Account, Methods) {
  Account ac1(1, 1000);
  EXPECT_EQ(1000, ac1.GetBalance());
  ac1.Lock();
  ac1.ChangeBalance(2000);
  ac1.Unlock();
  EXPECT_EQ(3000, ac1.GetBalance());
  try {
    ac1.ChangeBalance(1);
  }
  catch (std::runtime_error& el) {}
  EXPECT_EQ(3000, ac1.GetBalance());
}

TEST(Transaction, Methods) {
  Account ac1(1, 10000);
  Account ac2(2, 10000);
  Transaction t1;
  Transaction t2; t2.set_fee(500);
  try {t1.Make(ac1, ac1, 100); EXPECT_EQ(1, 0);}
  catch (std::logic_error& el) {}
  try {t1.Make(ac1, ac2, -100); EXPECT_EQ(1, 0);}
  catch (std::invalid_argument& el) {}
  try {t1.Make(ac1, ac2, 0); EXPECT_EQ(1, 0);}
  catch (std::logic_error& el) {}
  EXPECT_EQ(false, t2.Make(ac1, ac2, 200));
  t1.Make(ac1, ac2, 1999);
  EXPECT_EQ(ac1.GetBalance(), 8000); EXPECT_EQ(ac2.GetBalance(), 11999);
}
