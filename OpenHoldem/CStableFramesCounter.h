#ifndef _INC_CSTABLEFRAMESCOUNTER_H
#define _INC_CSTABLEFRAMESCOUNTER_H

extern class CStableFramesCounter
{
public:
	CStableFramesCounter();
	~CStableFramesCounter();
public:
	void ResetOnAutoplayerAction();
	int  NumberOfStableFrames() {return _NumberOfStableFrames;}
	int  UpdateNumberOfStableFrames();
private:
	void Reset();
private:
	int _NumberOfStableFrames;
} *p_stableframescounter;

#endif // _INC_CSTABLEFRAMESCOUNTER_H
