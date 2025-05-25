CC=clang
CXX=clang++

all: challenge_binary challenge_unittests

CXXFLAGS_COMMON = \
	-std=c++23

# ---------------- challenge_binary ----------------

SRC_CPP = $(shell find ./src -type f -name '*.cpp' -and ! -name 'main.cpp')
SRC_HPP = $(shell find ./src -type f -name '*.hpp')

challenge_binary: $(SRC_CPP) $(SRC_HPP)
	$(CXX) -o $@ $(SRC_CPP) ./src/main.cpp

# ---------------- challenge_unittests ----------------

TESTS_CPP = $(shell find ./tests -type f -name '*.cpp')
TESTS_HPP = $(shell find ./tests -type f -name '*.hpp')

CXXFLAGS_TESTS := \
	$(CXXFLAGS_COMMON) \
	-I./src \
	$(shell pkg-config --cflags gtest_main)
LDLIBS = $(shell pkg-config --libs gtest_main)

challenge_unittests: $(TESTS_CPP) $(TESTS_HPP)
	$(CXX) $(CXXFLAGS_TESTS) -o $@ $(TESTS_CPP) $(SRC_CPP) $(LDLIBS)
