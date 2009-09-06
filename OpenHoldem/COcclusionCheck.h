#ifndef __LIB_COCCLUSIONCHECK_H

#define __LIB_COCCLUSIONCHECK_H

// Heuristic occlusion-check for rebuy.

class COcclusionCheck
{
public:
	COcclusionCheck();
	~COcclusionCheck();
public:
	bool UserBalanceOccluded();
private:
	bool UserChairKnown();
	bool UserBalanceNonZero();
	bool UserNameKnown();
	bool AnyOpponentNameKnown();
	bool AnyApponentBalanceNonZero();
};

#endif