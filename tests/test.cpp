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

TEST(Account, BasicBehavior) {
  Account acc(1, 1000);
  EXPECT_EQ(1000, acc.GetBalance());
  EXPECT_EQ(1, acc.id());
  
  acc.Lock();
  acc.ChangeBalance(500);
  EXPECT_EQ(1500, acc.GetBalance());
  acc.Unlock();
  
  EXPECT_THROW(acc.ChangeBalance(100), std::runtime_error);
  
  acc.Lock();
  EXPECT_THROW(acc.Lock(), std::runtime_error);
  acc.Unlock();
}

TEST(Account, EqualityOperator) {
  Account acc1(1, 1000);
  Account acc2(1, 2000);
  Account acc3(2, 1000);
  
  EXPECT_TRUE(acc1 == acc2);
  EXPECT_FALSE(acc1 == acc3);
}

TEST(Transaction, LockUnlockOrder) {
  MockAccount from_acc(1, 2000);
  MockAccount to_acc(2, 1000);
  Transaction transaction;
  
  // Проверяем порядок блокировки и разблокировки счетов
  {
    InSequence seq;
    EXPECT_CALL(from_acc, Lock());
    EXPECT_CALL(to_acc, Lock());
    EXPECT_CALL(from_acc, Unlock());
    EXPECT_CALL(to_acc, Unlock());
  }
  
  // Ожидаем вызовы изменения баланса
  EXPECT_CALL(from_acc, GetBalance()).WillOnce(Return(2000));
  EXPECT_CALL(to_acc, ChangeBalance(500));
  EXPECT_CALL(from_acc, ChangeBalance(-501));
  
  transaction.Make(from_acc, to_acc, 500);
}

TEST(Transaction, InsufficientFunds) {
  MockAccount from_acc(1, 100);
  MockAccount to_acc(2, 1000);
  Transaction transaction;
  
  EXPECT_CALL(from_acc, GetBalance()).WillOnce(Return(100));
  
  bool result = transaction.Make(from_acc, to_acc, 500);
  EXPECT_FALSE(result);
}

TEST(Transaction, SelfTransfer) {
  MockAccount acc(1, 1000);
  Transaction transaction;
  
  EXPECT_THROW(transaction.Make(acc, acc, 100), std::logic_error);
}

TEST(Transaction, NegativeAmount) {
  MockAccount from_acc(1, 1000);
  MockAccount to_acc(2, 1000);
  Transaction transaction;
  
  EXPECT_THROW(transaction.Make(from_acc, to_acc, -100), std::invalid_argument);
}

TEST(Transaction, SmallAmount) {
  MockAccount from_acc(1, 1000);
  MockAccount to_acc(2, 1000);
  Transaction transaction;
  
  EXPECT_THROW(transaction.Make(from_acc, to_acc, 50), std::logic_error);
}

TEST(Transaction, FeeCalculation) {
  MockAccount from_acc(1, 1000);
  MockAccount to_acc(2, 1000);
  Transaction transaction;
  
  transaction.set_fee(100);
  EXPECT_EQ(100, transaction.fee());
  
  EXPECT_CALL(from_acc, GetBalance()).WillOnce(Return(1000));
  
  bool result = transaction.Make(from_acc, to_acc, 150);
  EXPECT_FALSE(result); // 150 + 100 > 1000
}

TEST(Transaction, SuccessfulTransfer) {
  Account from_acc(1, 2000);
  Account to_acc(2, 1000);
  Transaction transaction;
  
  bool result = transaction.Make(from_acc, to_acc, 500);
  EXPECT_TRUE(result);
  EXPECT_EQ(1499, from_acc.GetBalance()); // 2000 - 500 - 1 (fee)
  EXPECT_EQ(1500, to_acc.GetBalance());   // 1000 + 500
}
