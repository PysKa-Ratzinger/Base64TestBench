CC=clang
CXX=clang++

all: challenge_binary challenge_unittests

CXXFLAGS = $(shell pkg-config --cflags gtest_main)

LDLIBS = $(shell pkg-config --libs gtest_main)

ALL_SOURCES_CPP = $(shell find ./src -type f -name '*.cpp' -or -name '*.hpp')

UNIT_TEST_SOURCES_CPP = $(shell find ./tests -type f -name '*.cpp' -or -name '*hpp')

challenge_binary: $(ALL_SOURCES_CPP)
	$(CXX) -o $@ $+

challenge_unittests: $(UNIT_TEST_SOURCES_CPP)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LDLIBS)

