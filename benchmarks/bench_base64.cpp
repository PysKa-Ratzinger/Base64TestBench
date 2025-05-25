#include "Base64Codec.hpp"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

class BmBase64 : public benchmark::Fixture
{
public:
	BmBase64() noexcept = default;

	void SetUp(::benchmark::State& state) override;

	struct Data
	{
		std::vector<std::byte> arr_raw;
		std::vector<char>      arr_b64;
	};

	std::optional<Data> data;
};

void
BmBase64::SetUp(::benchmark::State& state)
{
	auto n_chars = uint64_t(state.range(0));

	if (data.has_value()) {
		if (data->arr_raw.size() < n_chars) {
			data.reset();
		} else {
			return;
		}
	}

	data.emplace();

	std::string_view alpha =
	        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.!";

	data->arr_raw.resize(n_chars);
	for (size_t i = 0; i < data->arr_raw.size(); i++) {
		data->arr_raw[i] = std::byte(alpha[i % alpha.size()]);
	}

	data->arr_b64.resize(
	        Base64Codec::getEncodedLength(data->arr_raw.size())
	);

	Base64Codec::encode(data->arr_raw, data->arr_b64);
}

BENCHMARK_DEFINE_F(BmBase64, decode)
(benchmark::State& state)
{
	auto n_chars = uint64_t(state.range(0));

	{
		auto result = Base64Codec::decode(
		        std::string_view(
		                std::span(data->arr_b64).subspan(0, n_chars)
		        )
		);
		benchmark::DoNotOptimize(result);
	}

	std::vector<std::byte> result;
	size_t result_sz = Base64Codec::getDecodedLength(n_chars);
	result.resize(result_sz);

	for (auto _ : state) {
		Base64Codec::decode(
		        std::string_view(
		                std::span(data->arr_b64).subspan(0, n_chars)
		        ),
		        result
		);

		benchmark::DoNotOptimize(result);
	}
}

BENCHMARK_REGISTER_F(BmBase64, decode)
        ->DenseRange(0, 1LU * 1024, 16) // L1
        ->Iterations(10000);

BENCHMARK_REGISTER_F(BmBase64, decode)
        ->DenseRange(1LU * 1024, 32LU * 1024, 128) // L1
        ->Iterations(200);

BENCHMARK_REGISTER_F(BmBase64, decode)
        ->DenseRange(32LU * 1024, 512LU * 1024, 1024) // L2
        ->Iterations(20);

BENCHMARK_REGISTER_F(BmBase64, decode)
        ->DenseRange(512LU * 1024, 4LU * 1024 * 1024, 128LU * 1024) // L3
        ->Iterations(3);

BENCHMARK_REGISTER_F(BmBase64, decode)
        ->DenseRange(4LU * 1024 * 1024, 32LU * 1024 * 1024, 128LU * 1024) // L3
        ->Iterations(2);
