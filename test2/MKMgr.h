#pragma once
class CMKMgr
{
public:
	CMKMgr(void);
	~CMKMgr(void);
	// your functio
	void patchAndRunMK();
	void doUpdate();
	void thanksList();
	void aboutMe();
protected:
	BOOL patchLauncher();
};

