#include "Base64Codec.hpp"

#include <unistd.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

using MainMainFunction_t         = std::function<int(int, char**)>;
using InitMainFunctionCallback_t = std::function<void(MainMainFunction_t)>;

struct CurrentHashState
{
	size_t hash_v = 0xdeadbeef;
	size_t p      = 31;
};

namespace
{

void
initActualMainFunction(InitMainFunctionCallback_t callback)
{
	MainMainFunction_t actualMain = [](int argc, char* argv[]) -> int
	{
		auto result = Base64Codec::decode(std::span(argv[0], argc));
		CurrentHashState curr_state{};

		std::ranges::for_each(
		        result,
		        [&](auto chr)
		        {
			        curr_state.hash_v =
			                (curr_state.hash_v + size_t(chr)) *
			                curr_state.p % 0x7437d327;
			        curr_state.p = (curr_state.p * 31) % 0x32714;
		        }
		);

		if (curr_state.hash_v == 1142767383) {
			std::cout.write(
			        (const char*)result.data(), result.size()
			);
			std::cout << "\n";
			return 0;
		}

		return 1;
	};

	callback(actualMain);
}

} // anonymous namespace

extern "C"
{
	__attribute__((constructor)) void init()
	{
		initActualMainFunction(
		        [](MainMainFunction_t actualMain)
		        {
			        // Allocate msg in stack
			        char msg[] =
			                "ICAgICAgICAsICAgICBcICAgIC8gICAgICAsICAg"
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
			        char* null_arg = "";
			        char* args[]   = { &msg[0], null_arg };

			        if (actualMain(576, args) == 0) {
				        exit(0);
			        }
		        }
		);
	}
}

int
main(int argc, char* argv[])
{
	std::cout << "Hello world! :)\n";
	std::cout << "Couldn't validate your decoding... sorry :)\n";
	return 0;
}
