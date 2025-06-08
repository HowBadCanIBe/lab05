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

// Test that Transaction properly calls Account methods during transaction
TEST(Transaction, MockAccountInteraction) {
  MockAccount from_account(1, 10000);
  MockAccount to_account(2, 5000);
  Transaction transaction;

  // Set up expectations for what Transaction should call on accounts
  EXPECT_CALL(from_account, Lock()).Times(1);
  EXPECT_CALL(to_account, Lock()).Times(1);
  EXPECT_CALL(to_account, ChangeBalance(1000)).Times(1);  // Credit
  EXPECT_CALL(from_account, GetBalance()).Times(1);        // Check balance for debit
  EXPECT_CALL(from_account, ChangeBalance(-1001)).Times(1); // Debit (1000 + 1 fee)
  EXPECT_CALL(from_account, Unlock()).Times(1);
  EXPECT_CALL(to_account, Unlock()).Times(1);

  // When GetBalance is called, return the current balance
  EXPECT_CALL(from_account, GetBalance())
    .WillOnce(::testing::Return(10000));

  // Execute transaction - this will implicitly call the mocked methods
  bool result = transaction.Make(from_account, to_account, 1000);
  EXPECT_TRUE(result);
}

// Test insufficient funds scenario with mocks
TEST(Transaction, MockInsufficientFunds) {
  MockAccount from_account(1, 500);  // Not enough for 1000 + fee
  MockAccount to_account(2, 5000);
  Transaction transaction;

  EXPECT_CALL(from_account, Lock()).Times(1);
  EXPECT_CALL(to_account, Lock()).Times(1);
  EXPECT_CALL(to_account, ChangeBalance(1000)).Times(1);    // Credit first
  EXPECT_CALL(from_account, GetBalance()).Times(1);         // Check balance
  EXPECT_CALL(from_account, ChangeBalance(-1000)).Times(1); // Rollback credit
  EXPECT_CALL(from_account, Unlock()).Times(1);
  EXPECT_CALL(to_account, Unlock()).Times(1);

  // Return insufficient balance
  EXPECT_CALL(from_account, GetBalance())
    .WillOnce(::testing::Return(500));

  bool result = transaction.Make(from_account, to_account, 1000);
  EXPECT_FALSE(result);
}

// Test SaveToDataBase is called with mock
TEST(Transaction, MockSaveToDataBase) {
  Account from_account(1, 10000);
  Account to_account(2, 5000);
  MockTransaction mock_transaction;

  // Expect SaveToDataBase to be called during transaction
  EXPECT_CALL(mock_transaction, SaveToDataBase(::testing::Ref(from_account), 
                                              ::testing::Ref(to_account), 1000))
    .Times(1);

  mock_transaction.Make(from_account, to_account, 1000);
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
