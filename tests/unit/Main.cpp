#include "UnitTest.h"

int main(int argc, const char** argv)
{
	try {
		for (auto& test : UnitTest::GetTests()) {
			std::cerr << test->Name() << " " << std::flush;
			test->Run();
			std::cerr << " OK" << std::endl;
		}
	} catch (std::runtime_error& e) {
		std::cerr << " failed\n" << e.what() << std::endl;
		return 1;
	}
	std::cerr << "All " << UnitTest::GetTests().size() << " test(s) passed." << std::endl;
	return 0;
}
