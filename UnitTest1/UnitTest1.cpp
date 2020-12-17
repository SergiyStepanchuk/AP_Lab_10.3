#include "pch.h"
#include "CppUnitTest.h"
#include <fstream>
#include "../Program/main.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace UnitTest1
{
	TEST_CLASS(UnitTest1)
	{
	public:
		
		TEST_METHOD(met_get_bill_by_payer)
		{
			fstream stream("r.txt");
			float sum = -1;
			if (stream.is_open()) {
				sum = get_bill_by_payer(stream, "ror");
				stream.close();
			}
			Assert::AreEqual(sum, 334.f);
		}
	};
}
