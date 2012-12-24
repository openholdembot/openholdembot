#ifndef INC_NUMERICALFUNCTIONS_H
#define INC_NUMERICALFUNCTIONS_H

#include <assert.h>

inline void SwapDoubles(double *first, double *second)
{
	double temp = *first;
	*first = *second;
	*second = temp;
}

inline void SwapInts(int *first, int *second)
{
	int temp = *first;
	*first = *second;
	*second = temp;
}

inline int RankOfCard(int card)
{
	// Suit is stored in bits 0..3
	// Rank is stored in bits 4..7
	return ((card >>4) & 0x0F);
}

inline void AssertRange(double value, double lower_bound, double upper_bound)
{
	assert(value >= lower_bound);
	assert(value <= upper_bound);
}

int	bitcount(int bit_vector);

#endif INC_NUMERICALFUNCTIONS_H