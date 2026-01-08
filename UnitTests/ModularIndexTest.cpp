#include "pch.h"
#include "CppUnitTest.h"
#include "../ReplayComms/ModularIndex.h"
#include <vector>
#include <span>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudioWritingTests 
{
	TEST_CLASS(ModularIndexTests)
	{
	public:

		TEST_METHOD(TestIncrementWrapAround)
		{
			constexpr std::pair<size_t, size_t> test_setup[] = {
				{5, 4},
				{5, 0},
				{3, 0},
				{7, 2},
				{10, 5}};


			auto test = [](size_t mod, size_t initial_index)
			{
				ModularIndex testIndex(mod, initial_index);

				// No wrap around
				for (size_t i = initial_index; i < mod; ++i)
				{
					Assert::AreEqual(i, static_cast<size_t>(testIndex));
					++testIndex;
				}

				// Wrap around
				Assert::AreEqual(size_t{0}, static_cast<size_t>(testIndex));
			};

			for (auto [mod, start] : test_setup)
			{
				test(mod, start);
			}
		}

		TEST_METHOD(TestDecrementWrapAround)
		{
			constexpr std::pair<size_t, size_t> test_setup[] = {
				{5, 4},
				{5, 0},
				{3, 0},
				{7, 2},
				{10, 5}};

			auto test = [](size_t mod, size_t initial_index)
				{
					ModularIndex testIndex(mod, initial_index);
					// No wrap around
					for (size_t i = initial_index; i > 0; --i)
					{
						Assert::AreEqual(i, static_cast<size_t>(testIndex));
						--testIndex;
					}
					Assert::AreEqual<size_t>(0, static_cast<size_t>(testIndex));
					--testIndex;
					Assert::AreEqual(size_t{ mod - 1 }, static_cast<size_t>(testIndex));
				};

			for (auto [mod, start] : test_setup)
			{
				test(mod, start);
			}
		}

		TEST_METHOD(TestAdditionWrapAround)
		{
			auto run_test = [](size_t mod, size_t initial_index, size_t increment)
			{
				ModularIndex testIndex(mod, initial_index);
				testIndex += increment;
				Assert::AreEqual<size_t>(testIndex, ModularArithmetic::add<std::ptrdiff_t>(initial_index, increment, mod));
			};

			const std::tuple<size_t, size_t, ptrdiff_t> test_cases[] = {
				{1, 0, 0},      // trivial mod = 1
				{2, 0, 1},      // small mod, simple increment
				{2, 1, 1},      // wrap-around increment
				{3, 0, 2},      // non-zero increment less than mod
				{3, 1, 2},      // wrap-around increment
				{3, 2, 2},      // wrap-around from last index
				{5, 0, 5},      // increment = mod
				{5, 2, 3},      // wrap-around increment
				{5, 4, 1},      // wrap-around from last element
				{6, 0, -1},     // negative decrement from zero
				{6, 3, -4},     // negative decrement crossing zero
				{6, 5, -5},     // negative decrement equal to mod
				{10, 0, 15},    // increment larger than mod
				{10, 7, 5},     // wrap-around increment
				{10, 9, 1},     // wrap-around from last index
			};
			for (auto [mod, start, inc] : test_cases)
				run_test(mod, start, inc);
		}

		TEST_METHOD(TestSubtractionWrapAround)
		{
			auto run_test = [](size_t mod, size_t initial_index, std::ptrdiff_t decrement)
			{
				ModularIndex testIndex(mod, initial_index);
				testIndex -= decrement;
				const size_t expected = ModularArithmetic::subtract<std::ptrdiff_t>(initial_index, decrement, mod);
				Assert::AreEqual<size_t>(testIndex, expected);
			};

			const std::tuple<size_t, size_t, ptrdiff_t> test_cases[] = {
				{1, 0, 0},      // trivial mod = 1
				{2, 0, 1},      // small mod, simple increment
				{2, 1, 1},      // wrap-around increment
				{3, 0, 2},      // non-zero increment less than mod
				{3, 1, 2},      // wrap-around increment
				{3, 2, 2},      // wrap-around from last index
				{5, 0, 5},      // increment = mod
				{5, 2, 3},      // wrap-around increment
				{5, 4, 1},      // wrap-around from last element
				{6, 0, -1},     // negative decrement from zero
				{6, 3, -4},     // negative decrement crossing zero
				{6, 5, -5},     // negative decrement equal to mod
				{10, 0, 15},    // increment larger than mod
				{10, 7, 5},     // wrap-around increment
				{10, 9, 1},     // wrap-around from last index
			};
			for (auto [mod, start, dec] : test_cases)
				run_test(mod, start, dec);
		}

		TEST_METHOD(TestAdditionSubtractionByNegatives)
		{
			auto run_test = [](size_t mod, size_t initial_index, std::ptrdiff_t change)
			{
				ModularIndex testIndex1(mod, initial_index);
				ModularIndex testIndex2(mod, initial_index);

				testIndex1 -= change;
				testIndex2 += -static_cast<long long>(change);
				Assert::AreEqual<size_t>(testIndex1, testIndex2);

				testIndex1 -= -static_cast<long long>(change);
				testIndex2 += change;
				Assert::AreEqual<size_t>(testIndex1, testIndex2);
			};

			const std::tuple<size_t, size_t, ptrdiff_t> test_cases[] = {
				{1, 0, 0},      // trivial mod = 1
				{2, 0, 1},      // small mod, simple increment
				{2, 1, 1},      // wrap-around increment
				{3, 0, 2},      // non-zero increment less than mod
				{3, 1, 2},      // wrap-around increment
				{3, 2, 2},      // wrap-around from last index
				{5, 0, 5},      // increment = mod
				{5, 2, 3},      // wrap-around increment
				{5, 4, 1},      // wrap-around from last element
				{6, 0, -1},     // negative decrement from zero
				{6, 3, -4},     // negative decrement crossing zero
				{6, 5, -5},     // negative decrement equal to mod
				{10, 0, 15},    // increment larger than mod
				{10, 7, 5},     // wrap-around increment
				{10, 9, 1},     // wrap-around from last index
			};
			for (auto [mod, start, change] : test_cases)
				run_test(mod, start, change);
		}

		TEST_METHOD(TestDifference)
		{
			auto run_test = [](size_t mod, size_t idx1, size_t idx2)
			{
				ModularIndex testIndex1(mod, idx1);
				ModularIndex testIndex2(mod, idx2);
				Assert::AreEqual<ptrdiff_t>(testIndex1 - testIndex2,
											static_cast<ptrdiff_t>(idx1) - static_cast<ptrdiff_t>(idx2));
			};

			const std::tuple<size_t, size_t, size_t> test_cases[] = {
				{1, 0, 0},    // trivial mod
				{2, 0, 0},    // equal indices
				{2, 1, 0},    // simple positive difference
				{2, 0, 1},    // simple negative difference
				{3, 0, 2},    // wrap-around negative difference
				{3, 2, 0},    // wrap-around positive difference
				{3, 1, 2},    // difference = -1
				{3, 2, 1},    // difference = +1
				{5, 0, 4},    // negative difference crossing zero
				{5, 4, 0},    // positive difference crossing zero
				{5, 2, 3},    // negative difference
				{5, 3, 2},    // positive difference
				{6, 5, 2},    // your original test case
				{6, 2, 5},    // wrap-around negative
				{6, 0, 5},    // difference from first to last
				{6, 5, 0},    // difference from last to first
				{10, 7, 3},   // general mid-range difference
				{10, 3, 7},   // negative mid-range
				{10, 9, 9},   // equal indices
				{10, 0, 5},   // crossing middle
				{10, 5, 0},   // crossing middle reverse
			};
			for (auto [mod, i1, i2] : test_cases)
				run_test(mod, i1, i2);
		}
	};
}
