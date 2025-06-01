#include <gtest/gtest.h>
#include <stdexcept>
#include "Account.h"
#include "Transaction.h"
#include <gmock/gmock.h>

using ::testing::AtLeast;
using ::testing::_;
using ::testing::Return;
using ::testing::Throw;
using ::testing::InSequence;

class MockAccount : public Account {
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

TEST(Account, RealBehavior) {
  Account acc(1, 1000);
  EXPECT_EQ(1000, acc.GetBalance());
  EXPECT_EQ(1, acc.id());
  
  acc.Lock();
  acc.ChangeBalance(500);
  EXPECT_EQ(1500, acc.GetBalance());
  acc.Unlock();
  
  EXPECT_THROW(acc.ChangeBalance(100), std::runtime_error);
  
  // Тест повторной блокировки
  acc.Lock();
  EXPECT_THROW(acc.Lock(), std::runtime_error);
  acc.Unlock();
}

TEST(Transaction, MockInteraction) {
  MockAccount from_acc(1, 2000);
  MockAccount to_acc(2, 1000);
  MockTransaction transaction;
  
  EXPECT_CALL(from_acc, Lock()).Times(1);
  EXPECT_CALL(to_acc, Lock()).Times(1);
  EXPECT_CALL(from_acc, GetBalance()).WillOnce(Return(2000));
  EXPECT_CALL(to_acc, ChangeBalance(500)).Times(1);
  EXPECT_CALL(from_acc, ChangeBalance(-501)).Times(1);
  EXPECT_CALL(transaction, SaveToDataBase(_, _, 500)).Times(1);
  EXPECT_CALL(from_acc, Unlock()).Times(1);
  EXPECT_CALL(to_acc, Unlock()).Times(1);
  
  bool result = transaction.Make(from_acc, to_acc, 500);
  EXPECT_TRUE(result);
}

TEST(Transaction, InsufficientFunds) {
  MockAccount from_acc(1, 100);
  MockAccount to_acc(2, 1000);
  Transaction transaction;
  
  bool result = transaction.Make(from_acc, to_acc, 500);
  EXPECT_FALSE(result);
}

TEST(Transaction, RealBehavior) {
  Account from_acc(1, 2000);
  Account to_acc(2, 1000);
  Transaction transaction;
  
  bool result = transaction.Make(from_acc, to_acc, 500);
  EXPECT_TRUE(result);
  EXPECT_EQ(1499, from_acc.GetBalance());
  EXPECT_EQ(1500, to_acc.GetBalance());
  
  EXPECT_THROW(transaction.Make(from_acc, from_acc, 100), std::logic_error);
  EXPECT_THROW(transaction.Make(from_acc, to_acc, -100), std::invalid_argument);
  EXPECT_THROW(transaction.Make(from_acc, to_acc, 50), std::logic_error);
}

TEST(Transaction, FeeTest) {
  Account from_acc(1, 1000);
  Account to_acc(2, 1000);
  Transaction transaction;
  
  transaction.set_fee(100);
  EXPECT_EQ(100, transaction.fee());
  
  bool result = transaction.Make(from_acc, to_acc, 150);
  EXPECT_FALSE(result);
}
