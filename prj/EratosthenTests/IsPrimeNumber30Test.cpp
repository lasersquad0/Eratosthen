
#include "pch.h"

uint64_t KNOWN_DIV_TEST[30] = { 0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0 };

#define FIRST_PRIMES_CNT 96
uint64_t FIRST_PRIMES[FIRST_PRIMES_CNT] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 
                79, 83, 89, 97,	101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 
	            193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 
	            317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 
	            457, 461, 463, 467, 479, 487, 491, 499, 503
				};

/////////////////////////////////////////////////////////////////////////
//   uint64_t
////////////////////////////////////////////////////////////////////////

TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test1) 
{
//	EXPECT_DEATH(IsPrimeNumber30(0, 0, 0), ""); // stop must be < number

	for (size_t i = 0; i < 30; i++)
	{
		EXPECT_EQ(KNOWN_DIV_TEST[i], IsPrimeNumber30(i, 0ull, 0));
	}
}

TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test2)
{
	for (size_t i = 0; i < 30; i++)
	{
		EXPECT_EQ(KNOWN_DIV_TEST[i], IsPrimeNumber30(i, 0ull, 0));
	}
}

TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test3)
{
	for (size_t i = 0; i < 30; i++)
	{
		for (size_t j = 0; j < 30; j++)
		{
			for (size_t k = 0; k < 30; k++)
			{
				EXPECT_EQ(KNOWN_DIV_TEST[i], IsPrimeNumber30(i, j, k));
			}
		}
	}
}

TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test4)
{
	EXPECT_EQ(0, IsPrimeNumber30(31, 30, 30));
	EXPECT_EQ(0, IsPrimeNumber30(37, 30, 30));
	EXPECT_EQ(0, IsPrimeNumber30(41, 30, 30));
	EXPECT_EQ(0, IsPrimeNumber30(43, 30, 30));
	EXPECT_EQ(0, IsPrimeNumber30(49, 30, 30));
	EXPECT_EQ(0, IsPrimeNumber30(53, 30, 30));
	EXPECT_EQ(0, IsPrimeNumber30(59, 30, 30));

	EXPECT_EQ(0, IsPrimeNumber30(31, 31, 31));
	EXPECT_EQ(0, IsPrimeNumber30(31, 32, 32));

	EXPECT_EQ(2, IsPrimeNumber30(30, 1, 0));
	EXPECT_EQ(5, IsPrimeNumber30(32535635, 2, 1));
	EXPECT_EQ(0, IsPrimeNumber30(307, 10, 5));

	// 1000000000039 is prime number
	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 10, 100));
	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 10, 1000000039));
	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 1000000039, 1000000039)); // when stop=1000000000039 calcs take too long time
	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 10, 10000040));
	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 1000000040, 1000000040));
	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 1000000040, 1000000039));
	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 1000000039, 1000000040));

	EXPECT_EQ(0, IsPrimeNumber30(1000000000039, 10, (uint64_t)sqrt(1000000000039)));
	EXPECT_EQ(2, IsPrimeNumber30(1000000000040, 0ull, 1000000000039));
	EXPECT_EQ(2, IsPrimeNumber30(1000000000040, 1, 1000000000039));
	EXPECT_EQ(2, IsPrimeNumber30(1000000000040, 2, 1000000000039));
	EXPECT_EQ(2, IsPrimeNumber30(1000000000040, 3, 1000000000039));
	EXPECT_EQ(2, IsPrimeNumber30(1000000000040, 10, 1000000000039));
	EXPECT_EQ(2, IsPrimeNumber30(1000000000040, 29, 1000000000039));
	EXPECT_EQ(17573, IsPrimeNumber30(1000000000040, 30, 1000000000039));
	EXPECT_EQ(17573, IsPrimeNumber30(1000000000040, 31, 1000000000039));
}

TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test5)
{
	uint64_t RESULTS[61] = { 0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0,
		 2, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 7, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0, 2
	};

	for (size_t i = 30; i < 61; i++)
	{
		for (size_t j = 0; j < 60; j++)
		{
			for (size_t k = 0; k < 60; k++)
			{
				if (j < 30 || (j > k && k < 30))
					EXPECT_EQ(RESULTS[i], IsPrimeNumber30(i, j, k));
				else
					EXPECT_EQ(0, IsPrimeNumber30(i, j, k));
			}
		}
	}
}

// check various lengths of FIRST_PRIMES array
TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test6)
{
	uint64_t RESULTS[61] = { 0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0,
							 2, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 7, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0, 2
						    };

	for (size_t i = 0; i < 61; i++)
		for (size_t j = 0; j < FIRST_PRIMES_CNT; j++)
			EXPECT_EQ(RESULTS[i], IsPrimeNumber30(i, FIRST_PRIMES, j));
}

// check that primes from FIRST_PRIMES are primes...
TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test7)
{
	for (size_t j = 0; j < FIRST_PRIMES_CNT; j++)
		EXPECT_EQ(0, IsPrimeNumber30(FIRST_PRIMES[j], FIRST_PRIMES, FIRST_PRIMES_CNT));
}

TEST(TestIsPrimeNumber30UInt64, IsPrimeNumber30_Test8)
{
	EXPECT_EQ(113, IsPrimeNumber30(113*241, FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(191, IsPrimeNumber30(503*191, FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(43,  IsPrimeNumber30(43*47,   FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(433, IsPrimeNumber30(433*449, FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(101, IsPrimeNumber30(101*101, FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(3,   IsPrimeNumber30(3*5*7*11, FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(13, IsPrimeNumber30(377*409, FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(379, IsPrimeNumber30(379 * 409, FIRST_PRIMES, FIRST_PRIMES_CNT));
	EXPECT_EQ(0,   IsPrimeNumber30(1000000000039, FIRST_PRIMES, FIRST_PRIMES_CNT));
}


/////////////////////////////////////////////////////////////////////////
//   BigInt
////////////////////////////////////////////////////////////////////////

typedef GenericBigInt<BigInt> TTestBigInt;
typedef GenericBigInt<TTMathBigInt> TTestMathBigInt;

TTestBigInt KNOWN_DIV_BIGINT[30] = { 0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0 };
TTestMathBigInt KNOWN_DIV_MATHBIGINT[30] = { 0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0 };


TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test1)
{
	TTestMathBigInt zero1(0), zero2(0);
	for (size_t i = 0; i < 30; i++)
	{
		EXPECT_EQ(KNOWN_DIV_MATHBIGINT[i], IsPrimeNumber30<TTestMathBigInt>(i, zero1, zero2, nullptr));
	}
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test2)
{
	for (size_t i = 0; i < 30; i++)
	{
		EXPECT_EQ(KNOWN_DIV_MATHBIGINT[i], IsPrimeNumber30<TTestMathBigInt>(i, nullptr));
	}
}


TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test3)
{
	for (size_t i = 0; i < 30; i++)
	{
		for (size_t j = 0; j < 30; j++)
		{
			for (size_t k = 0; k < 30; k++)
			{
				EXPECT_EQ(KNOWN_DIV_MATHBIGINT[i], IsPrimeNumber30<TTestMathBigInt>(i, j, k, nullptr));
			}
		}
	}
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test4)
{
	TTestMathBigInt ZERO(0), TWO(2), FIVE(5);

	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(31, 1, 0, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(31, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(37, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(41, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(43, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(49, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(53, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(59, 30, 30, nullptr));

	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(31, 31, 31, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(31, 32, 32, nullptr));

	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(30, 1, 0, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(32, 1, 0, nullptr));
	EXPECT_EQ(FIVE, IsPrimeNumber30<TTestMathBigInt>(32535635, 2, 1, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(307, 10, 5, nullptr));

	// 1000000000039 is prime number
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 10, 100, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 10, 100000039, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 100000039, 100000039, nullptr)); // when stop=1000000000039 calcs take too long time
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 10, 100000040, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 100000040, 100000040, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 100000040, 100000039, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 100000039, 100000040, nullptr));
	
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestMathBigInt>(1000000000039, 10, (uint64_t)sqrt(1000000000039), nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(1000000000040, ZERO, 1000000000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(1000000000040, 1, 1000000000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(1000000000040, 2, 1000000000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(1000000000040, 3, 1000000000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(1000000000040, 10, 1000000000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestMathBigInt>(1000000000040, 29, 1000000000039, nullptr));
	EXPECT_EQ(TTestMathBigInt(17573), IsPrimeNumber30<TTestMathBigInt>(1000000000040, 30, 1000000000039, nullptr));
	EXPECT_EQ(TTestMathBigInt(17573), IsPrimeNumber30<TTestMathBigInt>(1000000000040, 31, 1000000000039, nullptr));
	
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test5)
{
	TTestMathBigInt RESULTS[61] = { 0, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 0, 2, 0,
		 2, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 7, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0, 2
	};

	for (size_t i = 30; i < 61; i++)
	{
		for (TTestMathBigInt j = 0; j < 60; j++)
		{
			for (TTestMathBigInt k = 0; k < 60; k++)
			{
				if (j < 30 || (j > k && k < 30))
				{
					EXPECT_EQ(RESULTS[i], IsPrimeNumber30<TTestMathBigInt>(i, j, k, nullptr));
				}
				else
					EXPECT_EQ(TTestMathBigInt(0), IsPrimeNumber30<TTestMathBigInt>(i, j, k, nullptr));
			}
		}
	}
}

// check various lengths of FIRST_PRIMES array
TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test6)
{
	TTestMathBigInt RESULTS[61] = { 0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0,
							 2, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 7, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0, 2
	};

	for (size_t i = 0; i < 61; i++)
		for (size_t j = 0; j < FIRST_PRIMES_CNT; j++)
			EXPECT_EQ(RESULTS[i], IsPrimeNumber30<TTestMathBigInt>(i, FIRST_PRIMES, j, nullptr));
}

// check that primes from FIRST_PRIMES are primes...
TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test7)
{
	TTestMathBigInt zero(0);
	for (size_t j = 0; j < FIRST_PRIMES_CNT; j++)
	{
		EXPECT_EQ(zero, IsPrimeNumber30<TTestMathBigInt>(FIRST_PRIMES[j], FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	}
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test8)
{
	EXPECT_EQ(TTestMathBigInt(113), IsPrimeNumber30<TTestMathBigInt>(113 * 241,     FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(191), IsPrimeNumber30<TTestMathBigInt>(503 * 191,     FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(43),  IsPrimeNumber30<TTestMathBigInt>(43 * 47,       FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(433), IsPrimeNumber30<TTestMathBigInt>(433 * 449,     FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(101), IsPrimeNumber30<TTestMathBigInt>(101 * 101,     FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(5),   IsPrimeNumber30<TTestMathBigInt>(5 * 7 * 11,    FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(13),  IsPrimeNumber30<TTestMathBigInt>(377 * 409,     FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(379), IsPrimeNumber30<TTestMathBigInt>(379 * 409,     FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestMathBigInt(0),   IsPrimeNumber30<TTestMathBigInt>(1000000000039, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
}


////////////////////////////////////////////////////////

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test101)
{
	TTestBigInt zero1(0), zero2(0);
	for (size_t i = 0; i < 30; i++)
	{
		EXPECT_EQ(KNOWN_DIV_BIGINT[i], IsPrimeNumber30<TTestBigInt>(i, zero1, zero2, nullptr));
	}
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test102)
{
	for (size_t i = 0; i < 30; i++)
	{
		EXPECT_EQ(KNOWN_DIV_BIGINT[i], IsPrimeNumber30<TTestBigInt>(i, nullptr));
	}
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test103)
{
	for (size_t i = 0; i < 30; i++)
	{
		for (size_t j = 0; j < 30; j++)
		{
			for (size_t k = 0; k < 30; k++)
			{
				EXPECT_EQ(KNOWN_DIV_BIGINT[i], IsPrimeNumber30<TTestBigInt>(i, j, k, nullptr));
			}
		}
	}
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test104)
{
	TTestBigInt ZERO(0), TWO(2), FIVE(5);

	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(31, 1, 0, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(31, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(37, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(41, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(43, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(49, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(53, 30, 30, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(59, 30, 30, nullptr));

	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(31, 31, 31, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(31, 32, 32, nullptr));

	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(30, 1, 0, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(32, 1, 0ull, nullptr));
	EXPECT_EQ(FIVE, IsPrimeNumber30<TTestBigInt>(32535635, 2, 1, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(307, 10, 5, nullptr));

	// 1000000000039 is prime number
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 10, 100, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 10, 10000039, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 100000039, 100000039, nullptr)); // when stop=1000000000039 calcs take too long time
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 10, 10000040, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 100000040, 100000040, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 100000040, 100000039, nullptr));
	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 100000039, 100000040, nullptr));

	EXPECT_EQ(ZERO, IsPrimeNumber30<TTestBigInt>(1000000000039, 10, (uint64_t)sqrt(1000000000039), nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(1000000000040, ZERO, 100000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(1000000000040, 1, 100000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(1000000000040, 2, 100000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(1000000000040, 3, 100000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(1000000000040, 10, 100000039, nullptr));
	EXPECT_EQ(TWO, IsPrimeNumber30<TTestBigInt>(1000000000040, 29, 100000039, nullptr));
	EXPECT_EQ(TTestBigInt(17573), IsPrimeNumber30<TTestBigInt>(1000000000040, 30, 1000000000039, nullptr));
	EXPECT_EQ(TTestBigInt(17573), IsPrimeNumber30<TTestBigInt>(1000000000040, 31, 1000000000039, nullptr));

}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test105)
{
	TTestBigInt RESULTS[61] = { 0, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 0, 2, 0,
		 2, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 7, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0, 2
	};

	for (size_t i = 30; i < 61; i++)
	{
		for (TTestBigInt j = 0; j < 60; j++)
		{
			for (TTestBigInt k = 0; k < 60; k++)
			{
				if (j < 30 || (j > k && k < 30))
				{
					EXPECT_EQ(RESULTS[i], IsPrimeNumber30<TTestBigInt>(i, j, k, nullptr));
				}
				else
					EXPECT_EQ(TTestBigInt(0), IsPrimeNumber30<TTestBigInt>(i, j, k, nullptr));
			}
		}
	}
}


// check various lengths of FIRST_PRIMES array
TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test106)
{
	TTestBigInt RESULTS[61] = { 0, 0, 0, 0, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0,
							 2, 0, 2, 3, 2, 5, 2, 0, 2, 3, 2, 0, 2, 0, 2, 3, 2, 0, 2, 7, 2, 3, 2, 0, 2, 5, 2, 3, 2, 0, 2
	};

	for (size_t i = 0; i < 61; i++)
		for (size_t j = 0; j < FIRST_PRIMES_CNT; j++)
			EXPECT_EQ(RESULTS[i], IsPrimeNumber30<TTestBigInt>(i, FIRST_PRIMES, j, nullptr));
}

// check that primes from FIRST_PRIMES are primes...
TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test107)
{
	TTestBigInt zero(0);
	for (size_t j = 0; j < FIRST_PRIMES_CNT; j++)
	{
		EXPECT_EQ(zero, IsPrimeNumber30<TTestBigInt>(FIRST_PRIMES[j], FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	}
}

TEST(TestIsPrimeNumber30BigInt, IsPrimeNumber30_Test108)
{
	EXPECT_EQ(TTestBigInt(113), IsPrimeNumber30<TTestBigInt>(113 * 241, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(191), IsPrimeNumber30<TTestBigInt>(503 * 191, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(43), IsPrimeNumber30<TTestBigInt>(43 * 47, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(433), IsPrimeNumber30<TTestBigInt>(433 * 449, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(101), IsPrimeNumber30<TTestBigInt>(101 * 101, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(5), IsPrimeNumber30<TTestBigInt>(5 * 7 * 11, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(13), IsPrimeNumber30<TTestBigInt>(377 * 409, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(379), IsPrimeNumber30<TTestBigInt>(379 * 409, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
	EXPECT_EQ(TTestBigInt(0ull), IsPrimeNumber30<TTestBigInt>(1000000000039, FIRST_PRIMES, FIRST_PRIMES_CNT, nullptr));
}
