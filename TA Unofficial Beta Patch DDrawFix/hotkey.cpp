#include "oddraw.h"

#include "tamem.h"
#include "LimitCrack.h"
#include "UnitTypeLimit.h"

#include <vector>
using namespace std;

struct hotkeyTigger
{
	enum hotkeytype { inputtext, keyboard= 2, mouse = 4}
	type;

	union inputData
	{
		int vk_key;
		enum mouseControl { leftClick, rightClick, leftDbl, rightDbl, middleClick, wheelUp, wheelDown }
		mouse;
	} inputData0;
	union inputData inputData1;
	union inputData inputData2;

	char shortcommand[256];
};

class hotkey
{
public:
	struct hotkeyTigger tigger0;
	struct hotkeyTigger tigger1;

	void initTigger (struct hotkeyTigger & tigger, char * newshortcommand);
	void initTigger(struct hotkeyTigger & tigger, int vk_key0, int vk_key1, int vk_key2);

	////
	bool verify(char * chattext);

	bool verify();///verify whether  key is push down

	
};

class shortcommand:hotkey
{
public:
	char orgcommand[256 * 4];


};
class selectunit:hotkey, childType
{

};

class UIClicker :hotkey
{

};


class property
{
	//enum compareMethod { equal, bigger, smaller} match;
private:
	int refCount;

public:
	property() 
		: refCount (0)
	{
		;
	}
	~property()
	{
		;
	}

	virtual bool isMatch (UnitStruct * unit_p)
	{
		return false;
	}
	virtual bool isMatch (UnitDefStruct * unitdef)
	{
		return false;
	}

	virtual bool notMatch(UnitStruct * unit_p)
	{
		return !isMatch (unit_p);
	}
	virtual bool notMatch(UnitDefStruct * unitdef)
	{
		return !isMatch ( unitdef);
	}

	virtual const char * name()
	{
		return "default property";
	}
};
class INSCREEN:property
{
	bool isMatch(UnitStruct * unit_p)
	{
		return false;
	}

	const char * name()
	{
		return "INSCREEN";
	}
};

class INSELECTED :property
{
	bool isMatch(UnitStruct * unit_p)
	{
		return false;
	}
	const char * name()
	{
		return "INSELECTED";
	}
};
class ISSELECTEDTYPE :property
{
	bool isMatch(UnitStruct * unit_p)
	{
		return false;
	}

	const char * name()
	{
		return "SELECTEDTYPE";
	}
};

class CIRCLESELECT :property
{
	bool isMatch(UnitStruct * unit_p)
	{
		return false;
	}

	const char * name()
	{
		return "CIRCLESELECT";
	}
};

class CANFLY :property
{

	bool isMatch(UnitDefStruct * unitdef)
	{
		return canfly & unitdef->UnitTypeMask_0;
		
	}

	const char * name()
	{
		return "CANFLY";
	}
};
class CANMOVE :property
{
	bool isMatch(UnitDefStruct * unitdef)
	{
		return canmove & unitdef->UnitTypeMask_0;;
	}

	const char * name()
	{
		return "CANMOVE";
	}
}; 

class BUILDER :property
{
	bool isMatch(UnitDefStruct * unitdef)
	{
		return builder & unitdef->UnitTypeMask_0;
	}

	const char * name()
	{
		return "BUILDER";
	}
}; 

class ANTIWEAPONS :property
{
	bool isMatch(UnitDefStruct * unitdef)
	{
		return antiweapons & unitdef->UnitTypeMask_0;
	}

	const char * name()
	{
		return "ANTIWEAPONS";
	}
};

class COMMANDER :property
{
	bool isMatch(UnitDefStruct * unitdef)
	{
		if ((showplayername& unitdef->UnitTypeMask_1)
			&& (hidedamage& unitdef->UnitTypeMask_0))
		{// decoy and commander both set showplayername=1 and hidedamage= 1
			return true;
		}
		return false;
	}

	const char * name()
	{
		return "COMMANDER";
	}
};

class HASWEAPONS :property
{
	bool isMatch(UnitDefStruct * unitdef)
	{
		unsigned long NoWeaponPtr = reinterpret_cast<unsigned long> (NowCrackLimit->NowIncreaseWeaponTypeLimit->CurtPtr);

		if ((NoWeaponPtr != reinterpret_cast<unsigned long>(unitdef->weapon1) && (NULL != unitdef->weapon1) && (0 == (stockpile_mask&(unitdef->weapon1->WeaponTypeMask))))
			|| (NoWeaponPtr != reinterpret_cast<unsigned long>(unitdef->weapon2) && (NULL != unitdef->weapon2) && (0 == (stockpile_mask&(unitdef->weapon2->WeaponTypeMask))))
			|| (NoWeaponPtr != reinterpret_cast<unsigned long>(unitdef->weapon3) && (NULL != unitdef->weapon3) && (0 == (stockpile_mask&(unitdef->weapon3->WeaponTypeMask))))
			)
		{//////////////////////////////////////////////////////////////////////////
			// have at least one weapon, dont include stockpile weapon
			//////////////////////////////////////////////////////////////////////////
			return true;
		}
		return false;
	}

	const char * name()
	{
		return "HASWEAPONS";
	}
};
class STOCKPILEWEAPONS :property
{
	bool isMatch(UnitDefStruct * unitdef)
	{
		unsigned long NoWeaponPtr = reinterpret_cast<unsigned long> (NowCrackLimit->NowIncreaseWeaponTypeLimit->CurtPtr);

		if ((NoWeaponPtr != reinterpret_cast<unsigned long>(unitdef->weapon1) && (NULL != unitdef->weapon1) &&  (stockpile_mask&(unitdef->weapon1->WeaponTypeMask)))
			|| (NoWeaponPtr != reinterpret_cast<unsigned long>(unitdef->weapon2) && (NULL != unitdef->weapon2) &&  (stockpile_mask&(unitdef->weapon2->WeaponTypeMask)))
			|| (NoWeaponPtr != reinterpret_cast<unsigned long>(unitdef->weapon3) && (NULL != unitdef->weapon3) &&  (stockpile_mask&(unitdef->weapon3->WeaponTypeMask)))
			)
		{//////////////////////////////////////////////////////////////////////////
		 // have at least one stockpile weapon
		 //////////////////////////////////////////////////////////////////////////
			return true;
		}
		return false;
	}

	const char * name()
	{
		return "STOCKPILEWEAPONS";
	}
};

class TRANSPORTSIZE :property
{
private:
	int benchmark;
public:
	TRANSPORTSIZE(int newbenchmark)
	{
		benchmark = newbenchmark;
	}
	bool isMatch(UnitDefStruct * unitdef)
	{
		return unitdef->transportsize >= benchmark;
	}

	const char * name()
	{
		return "TRANSPORTSIZE";
	}
}; 
class BMCODE :property
{
	bool isMatch(UnitDefStruct * unitdef)// zero mean non-movable
	{
		return unitdef->bmcode;
	}

	const char * name()
	{
		return "BMCODE";
	}
}; 
class TRANSPORTCAPACITY :property
{
private:
	int benchmark;
public:
	TRANSPORTCAPACITY(int newbenchmark)
	{
		benchmark = newbenchmark;
	}

	bool isMatch(UnitDefStruct * unitdef)
	{
		return unitdef->transportcapacity>= benchmark;
	}

	const char * name()
	{
		return "TRANSPORTCAPACITY";
	}
};
class YARDMAPSIZE :property
{
private:
	int benchmark;
public:
	YARDMAPSIZE(int newbenchmark)
	{
		benchmark = newbenchmark;
	}

	bool isMatch(UnitDefStruct * unitdef)
	{
		return benchmark<=(unitdef->FootX* unitdef->FootY);
	}

	const char * name()
	{
		return "YARDMAP";
	}
};

 
class childType
{
	enum MaskWay { unitID, unitProperty} matchType;

	char unitID[32];//try to match first 32 char
	vector<property *> properties;

	//HANDLE semaphore;

	LPDWORD unitsMask;

	childType()
	{
		unitsMask = (LPDWORD)malloc(((NowCrackLimit->NowIncreaseUnitTypeLimit->CurtUnitTypeNum) / 8 / 0x40) * 0x40);
	}

	~childType()
	{
		if (unitsMask)
		{
			free ( unitsMask);
		}
	}
};
