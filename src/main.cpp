#include "Base64Codec.hpp"

#include <unistd.h>

#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <vector>

using MainMainFunction_t         = std::function<int(int, const char**)>;
using InitMainFunctionCallback_t = std::function<void(MainMainFunction_t)>;

const char* key2 =
        "\xc7\x02\x14\x30\x4e\xc7\xbb\x23\x27\xb4\xa4\xc0\x82\xc6\xe0\x59\x72\xeb\x76\x5b\x5f\x96\x11\xb2\x7f\x5a\x25\x2f\x72\x56\x82\x1c\x25\xb9\x5a\x8e\x88\xef\x89\x91\x2b\xc9\x75\x11\x36\x5c\x29\x3b\x71\x59\xc4\xe1\xec\xbb\x2b\x95\xd7\x0f\x86\xe8\x35\x29\xa2\x85\xeb\x6a\x2b\x0f\xef\x67\xf6\x50\x06\x38\xab\x1a\x05\x9c\xa4\x3d\x34\xfc\x1e\xcb\xd6\xd8\xc6\xf2\xe5\x6c\xd1\x61\xf4\xbe\xc6\x38\x17\x91\xaa\x08\xfc\x8e\x09\xab\xcb\xbc\xe8\xa9\x46\x7f\x80\xb9\x5a\xc7\xe1\xbc\xee\xd1\xfb\x50\x55\x60\xb6\xd1\x31\xfe\x63\x19\xe4\xac\xeb\x83\x53\x05\x34\x61\xe0\xa6\x3f\xde\x7d\xeb\xa8\x6d\x02\x32\xce\x71\x0f\x8b\x02\x39\x65\x95\x75\xdb\xc0\x15\x6b\xca\xf7\x35\xc2\x75\x6b\x39\xef\x80\x5b\xc7\x44\x1e\xeb\x09\x84\x26\x31\x08\xa5\x6b\xfe\x5a\x78\x00\xd9\x58\xc9\x64\x9b\x39\x47\xf8\x45\x26\xde\xc5\x4b\xd6\x4d\xf6\x0a\x95\xba\xd8\xe9\xa3\xcd\xbc\xc8\x4f\xee\xae\xd0\x0c\xf4\x91\x98\x77\xfe\x3c\x7f\x9b\x5a\xb7\x44\xbb\x65\xb1\x47\xac\x0c\x44\xd6\xce\xb0\xd7\x75\x54\x62\x29\xd0\xd0\x2e\x34\xd4\xdb\xef\xaa\x96\xe3\xbf\x74\x6b\x44\x6a\xc8\x57\x04\xf4\xf4\xd0\x27\x9a\x44\x3a\x9e\x12\x13\x24\x15\x70\xcf\xc9\xf6\x50\xa3\x06\xa5\x8e\x5e\x25\x24\x33\xba\x30\x53\xfc\xd9\x74\x29\xf1\xe8\x61\xbb\x79\xc6\xa1\x0e\x33\x0e\x9b\xb9\x6e\xc0\xca\x6f\x2f\xb6\x3b\x84\xa7\x79\x73\xdf\xb0\xb3\x3e\x0f\x31\xc6\xa6\x6b\x41\x15\xdc\xce\xa4\x05\xbf\xf4\x4e\x04\x29\x64\xea\x5e\x39\xd4\x94\x3f\xed\x78\x16\x1a\xab\x83\x41\xdb\x16\xaf\x14\x86\xff\x71\xe0\xff\xc1\x38\xf6\xe2\xc9\xf8\x38\x76\x4a\xce\x0b\x0d\x92\x60\x91\xdc\x3b\xf3\xa4\x85\x57\x58\xe8\x92\x65\xbd\x81\x86\x9a\xde\x06\xe3\xae\x11\x48\x60\x64\x07\x99\x3f\x27\x7a\x1a\x17\x73\x90\xdf\x04\x85\x1b\x4e\xbc\x00\x4c\x57\x8d\x76\x83\x7b\xb9\xb7\x1e\xb0\x96\xfa\xff\x6d\x1f\xf8\x91\xc8\x7e\xc5\x84\x0a";

struct CurrentHashState
{
	size_t hash_v;
	size_t p;

	CurrentHashState()
	        : hash_v(0xbadc0de)
	        , p(17)
	{}

	~CurrentHashState()
	{
		hash_v = 0xdeadbeef;
		p      = 31;
	}
};

namespace
{

template<typename Fcn>
std::vector<std::byte>
deferDecoding(std::span<const char> spn, Fcn fcn)
{
	return fcn(spn);
}

template<size_t Depth>
int
explode(std::span<const char>                                        spn,
        std::function<std::vector<std::byte>(std::span<const char>)> fcn,
        std::function<int(std::vector<std::byte>)>                   callback)
{
	if constexpr (Depth == 0) {
		return callback(deferDecoding(spn, fcn));
	} else {
		return explode<Depth - 1>(spn, fcn, callback);
	}
}

template<typename Tp1, typename Tp2, typename Fcn1, typename Fcn2>
int
compareFcn(Tp1 tp1, Tp2 tp2, Fcn1 f1, Fcn2 f2)
{
	if (tp1 != tp2) {
		return f1();
	} else {
		return f2();
	}
}

void
initActualMainFunction(
        InitMainFunctionCallback_t                 callback,
        std::function<int(std::vector<std::byte>)> arg2
)
{
	MainMainFunction_t actualMain = [&](int argc, const char* argv[]) -> int
	{
		if (argc <= 0) {
			return -1;
		}

		return explode<20>(
		        std::span(argv[0], size_t(argc)),
		        [](std::span<const char> spn)
		        { return Base64Codec::decode(spn); }, arg2
		);
	};

	callback(actualMain);
}

} // anonymous namespace

struct TheDoer
{
	TheDoer(std::function<void(CurrentHashState&)> callback,
	        CurrentHashState&                      state);

	~TheDoer();

	/// Destructor type callback
	std::function<void(CurrentHashState&)> destroy_state;
	CurrentHashState&                      curr_state;
};

TheDoer::TheDoer(
        std::function<void(CurrentHashState&)> callback,
        CurrentHashState&                      state
)
        : destroy_state(std::move(callback))
        , curr_state(state)
{
	destroy_state(curr_state);
}

TheDoer::~TheDoer()
{
	destroy_state(curr_state);
}

struct MagicPotato
{
	MagicPotato(std::function<void(CurrentHashState&)> white_rabbit)
	{
		the_doo = std::make_unique<TheDoer>(white_rabbit, curr_state);
	}

	std::unique_ptr<TheDoer> the_doo{};
	CurrentHashState         curr_state;
};

extern "C"
{
	void do_the_thing(CurrentHashState& curr_state)
	{
		auto callback = [](MainMainFunction_t actualMain)
		{
			// Allocate msg in stack
			char msg[] = "UNRg6n5wn2zjClXmGwvOiRoEcfRgxLSKJ8Hhtj0D"
			             "B9qQKF12MnFDwjD0IsLPtfaKhfLonuY+pW8WCLzq"
			             "ctcyYrGVTPuqyK0ToEnACQUBIOUt9GzZVnhwVQRU"
			             "TKbhN9yUXNntTJWrLR741A3iwz3D4G3o7zTNpmvC"
			             "rtiq/BhxznbRtK7YwgtSADAGIv5CU5g1wbVXJR1R"
			             "dMyhdqXSMeLFCgun9+OH54Ck08L11+5CsFFbLn4R"
			             "NNwGjUBLsZFH1bCevLGebfvdCt0Uhz8pkpHafZK7"
			             "SuOsWR+XR11MupsksEd+U+i3tAihpuy+AICgUVlE"
			             "BKSS62NeT+Vd+cDhlr7WSGig9L6gGid91hz+51qI"
			             "KQSRXiO3UKJh9TBoowcJHOBQ/R6fD834bhtqch6k"
			             "45wcPzdAEniy7IDHJgzVJ871mn1197lP5BEC0a8L"
			             "hINIzfReoGL42M06U8xoHXvx83yRs6P39h+eHU99"
			             "eE6b3KXBxgMQkeLMprviIBXfBfax6RkXrmx3IPq1"
			             "FgJuG3KAhxXh1E0R5fXZs9LrRQI7SDjWeUJs8e6c"
			             "JSTA7VLi6gsYqqc=";
			char nullarg[]     = "";
			const char* args[] = { &msg[0], &nullarg[0] };

			if (actualMain(576, args) == 0) {
				exit(0);
			}
		};

		auto actuallyHashTheThing =
		        [](CurrentHashState& state, std::byte chr)
		{
			state.hash_v = (state.hash_v + size_t(chr)) * state.p %
			               0x7437d327;
			state.p = (state.p * 31) % 0x32714;
		};

		auto callback2 = [&](std::vector<std::byte> result) -> int
		{
			auto doTheHash =
			        [&](CurrentHashState            state,
			            std::function<void(size_t)> op) -> size_t
			{
				std::ranges::for_each(
				        result,
				        [&](auto chr)
				        {
					        actuallyHashTheThing(state, chr);
					        op(state.hash_v);
				        }
				);

				return state.hash_v;
			};

			auto doThePrinting = [&]()
			{
				size_t i = 0;
				doTheHash(
				        curr_state,
				        [&](size_t vvv)
				        {
					        std::cout << std::format(
					                "{}",
					                char(uint8_t(key2[i]) ^
					                     uint8_t(vvv))
					        );
					        ++i;
				        }
				);

				std::cout << "\n";
				return 0;
			};

			auto returnError = []() { return 1; };

			size_t vvv = doTheHash(curr_state, [](size_t) {});

			return compareFcn(
			        vvv, 1142767383, doThePrinting, returnError
			);
		};

		initActualMainFunction(callback, callback2);
	}

	__attribute__((constructor)) void init()
	{
		MagicPotato potato([](CurrentHashState& state)
		                   { do_the_thing(state); });
	}
}

int
main(int argc, char* argv[])
{
	std::cout << "Hello world! :)\n";
	std::cout << "Couldn't validate your decoding... sorry :)\n";
	return 0;
}
