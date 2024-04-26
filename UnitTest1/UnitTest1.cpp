#include "pch.h"
#include "CppUnitTest.h"
#include<iostream>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			std::vector<message> mset;
			for (int i = 1; i < 10; i++) {
				mset.push_back(message(10, i));
			}
			int time = 0;
			Assert::AreEqual(critical_check(mset, time), true);
			std::cout << time;

		}
	};
}
