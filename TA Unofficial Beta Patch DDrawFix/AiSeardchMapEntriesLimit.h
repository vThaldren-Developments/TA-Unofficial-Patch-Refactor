#pragma once

class IncreaseAISearchMapEntriesLimit
{
public:
	DWORD m_NewLimit;
	BOOL m_Work_b;
private:
	//DWORD OrginalLimit;
	SingleHook * ModifyTheLimit;

	SingleHook * avoidOrginalTick;

	HANDLE pathFindThd;
	DWORD pathFindTID;
public:
	IncreaseAISearchMapEntriesLimit ();
	IncreaseAISearchMapEntriesLimit (DWORD NewLimit);
	~IncreaseAISearchMapEntriesLimit ();
};
