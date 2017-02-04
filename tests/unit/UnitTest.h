#ifndef UNIT_TEST_H__
#define UNIT_TEST_H__

#include <iostream>
#include <vector>
#include <stdexcept>

class UnitTest
{
public:
	UnitTest() { GetTests().push_back(this); }
	virtual ~UnitTest() { }

	virtual std::string Name() const = 0;
	virtual void Run() = 0;

	UnitTest(const UnitTest&) = delete;
	UnitTest& operator = (const UnitTest&) = delete;

	static std::vector<UnitTest*>& GetTests() {
		static std::vector<UnitTest*> tests;
		return tests;
	}
};

#define UNIT_TEST(module, test) \
	struct module##test: public UnitTest { \
		std::string Name() const { return #module "::" #test; } \
		void Run(); \
	}; \
	module##test instance##module##test; \
	void module##test::Run()

#define UNIT_ASSERT(exp) \
	do { \
		if (!(exp)) { \
			throw std::runtime_error(#exp); \
		} \
		std::cerr << "." << std::flush; \
	} while (false)

#endif
