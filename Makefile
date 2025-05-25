CC=clang
CXX=clang++

.PHONY: clean

all: challenge_binary challenge_unittests challenge_bench

CXXFLAGS_COMMON = \
	-O3 \
	-gdwarf-4 \
	-std=c++26 \
	-stdlib=libc++ \
	-march=native

# ---------------- challenge_binary ----------------

SRC_CPP = $(shell find ./src -type f -name '*.cpp' -and ! -name 'main.cpp')
SRC_HPP = $(shell find ./src -type f -name '*.hpp')

challenge_binary: $(SRC_CPP) $(SRC_HPP)
	$(CXX) $(CXXFLAGS_COMMON) -o $@ $(SRC_CPP) ./src/main.cpp

clean_challenge_binary:
	$(RM) -v ./challenge_binary

clean: clean_challenge_binary

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

# ------------------ GOOGLE BENCHMARK -----------------

LIBBENCHMARK = ./third_party/benchmark/build/src/libbenchmark.a
LIBBENCHMARK_MAIN = ./third_party/benchmark/build/src/libbenchmark_main.a

$(LIBBENCHMARK) $(LIBBENCHMARK_MAIN):
	@$(call ECHO,Building libbenchmark.a and libbenchmark_main.a)
	cd ./third_party/benchmark && \
		cmake -DBENCHMARK_USE_LIBCXX=On -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBENCHMARK_DOWNLOAD_DEPENDENCIES=On -DCMAKE_BUILD_TYPE=Release -S . -B "build" && \
		cmake --build "build" --config Release

CFLAGS-LIBBENCHMARK := -isystem ./third_party/benchmark/include/
CXXFLAGS-LIBBENCHMARK := $(CFLAGS-LIBBENCHMARK)
LDFLAGS-LIBBENCHMARK := -L./third_party/benchmark/build/src/ -l:libbenchmark.a -lpthread
LDFLAGS-LIBBENCHMARKMAIN := -L./third_party/benchmark/build/src/ -l:libbenchmark.a -l:libbenchmark_main.a -lpthread

# ---------------- challenge_unittests ----------------

TESTS_CPP = $(shell find ./tests -type f -name '*.cpp')
TESTS_HPP = $(shell find ./tests -type f -name '*.hpp')

CXXFLAGS_TESTS := \
	$(CXXFLAGS_COMMON) \
	-I./src \
	$(CXXFLAGS-LIBGTEST)
LDLIBS_TESTS = \
	$(LDFLAGS-LIBGTEST)

challenge_unittests: $(SRC_CPP) $(SRC_HPP) $(TESTS_CPP) $(TESTS_HPP) $(LIBGTEST)
	$(CXX) $(CXXFLAGS_TESTS) -o $@ $(TESTS_CPP) $(SRC_CPP) $(LDLIBS_TESTS)

clean_challenge_unittests:
	$(RM) -v ./challenge_unittests

clean: clean_challenge_unittests

# ------------------- challenge_benchmarks ------------

BENCH_CPP = $(shell find ./benchmarks -type f -name '*.cpp')
BENCH_HPP = $(shell find ./benchmarks -type f -name '*.hpp')

CXXFLAGS_BENCH := \
	$(CXXFLAGS_COMMON) \
	-I./src \
	$(CXXFLAGS-LIBBENCHMARK)
LDLIBS_BENCH = \
	$(LDFLAGS-LIBBENCHMARKMAIN)

challenge_bench: $(SRC_CPP) $(SRC_HPP) $(BENCH_CPP) $(BENCH_HPP) $(LIBBENCHMARK_MAIN)
	$(CXX) $(CXXFLAGS_BENCH) -o $@ $(BENCH_CPP) $(SRC_CPP) $(LDLIBS_BENCH)

clean_challenge_bench:
	$(RM) -v ./challenge_bench

clean: clean_challenge_bench
