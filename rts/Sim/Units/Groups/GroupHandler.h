/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef	_GROUPHANDLER_H
#define	_GROUPHANDLER_H

#include <map>
#include <string>
#include <vector>
#include <set>
#include "creg/creg_cond.h"

class CGroup;

/**
 * Handles All Groups of a single team.
 */
class CGroupHandler {
private:
	CR_DECLARE(CGroupHandler);
	CGroupHandler(int teamId);
	virtual ~CGroupHandler();

public:

	/**
	 * This function initialized a singleton instance,
	 * if not yet done by a call to GetInstance()
	 */
//	static void Initialize(int teamId);
//	static CGroupHandler* GetInstance(int teamId);
//	static void Destroy(int teamId);

	void Update();
	void DrawCommands();
	void GroupCommand(int num);
	void GroupCommand(int num, const std::string& cmd);
	CGroup* CreateNewGroup();
	void RemoveGroup(CGroup* group);

	std::vector<CGroup*> groups;

	int team;
protected:
	std::vector<int> freeGroups;
	int firstUnusedGroup;
};

//extern CGroupHandler* grouphandler;
extern std::vector<CGroupHandler*> grouphandlers;

#endif	// _GROUPHANDLER_H
