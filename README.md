### Задание
1. Создайте `CMakeList.txt` для библиотеки *banking*.
```sh
git clone https://github.com/tp-labs/lab05.git lab05             
Клонирование в «lab05»...
remote: Enumerating objects: 137, done.
remote: Counting objects: 100% (25/25), done.
remote: Compressing objects: 100% (9/9), done.
remote: Total 137 (delta 18), reused 16 (delta 16), pack-reused 112 (from 1)
Получение объектов: 100% (137/137), 918.92 КиБ | 518.00 КиБ/с, готово.
Определение изменений: 100% (60/60), готово.
                                                                                                                                          
cd lab05/banking
                                                                                                                                          
touch CMakeLists.txt
CMakeLists.txt:
'''
cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(banking)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(banking STATIC 
    Account.cpp Account.h 
    Transaction.cpp Transaction.h
)
'''                                                                                                                                       
ls                                       
Account.cpp  Account.h  CMakeLists.txt  CMakeList.txt  Transaction.cpp  Transaction.h
                                                                                                                                          
rm CMakeList.txt

cd ..                       
                                                                                                                                 
touch CMakeLists.txt

CMakeLists.txt:
'''
cmake_minimum_required(VERSION 3.4)

set(COVERAGE OFF CACHE BOOL "Coverage")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

project(TestRunning)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/googletest" "gtest")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/banking)

add_executable(RunTest ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)

if(COVERAGE)
    target_compile_options(RunTest PRIVATE --coverage)
    target_link_libraries(RunTest PRIVATE --coverage)
endif()

target_include_directories(RunTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/banking)
target_link_libraries(RunTest PRIVATE gtest gtest_main gmock_main banking)
'''

```
2. Создайте модульные тесты на классы `Transaction` и `Account`.
    * Используйте mock-объекты.
    * Покрытие кода должно составлять 100%.
```sh
git submodule add https://github.com/google/googletest.git
Клонирование в «/home/HowBadCanIBe/workspace/projects/lab05/googletest»...
remote: Enumerating objects: 28085, done.
remote: Counting objects: 100% (303/303), done.
remote: Compressing objects: 100% (194/194), done.
remote: Total 28085 (delta 194), reused 114 (delta 106), pack-reused 27782 (from 4)
Получение объектов: 100% (28085/28085), 13.57 МиБ | 1.75 МиБ/с, готово.
Определение изменений: 100% (20802/20802), готово.
                                                                                                                                 
touch test.cpp
test.cpp:
'''
#include <iostream>
#include <Account.h>
#include <Transaction.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Mock-класс для Account
class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    MOCK_METHOD(void, Unlock, ());
    MOCK_METHOD(void, Lock, ());
    MOCK_METHOD(int, id, (), (const));
    MOCK_METHOD(void, ChangeBalance, (int diff));
    MOCK_METHOD(int, GetBalance, ());
};

// Mock-класс для Transaction
class MockTransaction : public Transaction {
public:
    MOCK_METHOD(bool, Make, (Account& from, Account& to, int sum));
    MOCK_METHOD(void, set_fee, (int fee));
    MOCK_METHOD(int, fee, ());
};

// Тесты для Account
TEST(Account, Balance_ID_Change) {
    MockAccount acc(1, 100);
    // Проверки вызовов методов
    acc.GetBalance();
    acc.id();
    acc.Unlock();
    acc.ChangeBalance(1000);
    acc.GetBalance();
    acc.ChangeBalance(2);
    acc.GetBalance();
    acc.Lock();
}

TEST(Account, Balance_ID_Change_2) {
    Account acc(0, 100);
    EXPECT_THROW(acc.ChangeBalance(50), std::runtime_error);
    acc.Lock();
    acc.ChangeBalance(50);
    EXPECT_EQ(acc.GetBalance(), 150);
    EXPECT_THROW(acc.Lock(), std::runtime_error);
    acc.Unlock();
}

// Тесты для Transaction
TEST(Transaction, TransTest) {
    MockTransaction trans;
    MockAccount first(1, 100), second(2, 250);
    MockAccount flat_org(3, 10000), org(4, 5000);
    
    trans.set_fee(300);
    trans.Make(first, second, 2000);
    trans.fee();
    first.GetBalance();
    second.GetBalance();
    trans.Make(org, first, 1000);
}
'''
```
3. Настройте сборочную процедуру на **TravisCI**.
```sh
mkdir -p .github/workflows
                                                                                                                                 
touch .github/workflows/main.yml
main.yml:
'''
name: bank

on:
  push:
    branches: [main]
  workflow_dispatch:

jobs:
  BuildProject:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build banking library
        run: |
          cd banking
          cmake -H. -B_build
          cmake --build _build

  Testing:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Setup environment
        run: |
          git submodule update --init
          sudo apt install lcov g++-9
      - name: Run tests
        run: |
          mkdir _build && cd _build
          CXX=/usr/bin/g++-9 cmake -DCOVERAGE=1 ..
          cmake --build .
          ./RunTest
          lcov -t "banking" -o lcov.info -c -d .
      - name: Upload coverage (initial)
        uses: coverallsapp/github-action@master
        with:
          github-token: ${{ secrets.github_token }}
          path-to-lcov: ./_build/lcov.info
      - name: Upload coverage (final)
        uses: coverallsapp/github-action@master
        with:
          github-token: ${{ secrets.github_token }}
'''
```
4. Настройте [Coveralls.io](https://coveralls.io/).
