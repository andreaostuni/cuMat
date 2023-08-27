#include "TestLinAlgOps.cuh"
TEST_CASE("dense lin-alg 3", "[Dense]")
{
	SECTION("5x5")
	{
		testlinAlgOps2<5>();
	}
}