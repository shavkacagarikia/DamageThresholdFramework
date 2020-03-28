#pragma once
#include "f4se\GameEvents.h"

#include "f4se/PluginAPI.h"
#include "f4se/GameAPI.h"
#include "f4se/GameData.h"
#include "f4se/GameReferences.h"
#include "f4se/GameForms.h"
#include "f4se/GameMenus.h"
#include "f4se/GameRTTI.h"
#include "f4se_common\SafeWrite.h"
#include "xbyak/xbyak.h"
#include "f4se_common\SafeWrite.h"
#include "f4se_common\BranchTrampoline.h"
#include "ScaleformF4NV.h"
#include "f4se/GameReferences.h"
#include <shlobj.h>
#include <string>
#include "f4se/Translation.h"
#include "f4se/ScaleformLoader.h"
#include "f4se/CustomMenu.h"
#include "f4se\GameTypes.h"

#include "Globals.h"

void HitHandle(TESHitEvent* evn);

void EnemyHealthHandle(HUDEnemyHealthDisplayEvent * evn);

void VatsTargetHandle(UInt32 handle);

void InitDamageThresholdHooks();