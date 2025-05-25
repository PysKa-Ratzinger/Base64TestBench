CC=clang
CXX=clang++

all: challenge_binary

ALL_SOURCES_CPP = $(shell find ./src -type f -name '*.cpp' -or -name '*.hpp')

challenge_binary: $(ALL_SOURCES_CPP)
	$(CXX) -o $@ $+

