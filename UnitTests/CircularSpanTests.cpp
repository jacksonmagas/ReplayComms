#include "pch.h"
#include "CppUnitTest.h"
#include "../ReplayComms/CircularSpan.h"
#include <vector>
#include <span>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AudioWritingTests
{
	TEST_CLASS(CircularSpanTests)
	{
	public:

		TEST_METHOD(IteratesCorrectlyWithoutWrapAround)
		{
			std::vector<int> buffer = { 1, 2, 3, 4, 5 };
			CircularSpan<int> view(buffer, 1, 4); // elements: 2, 3, 4

			std::vector<int> expected = { 2, 3, 4 };
			std::vector<int> actual;

			for (auto val : view)
			{
				actual.push_back(val);
			}

			Assert::AreEqual(expected.size(), actual.size());
			for (size_t i = 0; i < expected.size(); ++i)
			{
				Assert::AreEqual(expected[i], actual[i]);
			}
		}

		TEST_METHOD(IteratesCorrectlyWithWrapAround)
		{
			std::vector<int> buffer = { 10, 20, 30, 40, 50 };
			CircularSpan<int> view(buffer, 3, 1); // elements: 40, 50, 10

			std::vector<int> expected = { 40, 50, 10 };
			std::vector<int> actual;

			for (auto val : view)
			{
				actual.push_back(val);
			}

			Assert::AreEqual(expected.size(), actual.size());
			for (size_t i = 0; i < expected.size(); ++i)
			{
				Assert::AreEqual(expected[i], actual[i]);
			}
		}

		TEST_METHOD(SizeCalculationWrapAround)
		{
			std::vector<int> buffer = { 1, 2, 3, 4, 5 };
			CircularSpan<int> view(buffer, 4, 2); // 5, 1, 2
			Assert::AreEqual<size_t>(3, view.size());
		}

		TEST_METHOD(IndexOperatorTest)
		{
			std::vector<int> buffer = { 5, 6, 7, 8, 9 };
			CircularSpan<int> view(buffer, 3, 1); // 8, 9, 5

			auto it = view.begin();
			Assert::AreEqual(8, it[0]);  // 0th element
			Assert::AreEqual(9, it[1]);  // 1st element
			Assert::AreEqual(5, it[2]);  // 2nd element
		}

		TEST_METHOD(IteratorArithmetic)
		{
			std::vector<int> buffer = { 10, 20, 30, 40, 50 };
			CircularSpan<int> view(buffer, 4, 2); // 50, 10, 20

			auto it = view.begin();
			it += 2;

			Assert::AreEqual(20, *it); // Should point to 3rd element (20)
			it -= 1;
			Assert::AreEqual(10, *it); // Back to 2nd element (10)
		}

		TEST_METHOD(IteratorComparison)
		{
			std::vector<int> buffer = { 100, 200, 300 };
			CircularSpan<int> view(buffer, 1, 1); // whole buffer

			auto it1 = view.begin();
			auto it2 = it1 + 2;

			Assert::IsTrue(it1 < it2);
			Assert::IsTrue(it2 > it1);
			Assert::IsTrue(it1 != it2);
			Assert::AreEqual(it2 - it1, ptrdiff_t(2));
		}
	};
}
