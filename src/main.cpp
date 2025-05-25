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
	size_t hash_v = 0xdeadbeef;
	size_t p      = 31;
};

namespace
{

template<typename Fcn>
std::vector<std::byte>
deferDecoding(std::span<const char> spn, Fcn fcn)
{
	return fcn(spn);
}

template<typename Fcn>
[[clang::always_inline]]
int
explode(std::span<const char>                      spn,
        Fcn                                        fcn,
        std::function<int(std::vector<std::byte>)> callback)
{
	return callback(deferDecoding(spn, fcn));
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
initActualMainFunction(InitMainFunctionCallback_t callback)
{
	MainMainFunction_t actualMain = [](int argc, const char* argv[]) -> int
	{
		if (argc <= 0) {
			return -1;
		}

		return explode(
		        std::span(argv[0], size_t(argc)),
		        [](std::span<const char> spn)
		        { return Base64Codec::decode(spn); },
		        [](std::vector<std::byte> result)
		        {
			        CurrentHashState curr_state{};

			        std::ranges::for_each(
			                result,
			                [&](auto chr)
			                {
				                curr_state.hash_v =
				                        (curr_state.hash_v +
				                         size_t(chr)) *
				                        curr_state.p %
				                        0x7437d327;
				                curr_state.p =
				                        (curr_state.p * 31) %
				                        0x32714;
			                }
			        );

			        return compareFcn(
			                curr_state.hash_v, 1142767383,
			                [&]()
			                {
				                std::cout.write(
				                        reinterpret_cast<
				                                const char*>(
				                                result.data()
				                        ),
				                        std::streamsize(
				                                result.size()
				                        )
				                );
				                std::cout << "\n";
				                return 0;
			                },
			                []() { return 1; }
			        );
		        }
		);
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
			        char        nullarg[] = "";
			        const char* args[] = { &msg[0], &nullarg[0] };

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
