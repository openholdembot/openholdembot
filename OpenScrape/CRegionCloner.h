#ifndef INC_C_REGIONCLONER_H
#define INC_C_REGIONCLONER_H

#include "..\CTablemap\CTablemap.h"
#include "..\OpenHoldem\MagicNumbers.h"

class CRegionCloner
{
public:
	CRegionCloner();
	~CRegionCloner();
	void CloneRegions();
private:
	bool TableSizeUndefined();
	void CloneCommonCards();
	void CloneCircularCloneableRegions();
	CString CreateName(CString prefix, int number, CString postfix);
	void CalculateLinearRegions(STablemapRegion first_region, int number_of_regions);
	void CalculateCircularRegions(STablemapRegion first_region, int unmber_of_regions);
	void ApplyNextLinearRegionPosition(STablemapRegion *new_region, int index);
	void ApplyNextCircularRegionPosition(STablemapRegion region_to_be_cloned, STablemapRegion *new_region, int index);
private:
	RECT linear_region_positions[k_max_number_of_regions_to_clone];
	RECT circular_region_positions[k_max_number_of_regions_to_clone];
};

#endif // INC_C_REGIONCLONER_H