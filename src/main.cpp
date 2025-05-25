#include "Base64Codec.hpp"

#include <unistd.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

using MainMainFunction_t         = std::function<int(int, const char**)>;
using InitMainFunctionCallback_t = std::function<void(MainMainFunction_t)>;

const char* key2 =
        "\x84\x4d\x27\x58\xd5\x3b\xf3\xc2\xd0\x5d\x94\x6f\x93\x0b\xd3\xf0\x49\xad\x78\x38\xce\x9b\x37\xd4\x2a\x4e\x3a\x7d\x4a\xe3\x7c\xc3\x2b\xbf\x31\xf7\x50\x2f\x1d\x83\x65\x43\xff\xa6\x96\x98\x96\xfc\x89\x72\xbb\x29\x65\x65\x64\x7d\x08\xa5\x0d\x9f\x18\x66\x9a\x3d\x44\x13\x3a\x51\x6d\x5a\x9d\xf4\x05\xaa\x4b\x35\xe7\xc5\x45\x8d\xd5\x7b\xe7\x97\x7c\x03\x03\xf6\x5e\x76\xdc\x71\x56\x82\x4f\x69\x5c\xd1\x6f\xbb\xb5\x08\x75\x30\xd7\x90\x68\x80\xfb\xbb\xaf\x40\x45\xd4\x7a\x5a\xc0\x18\xc9\x7e\x6e\xe7\xdf\x6d\x9b\x1b\xbe\x25\x16\x9a\x4e\x27\x85\xe1\x40\x6b\xeb\x2e\xfc\xdb\xdf\x89\xaa\xc1\x47\xe3\x42\x6d\x0e\x78\x0f\x9f\x1e\x09\x66\xef\x8f\xcc\xf1\x2e\x25\xd7\x1f\x34\x18\xa8\xdc\xb5\x21\xf8\xd1\xdd\xfc\x64\x9d\x0e\xd6\xea\x26\x90\xd6\x3e\xed\xc1\xa3\xd8\x57\xb5\xa7\x62\x6b\xd7\x60\x45\x5e\x33\x7c\xfd\x32\xce\xc7\x74\xfc\x18\xc9\x1e\x12\x36\x7b\xac\xa3\x55\x19\x9a\x56\x02\xbb\x78\x84\xfa\x44\x91\x3d\xab\x86\x12\x44\x3f\xa2\xd1\xa4\x5c\xe0\xc9\x45\xc9\xee\x20\x77\xf6\x24\x5d\x6e\x5e\x79\xf0\x5c\x73\x2c\x36\x3b\x1b\xf8\x4c\x31\x9f\x08\x4f\x2f\xc1\x76\x5f\xdb\x07\xfc\x09\xb7\x55\x33\x2f\x87\xdf\x72\x18\xda\xa5\xb2\xaa\x3f\x28\x3f\x0e\x05\x21\x69\x4b\xf0\x14\x7f\x61\x4f\xd2\x92\xc6\x01\xec\xe1\x6b\x79\xbe\xd9\x99\xfb\xb9\xe3\x8c\x5a\x55\xd6\xcc\x0c\xdd\xe6\x82\xbd\x18\xbd\x0c\x94\x07\x02\xf4\xfd\x87\x04\x44\xea\x7a\xfa\x4b\x65\xc4\x69\x66\x89\xef\xcf\xa7\xb3\xb5\x8f\x6e\x10\xb1\x9e\xa2\x7a\xd2\xad\xb4\x3f\x0b\x2a\x77\xea\x39\xaa\xdb\x39\x8b\xce\xcc\x30\x65\xcc\x2d\xae\xae\xe1\x4c\x7a\x24\xf9\x7f\xfb\xe0\x58\x31\x84\x2d\x59\x77\x2e\xa4\x95\x89\xbe\x4d\x91\x7b\x00\x06\x90\xa9\xe5\x92\xb2\xe4\x3c\x9c\x91\xed\x45\x13\x61\xef\x4c\x34\xa9\x36\xd8\x11\xd5\xfd\x14\x81\xde\xc0\x75\xee\xa1\xf0\x83\xf8\x85\xf4\xf5\x6b\xc5\xad\x3b";

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
	if (tp1 == tp2) {
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
					        std::cout.put(
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
			        vvv, 1249608753, doThePrinting, returnError
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
