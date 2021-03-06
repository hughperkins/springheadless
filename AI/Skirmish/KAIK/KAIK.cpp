#ifdef LUA_LIB_EXT
#include <lua5.1/lua.hpp>
#else
#include "lib/lua/include/lua.h"
#include "lib/lua/include/lualib.h"
#include "lib/lua/include/lauxlib.h"
#endif

#include "KAIK.h"
#include "IncGlobalAI.h"

CR_BIND(CKAIK, );
CR_REG_METADATA(CKAIK, (
	CR_MEMBER(ai),
	CR_SERIALIZER(Serialize),
	CR_POSTLOAD(PostLoad)
));

CREX_REG_STATE_COLLECTOR(KAIK, CKAIK);
// #define CREX_REG_STATE_COLLECTOR(Name, RootClass)
//    static RootClass* Name ## State;
CKAIK* KAIKStateExt = KAIKState;



// called instead of InitAI() on load if IsLoadSupported() returns 1
void CKAIK::Load(IGlobalAICallback* callback, std::istream* ifs) { CREX_SC_LOAD(KAIK, ifs); }
void CKAIK::Save(std::ostream* ofs) { CREX_SC_SAVE(KAIK, ofs); }

void CKAIK::PostLoad(void) {
	assert((ai != NULL) && ai->Initialized());
}

void CKAIK::Serialize(creg::ISerializer* s) {
	if (ai->Initialized()) {
		for (int i = 0; i < MAX_UNITS; i++) {
			if (ai->ccb->GetUnitDef(i) != NULL) {
				// do not save non-existing units
				s->SerializeObjectInstance(ai->MyUnits[i], ai->MyUnits[i]->GetClass());

				if (!s->IsWriting()) {
					ai->MyUnits[i]->myid = i;
				}
			} else if (!s->IsWriting()) {
				ai->MyUnits[i]->myid = i;
				ai->MyUnits[i]->groupID = -1;
			}
		}

		s->SerializeObjectInstance(ai, ai->GetClass());
	}
}

void CKAIK::InitAI(IGlobalAICallback* callback, int team) {
	ai = new AIClasses(callback); ai->Init();

	std::string verMsg =
		std::string(AI_VERSION(team)) + (ai->Initialized()? " initialized successfully!": " failed to initialize");
	std::string logMsg =
		ai->Initialized()? ("logging events to " + ai->logger->GetLogName()): "not logging events";

	ai->cb->SendTextMsg(verMsg.c_str(), 0);
	ai->cb->SendTextMsg(logMsg.c_str(), 0);
	ai->cb->SendTextMsg(AI_CREDITS, 0);
}

void CKAIK::ReleaseAI() {
	delete ai; ai = NULL;
}

void CKAIK::UnitCreated(int unitID, int builderID) {
	if (ai->Initialized()) {
		ai->uh->UnitCreated(unitID);
		ai->econTracker->UnitCreated(unitID);
	}
}

void CKAIK::UnitFinished(int unit) {
	if (ai->Initialized()) {
		ai->econTracker->UnitFinished(unit);

		const int      frame = ai->cb->GetCurrentFrame();
		const UnitDef* udef  = ai->cb->GetUnitDef(unit);

		if (udef != NULL) {
			// let attackhandler handle cat_g_attack units
			if (GCAT(unit) == CAT_G_ATTACK) {
				ai->ah->AddUnit(unit);
			} else {
				ai->uh->IdleUnitAdd(unit, frame);
			}

			ai->uh->BuildTaskRemove(unit);
		}
	}
}

void CKAIK::UnitDestroyed(int unit, int attacker) {
	if (ai->Initialized()) {
		attacker = attacker;
		ai->econTracker->UnitDestroyed(unit);

		if (GUG(unit) != -1) {
			ai->ah->UnitDestroyed(unit);
		}

		ai->uh->UnitDestroyed(unit);
	}
}

void CKAIK::UnitIdle(int unit) {
	if (ai->Initialized()) {
		if (ai->uh->lastCapturedUnitFrame == ai->cb->GetCurrentFrame()) {
			if (unit == ai->uh->lastCapturedUnitID) {
				// KLOOTNOTE: for some reason this also gets called when one
				// of our units is captured (in the same frame as, but after
				// HandleEvent(AI_EVENT_UNITCAPTURED)), *before* the unit has
				// actually changed teams (ie. for any unit that is no longer
				// on our team but still registers as such)
				ai->uh->lastCapturedUnitFrame = -1;
				ai->uh->lastCapturedUnitID = -1;
				return;
			}
		}

		// AttackHandler handles cat_g_attack units
		if (GCAT(unit) == CAT_G_ATTACK && ai->MyUnits[unit]->groupID != -1) {
			// attackHandler->UnitIdle(unit);
		} else {
			ai->uh->IdleUnitAdd(unit, ai->cb->GetCurrentFrame());
		}
	}
}

void CKAIK::UnitDamaged(int damaged, int attacker, float damage, float3 dir) {
	if (ai->Initialized()) {
		attacker = attacker;
		dir = dir;
		ai->econTracker->UnitDamaged(damaged, damage);
	}
}

void CKAIK::UnitMoveFailed(int unit) {
	if (ai->Initialized()) {
		unit = unit;
	}
}



void CKAIK::EnemyEnterLOS(int enemy) {
	if (ai->Initialized()) {
		enemy = enemy;
	}
}

void CKAIK::EnemyLeaveLOS(int enemy) {
	if (ai->Initialized()) {
		enemy = enemy;
	}
}

void CKAIK::EnemyEnterRadar(int enemy) {
	if (ai->Initialized()) {
		enemy = enemy;
	}
}

void CKAIK::EnemyLeaveRadar(int enemy) {
	if (ai->Initialized()) {
		enemy = enemy;
	}
}

void CKAIK::EnemyDestroyed(int enemy, int attacker) {
	if (ai->Initialized()) {
		ai->dgunConHandler->NotifyEnemyDestroyed(enemy, attacker);
	}
}

void CKAIK::EnemyDamaged(int damaged, int attacker, float damage, float3 dir) {
	if (ai->Initialized()) {
		damaged = damaged;
		attacker = attacker;
		damage = damage;
		dir = dir;
	}
}



void CKAIK::GotChatMsg(const char* msg, int player) {
	if (ai->Initialized()) {
		msg = msg;
		player = player;
	}
}


int CKAIK::HandleEvent(int msg, const void* data) {
	if (ai->Initialized()) {
		switch (msg) {
			case AI_EVENT_UNITGIVEN:
			case AI_EVENT_UNITCAPTURED:
			{
				const ChangeTeamEvent* cte = (const ChangeTeamEvent*) data;

				const int myAllyTeamId = ai->cb->GetMyAllyTeam();
				const bool oldEnemy = !ai->cb->IsAllied(myAllyTeamId, ai->cb->GetTeamAllyTeam(cte->oldteam));
				const bool newEnemy = !ai->cb->IsAllied(myAllyTeamId, ai->cb->GetTeamAllyTeam(cte->newteam));

				if (oldEnemy && !newEnemy) {
					// unit changed from an enemy to an allied team
					// we got a new friend! :)
					EnemyDestroyed(cte->unit, -1);
				} else if (!oldEnemy && newEnemy) {
					// unit changed from an ally to an enemy team
					// we lost a friend! :(
					EnemyCreated(cte->unit);
					if (!ai->cb->UnitBeingBuilt(cte->unit)) {
						EnemyFinished(cte->unit);
					}
				}

				if (cte->oldteam == ai->cb->GetMyTeam()) {
					// we lost a unit
					UnitDestroyed(cte->unit, -1);

					// FIXME: multiple units given during same frame?
					ai->uh->lastCapturedUnitFrame = ai->cb->GetCurrentFrame();
					ai->uh->lastCapturedUnitID = cte->unit;
				} else if (cte->newteam == ai->cb->GetMyTeam()) {
					// we have a new unit
					UnitCreated(cte->unit, -1);
					if (!ai->cb->UnitBeingBuilt(cte->unit)) {
						UnitFinished(cte->unit);
						ai->uh->IdleUnitAdd(cte->unit, ai->cb->GetCurrentFrame());
					}
				}
			} break;
		}
	}

	return 0;
}




void CKAIK::Update() {
	if (ai->Initialized()) {
		const int frame = ai->cb->GetCurrentFrame();

		// call economy tracker update routine
		ai->econTracker->frameUpdate(frame);
		ai->dgunConHandler->Update(frame);

		if ((frame - ai->InitFrame()) == 1) {
			// init defense matrix
			ai->dm->Init();
		}

		if ((frame - ai->InitFrame()) > 60) {
			// call buildup manager and unit handler (idle) update routine
			ai->bu->Update(frame);
			ai->uh->IdleUnitUpdate(frame);
		}

		// call attack handler and unit handler (metal maker) update routines
		ai->ah->Update(frame);
		ai->uh->MMakerUpdate(frame);
		ai->ct->Update(frame);
	}
}
