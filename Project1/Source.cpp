#include "stdio.h"
#include "chrono"
#include "memory"
#include "MemoryPool.h"

/*This test shows performance improvment that comes
with using a memory pool as opposed to new operator.

If you want to see a difference beetwen big and small amout 
of chunks then you have to set LOOP_REPEATS to a big number
otherwise it's unnoticeable
*/

#define ARRAY_SIZE 1024
#define LOOP_REPEATS 1000
#define TEST_REPEATS 1

int main()
{
	{
		float Results[TEST_REPEATS];
		for (size_t TestIndex = 0; TestIndex < TEST_REPEATS; TestIndex++)
		{

			std::chrono::time_point<std::chrono::high_resolution_clock> Point;
			std::chrono::duration<float> Time;

			float TimeA, TimeB;

			MemoryPool* Pool = new MemoryPool();
			Pool->Initialize(4, 1024);//Chunks size and ammout of them
			int* A[ARRAY_SIZE];


			//Simulating new
			Point = std::chrono::high_resolution_clock::now();

			for (size_t I = 0; I < LOOP_REPEATS; I++)
			{
				for (size_t I = 0; I < ARRAY_SIZE; I++)
				{
					A[I] = new int();
					*A[I] = I;
				}

				for (size_t I = 0; I < ARRAY_SIZE; I++)
				{
					delete A[I];
				}
			}

			Time = std::chrono::high_resolution_clock::now() - Point;

			TimeA = Time.count();
			printf("New Time: %f", TimeA);
			//End of new


			printf("\n\n");


			//Simulating MemoryPool usage
			Point = std::chrono::high_resolution_clock::now();

			for (size_t I = 0; I < LOOP_REPEATS; I++)
			{
				for (size_t I = 0; I < ARRAY_SIZE; I++)
				{
					A[I] = (int*)Pool->Alloc();
					*A[I] = I;
				}

				for (size_t I = 0; I < ARRAY_SIZE; I++)
				{
					Pool->FreeMem(A[I]);
				}
			}

			Time = std::chrono::high_resolution_clock::now() - Point;

			TimeB = Time.count();
			printf("Pool Time: %f", TimeB);
			//End of MemoryPool

			float PercentageImprovement = TimeA / TimeB;
			Results[TestIndex] = PercentageImprovement;

			printf("\n\nMemoryPool is %f times faster.\n\n", PercentageImprovement);

			delete Pool;
		}

		float AvgResult = 0.0f;
		for (size_t TestIndex = 0; TestIndex < TEST_REPEATS; TestIndex++)
		{
			AvgResult += Results[TestIndex];
		}
		AvgResult /= TEST_REPEATS;

		printf("\nAvarage improvement: %f\n", AvgResult);

	}
	
	return 0;
}