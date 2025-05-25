CC=clang
CXX=clang++

all: challenge_binary challenge_unittests

CXXFLAGS_COMMON = \
	-std=c++23 \
	-stdlib=libc++

# ---------------- challenge_binary ----------------

SRC_CPP = $(shell find ./src -type f -name '*.cpp' -and ! -name 'main.cpp')
SRC_HPP = $(shell find ./src -type f -name '*.hpp')

challenge_binary: $(SRC_CPP) $(SRC_HPP)
	$(CXX) -o $@ $(SRC_CPP) ./src/main.cpp

# ---------------- GOOGLETEST ----------------

LIBGTEST = ./third_party/googletest/build/lib/libgtest.a
LIBGMOCK = ./third_party/googletest/build/lib/libgmock.a

$(LIBGTEST) $(LIBGMOCK):
	@$(call ECHO,Building googletest library)
	cd ./third_party/googletest && \
		cmake "-DCMAKE_C_COMPILER=clang" "-DCMAKE_CXX_COMPILER=clang++" -DCMAKE_CXX_FLAGS="-std=c++11 -stdlib=libc++ -U__STRICT_ANSI__" -DCMAKE_BUILD_TYPE=Release -S . -B "build" && \
		cmake --build "build" --config Release

CFLAGS-LIBGTEST := -isystem ./third_party/googletest/googletest/include
CXXFLAGS-LIBGTEST := $(CFLAGS-LIBGTEST)
LDFLAGS-LIBGTEST := -L./third_party/googletest/build/lib/ -l:libgtest.a

# ---------------- challenge_unittests ----------------

TESTS_CPP = $(shell find ./tests -type f -name '*.cpp')
TESTS_HPP = $(shell find ./tests -type f -name '*.hpp')

CXXFLAGS_TESTS := \
	$(CXXFLAGS_COMMON) \
	-I./src \
	$(CXXFLAGS-LIBGTEST)
LDLIBS_TESTS = \
	$(LDFLAGS-LIBGTEST)

challenge_unittests: $(SRC_CPP) $(TESTS_CPP) $(TESTS_HPP) $(LIBGTEST)
	$(CXX) $(CXXFLAGS_TESTS) -o $@ $(TESTS_CPP) $(SRC_CPP) $(LDLIBS_TESTS)
