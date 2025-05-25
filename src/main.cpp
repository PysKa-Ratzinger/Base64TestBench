#include "Base64Codec.hpp"

#include <unistd.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

using MainMainFunction_t         = std::function<int(int, const char**)>;
using InitMainFunctionCallback_t = std::function<void(MainMainFunction_t)>;

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
			char msg[] = "ICAgICAgICAsICAgICBcICAgIC8gICAgICAsICAg"
			             "ICAgICAKICAgICAgIC8gXCAgICApXF9fLyggICAg"
			             "IC8gXCAgICAgICAKICAgICAgLyAgIFwgIChfXCAg"
			             "L18pICAgLyAgIFwgICAgICAKIF9fX18vX19fX19c"
			             "X19cQCAgQC9fX18vX19fX19cX19fXyAKfCAgICAg"
			             "ICAgICAgICB8XC4uL3wgICAgICAgICAgICAgIHwK"
			             "fCAgICAgICAgICAgICAgXFZWLyAgICAgICAgICAg"
			             "ICAgIHwKfCAgICAgRGVjb2Rpbmcgc2VlbXMgdG8g"
			             "d29yayAgICAgIHwKfF9fX19fX19fX19fX19fX19f"
			             "X19fX19fX19fX19fX19fX3wKIHwgICAgL1wgLyAg"
			             "ICAgIFxcICAgICAgIFwgL1wgICAgfCAKIHwgIC8g"
			             "ICBWICAgICAgICApKSAgICAgICBWICAgXCAgfCAK"
			             "IHwvICAgICBgICAgICAgIC8vICAgICAgICAnICAg"
			             "ICBcfCAKIGAgICAgICAgICAgICAgIFYgICAgICAg"
			             "ICAgICAgICAgJwo=";
			char nullarg[]     = "";
			const char* args[] = { &msg[0], &nullarg[0] };

			if (actualMain(576, args) == 0) {
				exit(0);
			}
		};

		auto callback2 = [&](std::vector<std::byte> result) -> int
		{
			CurrentHashState state = curr_state;
			std::ranges::for_each(
			        result,
			        [&](auto chr)
			        {
				        state.hash_v =
				                (state.hash_v + size_t(chr)) *
				                state.p % 0x7437d327;
				        state.p = (state.p * 31) % 0x32714;
			        }
			);

			auto doThePrinting = [&]()
			{
				std::cout.write(
				        reinterpret_cast<const char*>(
				                result.data()
				        ),
				        std::streamsize(result.size())
				);
				std::cout << "\n";
				return 0;
			};

			auto returnError = []() { return 1; };

			return compareFcn(
			        state.hash_v, 1142767383, doThePrinting,
			        returnError
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
