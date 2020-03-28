

#include "DamageThreshold.h"


#include <functional>
#include <chrono>
#include <future>
#include <cstdio>

RelocPtr <uintptr_t> PreDRCalculation_HookTarget(0x00FC020F);
RelocPtr <uintptr_t> GetHardcodedAVList(0x0006B1F0);
RelocPtr <uintptr_t> PostDRCalculation_HookTarget(0x00FC022E);

float MinDTPenetrateValue = 0.2;



//struct DamageData
//{
//	UInt8       unk00[0x40];        // 00
//	UInt32      attackerHandle;     // 40
//	UInt32      targetHandle;       // 44
//	UInt8       unk48[0x58 - 0x48]; // 48
//	TESForm*    source;             // 58
//	void*       sourceInstanceData; // 60
//	UInt8       unk68[0x98 - 0x68]; // 48
//	float       damage;             // 98
//	UInt8		unk98[0x210 - 0xA0];//0A0
//	TESObjectREFR* act1;
//	UInt8		unk218;
//	TESObjectREFR* act2;
//};
//STATIC_ASSERT(offsetof(DamageData, act1) == 0x210);
//STATIC_ASSERT(offsetof(DamageData, act2) == 0x220);

struct DamageData
{
	NiPoint3                    hitLocation;                    // 00
	UInt32                      pad0C;                          // 0C
	float                       unk10[8];                       // 10
	bhkNPCollisionObject*		collisionObj;                   // 30
	UInt64                      unk38;							// 00
	UInt32						attackerHandle;					// 40
	UInt32						targetHandle;					// 44
	UInt8						unk48[0x58 - 0x48];				// 48
	TESForm*					source;							// 58
	void*						sourceInstanceData;				// 60
	UInt64                      unk68[(0x80 - 0x68) >> 3];      // 68
	TESAmmo*					ammo;                           // 80 occur when use gun.
	void*						unk88;							// 88
	float                       damage2;                        // 90 game uses this value to calc final damage.
	float                       unk94;                          // 94
	float                       damage;
};


float armorpiercingdam = 0;
float reduction = 0;
UInt32 previousHandle = 0;
float PreDRdamage = 0;
float previousDamage = 0;
TESObjectWEAP::InstanceData* weapInts = nullptr;
int i = 0;
void PreDRCalculation_Hook(DamageData* damageData, TESObjectREFR* attackTarget) {
	PreDRdamage = 0;
	armorpiercingdam = 0;
	reduction = 0;
	if (damageData && attackTarget) {
		if (DRCalculation && (*g_player)->actorValueOwner.GetValue(DRCalculation) == 0) {
			PreDRdamage = 0;
		}
		else {
			PreDRdamage = damageData->damage;
		}

		previousDamage = damageData->damage;
	}
}


float GetExactDamageByProjectileCount(float damage, TESObjectWEAP* weap, TESObjectWEAP::InstanceData* instance) {
	float finalDamage = damage;
	UInt8 projectileCount = 0;
	if (instance && instance->firingData) {
		projectileCount = instance->firingData->numProjectiles;

	}
	else {
		if (weap && weap->weapData.firingData) {
			projectileCount = weap->weapData.firingData->numProjectiles;
		}
	}
	if (projectileCount == 1) {
		return finalDamage;
	}
	if (projectileCount > 0) {

		finalDamage = damage / projectileCount;
	}
	return finalDamage;
}


//2097152 player handle
float appliedDamage = 0;
float targetDT = 0;
UInt32 prevHandle = 0;
void PostDRCalculation_Hook(DamageData* damageData, TESObjectREFR* attackTarget) {
	appliedDamage = 0;
	targetDT = 0;
	if (damageData->source) {
		TESObjectREFR* attacker = nullptr;
		UInt32 a = damageData->attackerHandle;

		LookupREFRByHandle(&a, &attacker);
		if (attacker) {
			if (attacker->baseForm) {
				if (attacker->baseForm->formType == FormType::kFormType_NPC_) {
					armorpiercingdam = attacker->actorValueOwner.GetValue(AP);
				}
			}
		}


		if (attackTarget) {
			reduction = attackTarget->actorValueOwner.GetValue(DT) - armorpiercingdam;
			if (reduction < 0) {
				reduction = 0;
			}
		}

		if (damageData->source->formType == FormType::kFormType_WEAP) {
			if (damageData && attackTarget) {
				float dam = damageData->damage;
				if (PreDRdamage != 0) {
					dam = PreDRdamage;
				}
				float toapply = 0;
				toapply = max(dam - reduction, dam * MinDTPenetrateValue);


				//toapply = GetExactDamageByProjectileCount(toapply, (TESObjectWEAP*)damageData->source, (TESObjectWEAP::InstanceData*)damageData->sourceInstanceData);

				targetDT = reduction;
				appliedDamage = toapply;
				damageData->damage = toapply;
			}
		}
	}
	/*_MESSAGE("HOOK %f Target DT | comin DR-ed damage: %f | final damage: %f", reduction, dam, damageData->damage);*/
}

// 0 - hide, 1 - broken, 2 - full
int CompareGetState(float dt, float ap, float dam) {
	if (dt <= 0) {
		return 0;
	}
	else if (dt < ap) {
		return 1;
	}
	else if (dt >= dam)
	{
		return 2;
	}
	else {
		return 0;
	}
}

bool isReady = true;
bool isPlayerReady = true;
int prevState = -1;
int prevStatePlayer = -1;

void handleAsyncDisplay(int state) {
	switch (state)
	{
	case 0:
		SetState(false, false);
		break;
	case 1:
		SetState(false, true);
		break;
	case 2:
		SetState(true, false);
		break;
	default:
		SetState(false, false);
		break;
	}

	isReady = true;


	return;
}


void handleAsyncDisplayPlayer(int state) {
	switch (state)
	{
	case 0:
		SetStatePlayer(false, false);
		break;
	case 1:
		SetStatePlayer(false, true);
		break;
	case 2:
		SetStatePlayer(true, false);
		break;
	default:
		SetStatePlayer(false, false);
		break;
	}

	isPlayerReady = true;




	return;
}


void CompareAndSet(float dt, float dam, float ap)
{
	int state = CompareGetState(dt, ap, dam);

	if (isReady) {
		isReady = false;
		//later target(100, true, &handleAsyncDisplay, state);
		handleAsyncDisplay(state);
	}
}
void CompareAndSetPlayer(float dt, float dam, float ap)
{

	int state = CompareGetState(dt, ap, dam);

	if (isPlayerReady) {
		isPlayerReady = false;
		//later player(100, true, &handleAsyncDisplayPlayer, state);
		handleAsyncDisplayPlayer(state);
	}



}

bool isVatsOpen() {
	BSFixedString bf = "VATSMenu";
	if ((*g_ui)->IsMenuOpen(bf))
		return true;
	else
		return false;

}

int prevActorId = 0;
void HitHandle(TESHitEvent* evn) {
	TESObjectREFR* target = evn->target;
	TESObjectREFR* attacker = evn->attacker;

	if (!target) {
		LookupREFRByHandle(&evn->targetHandle, &target);
	}
	if (!attacker) {
		LookupREFRByHandle(&evn->attackerHandle, &attacker);
	}

	if (evn->source) {
		if (evn->source->formType == FormType::kFormType_WEAP) {

			if (attacker && attacker == *g_player && target) {
				if (target->baseForm->formType == FormType::kFormType_NPC_) {
					CompareAndSet(target->actorValueOwner.GetValue(DT), appliedDamage, attacker->actorValueOwner.GetValue(AP));
				}
			}
			else if (target && target == *g_player && attacker) {
				CompareAndSetPlayer(target->actorValueOwner.GetValue(DT), appliedDamage, attacker->actorValueOwner.GetValue(AP));
			}
		}
	}
}


UInt32 _handle;
std::string prev = "";
int prevId = 0;
bool b = true;
bool vatsEntered = false;
void EnemyHealthHandle(HUDEnemyHealthDisplayEvent * evn) {
	if ((*g_player)->actorValueOwner.GetValue(HasLivingAnatomyPerk) > 0) {
		if (evn->unk14) {
			TESObjectREFR* target = nullptr;
			LookupREFRByHandle(&_handle, &target);
			if (target) {
				UInt32 hp = target->actorValueOwner.GetValue(HP);
				UInt32 dt = target->actorValueOwner.GetValue(DT);
				if (dt <= 0) {
					dt = 0;
				}
				if (hp <= 0) {
					hp = 0;
					HideTargetHPDT();
				}
				UpdateTargetHPDT(hp, dt);
				ShowTargetHPDT();
				vatsEntered = true;
			}
			else {
				HideTargetHPDT();
			}
		}
		else {
			if (vatsEntered) {
				vatsEntered = false;
				HideTargetHPDT();
			}
		}
	}
	else {
		/*logMessage("------------");
		logMessage("third hide");
		logMessage("------------");*/
		//HideTargetHPDT();
	}

	if (evn->show) {
		if (evn->name == nullptr) {

		}
		else
		{

			if (prev.compare(evn->name) != 0 && prev != "") {
				/*SetState(false, false);*/
				handleAsyncDisplay(0);
			}
			prev = evn->name;

		}
	}
	/*if (evn->name == nullptr) {
		handleAsyncDisplay(0);
	}*/
}

void VatsTargetHandle(UInt32 handle) {
	_handle = handle;
}


void InitDamageThresholdHooks() {
	{
		struct DamageCalculation_Code : Xbyak::CodeGenerator {
			DamageCalculation_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel, hook;

				// original code
				mulss(xmm0, ptr[r14 + 0x98]);

				// move result of DR calculation into DamageData struct
				movss(ptr[r14 + 0x98], xmm0);

				// call hook
				mov(rcx, r14);                      // arg 1: DamageData
				mov(rdx, ptr[rsp + 0x118 - 0xE8]);  // arg 2: Attack target
				call(ptr[rip + hook]);

				// move updated damage into xmm0
				movss(xmm0, ptr[r14 + 0x98]);

				// return
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(PostDRCalculation_HookTarget.GetUIntPtr() + 9);
				L(hook);
				dq((uintptr_t)PostDRCalculation_Hook);
			}
		};

		void * codeBuf = g_localTrampoline.StartAlloc();
		DamageCalculation_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write5Branch(PostDRCalculation_HookTarget.GetUIntPtr(), (uintptr_t)codeBuf);
		SafeWrite32(PostDRCalculation_HookTarget.GetUIntPtr() + 5, 0x90909090);
	}
	{
		struct DamageCalculation_Code : Xbyak::CodeGenerator {
			DamageCalculation_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label originalCall, retnLabel, hook;

				mov(rcx, r14);                      // arg 1: DamageData
				mov(rdx, ptr[rsp + 0x118 - 0xE8]);  // arg 2: Attack target
				call(ptr[rip + hook]);

				call(ptr[rip + originalCall]);      // original code

				jmp(ptr[rip + retnLabel]);          // return

				L(retnLabel);
				dq(PreDRCalculation_HookTarget.GetUIntPtr() + 5);
				L(originalCall);
				dq(GetHardcodedAVList.GetUIntPtr());
				L(hook);
				dq((uintptr_t)PreDRCalculation_Hook);
			}
		};

		void * codeBuf = g_localTrampoline.StartAlloc();
		DamageCalculation_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write5Branch(PreDRCalculation_HookTarget.GetUIntPtr(), (uintptr_t)codeBuf);
	}



}