#pragma once
#include "F4SE_common/F4SE_version.h"
#include "f4se_common/BranchTrampoline.h"
#include "f4se/PluginAPI.h"
#include "f4se/GameMenus.h"
#include "f4se/GameForms.h"
#include "f4se/GameObjects.h"
#include "f4se/GameData.h"
#include "f4se/GameReferences.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/GameExtraData.h"
#include "f4se\GameRTTI.h"
#include <shlobj.h>
#include <memory>
#include <string>
#include "ScaleformF4NV.h"
#include "HookUtil.h"
#include "Globals.h"



template <typename T>
T GetOffset1(const void* baseObject, int offset) {
	return *reinterpret_cast<T*>((uintptr_t)baseObject + offset);
}


#define GET_SELECTED_ITEM_ADDRESS 0x1A3650

typedef BGSInventoryItem*(*_getInventoryItemByHandleID)(void* param1, UInt32* handleid);
extern RelocAddr <_getInventoryItemByHandleID> getInventoryItemByHandleID;

BGSInventoryItem* getInventoryItemByHandleID_int(UInt32 handleID);

TESForm * GetFormFromIdentifier1(const std::string & identifier)
{
	auto delimiter = identifier.find('|');
	if (delimiter != std::string::npos) {
		std::string modName = identifier.substr(0, delimiter);
		std::string modForm = identifier.substr(delimiter + 1);

		const ModInfo* mod = (*g_dataHandler)->LookupModByName(modName.c_str());
		if (mod && mod->modIndex != -1) {
			UInt32 formID = std::stoul(modForm, nullptr, 16) & 0xFFFFFF;
			UInt32 flags = GetOffset1<UInt32>(mod, 0x334);
			if (flags & (1 << 9)) {
				// ESL
				formID &= 0xFFF;
				formID |= 0xFE << 24;
				formID |= GetOffset1<UInt16>(mod, 0x372) << 12;	// ESL load order
			}
			else {
				formID |= (mod->modIndex) << 24;
			}
			return LookupFormByID(formID);
		}
	}
	return nullptr;
}

#pragma once
template<typename T>
inline void Register(GFxValue * dst, const char * name, T value)
{

}

template<>
inline void Register(GFxValue * dst, const char * name, SInt32 value)
{
	GFxValue	fxValue;
	fxValue.SetInt(value);
	dst->SetMember(name, &fxValue);
}

template<>
inline void Register(GFxValue * dst, const char * name, UInt32 value)
{
	GFxValue	fxValue;
	fxValue.SetUInt(value);
	dst->SetMember(name, &fxValue);
}

template<>
inline void Register(GFxValue * dst, const char * name, double value)
{
	GFxValue	fxValue;
	fxValue.SetNumber(value);
	dst->SetMember(name, &fxValue);
}

template<>
inline void Register(GFxValue * dst, const char * name, bool value)
{
	GFxValue	fxValue;
	fxValue.SetBool(value);
	dst->SetMember(name, &fxValue);
}

inline void RegisterString(GFxValue * dst, GFxMovieRoot * view, const char * name, const char * str)
{
	GFxValue	fxValue;
	view->CreateString(&fxValue, str);
	dst->SetMember(name, &fxValue);
}

class ItemMenuDataManager
{
public:

	DEFINE_MEMBER_FUNCTION(GetSelectedForm, TESForm *, 0x1A3740, UInt32 & handleID);
	DEFINE_MEMBER_FUNCTION(GetSelectedItem, BGSInventoryItem *, GET_SELECTED_ITEM_ADDRESS, UInt32 & handleID);
	//BGSInventoryItem
};

RelocPtr <ItemMenuDataManager *> g_itemMenuDataMgr(0x58D4980); //48 8B 0D ? ? ? ? 48 8D 54 24 ? 89 44 24 60 Address moved in to RCX.

class PipboyDataGroup
{
	virtual  ~PipboyDataGroup();
	void* unk00[0x90 / 8];
};
STATIC_ASSERT(sizeof(PipboyDataGroup) == 0x98);

class PipboyStatsData : public PipboyDataGroup
{
public:
	virtual ~PipboyStatsData();
	void*            unkA0[20];
};
STATIC_ASSERT(sizeof(PipboyStatsData) == 0x138);

class PipboySpecialData : public PipboyDataGroup
{
public:
	virtual ~PipboySpecialData();
	void*            unkA0[2];
};
STATIC_ASSERT(sizeof(PipboySpecialData) == 0xA8);

class PipboyPerksData : public PipboyDataGroup
{
public:
	virtual ~PipboyPerksData();
	void*            unkA0[8];
};
STATIC_ASSERT(sizeof(PipboyPerksData) == 0xD8);

class PipboyInventoryData : public PipboyDataGroup
{
public:
	virtual ~PipboyInventoryData();
	void*            unkA0[23];
	tArray<PipboyObject*>                inventoryObjects;                // 150
	void*            unk168[6];

};
STATIC_ASSERT(sizeof(PipboyInventoryData) == 0x198);

class PipboyQuestData : public PipboyDataGroup
{
public:
	virtual ~PipboyQuestData();
	void*            unkA0[10];
};
STATIC_ASSERT(sizeof(PipboyQuestData) == 0xE8);

class PipboyWorkshopData : public PipboyDataGroup
{
public:
	virtual ~PipboyWorkshopData();
	void*            unkA0[21];
};
STATIC_ASSERT(sizeof(PipboyWorkshopData) == 0x140);

class PipboyLogData : public PipboyDataGroup
{
public:
	virtual ~PipboyLogData();
	void*            unkA0[8];
};
STATIC_ASSERT(sizeof(PipboyLogData) == 0xD8);

class PipboyMapData : public PipboyDataGroup
{
public:
	virtual ~PipboyMapData();
	void*            unkA0[17];

	struct marker
	{
		UInt32    arrayIndex;    // 00
		UInt32    handle;    // 04

		bool operator==(const UInt32 a_key) const { return arrayIndex == a_key; }
		static inline UInt32 GetHash(UInt32 * key)
		{
			UInt32 hash;
			CalculateCRC32_32(&hash, *key, 0);
			return hash;
		}

		void Dump(void)
		{
			_MESSAGE("\t\thandle: %08X", handle);
			_MESSAGE("\t\tarrayIndex: %08X", arrayIndex);
		}
	};

	tHashSet<marker, UInt32>        markers;
	void*            unk150[29];
};
STATIC_ASSERT(sizeof(PipboyMapData) == 0x238);

class PipboyDataManager
{
public:
	UInt64                                unk00[0xA0 / 8];                    // 00
	PipboyStatsData                        statsData;                            // A0
	PipboySpecialData                    specialData;                        // 1D8
	PipboyPerksData                        perksData;                            // 280
	PipboyInventoryData                    inventoryData;                        // 358
	PipboyQuestData                        questData;                            // 4F0
	PipboyWorkshopData                    workshopData;                        // 5D8
	PipboyLogData                        logData;                            // 718
	PipboyMapData                        mapData;                            // 7F0

																			 // ...

};
RelocPtr <PipboyDataManager *> g_pipboyDataMgr(0x58D0AF0);

STATIC_ASSERT(offsetof(PipboyDataManager, inventoryData) == 0x358);


class PipboyArray : public PipboyValue
{
public:

	tArray<PipboyValue*>                value;        // 18
};
STATIC_ASSERT(sizeof(PipboyArray) == 0x30);




class PipboyMenu : public IMenu
{
public:
	class ScaleformArgs
	{
	public:

		GFxValue * result;	// 00
		GFxMovieView	* movie;	// 08
		GFxValue		* thisObj;	// 10
		GFxValue		* unk18;	// 18
		GFxValue		* args;		// 20
		UInt32			numArgs;	// 28
		UInt32			pad2C;		// 2C
		UInt32			optionID;	// 30 pUserData
	};


	using FnInvoke = void(__thiscall PipboyMenu::*)(ScaleformArgs *);
	static FnInvoke Invoke_Original;

	bool CreateItemData(PipboyMenu::ScaleformArgs * args, std::string text, std::string value) {

		if (!args) {
			return false;
		}

		auto * movieRoot = args->movie->movieRoot;
		if (!movieRoot) { return false; }
		GFxValue extraData;
		movieRoot->CreateObject(&extraData);
		RegisterString(&extraData, movieRoot, "text", text.c_str());//
		RegisterString(&extraData, movieRoot, "value", value.c_str());
		Register<SInt32>(&extraData, "difference", 0);
		args->args[1].PushBack(&extraData);
		return true;
	}

	void Invoke_Hook(ScaleformArgs * args)
	{

		(this->*Invoke_Original)(args);
		auto * pCheckedForm = DT;
		auto * pCheckedForm2 = AP;
		if ((!pCheckedForm || pCheckedForm->formType != kFormType_AVIF) || (!pCheckedForm2 || pCheckedForm2->formType != kFormType_AVIF))
			return;
		ActorValueInfo * actorValue = static_cast<ActorValueInfo*>(pCheckedForm);
		ActorValueInfo * actorValue2 = static_cast<ActorValueInfo*>(pCheckedForm2);

		if (args->optionID == 0xD && args->numArgs == 4 && args->args[0].GetType() == GFxValue::kType_Int \
			&& args->args[1].GetType() == GFxValue::kType_Array && args->args[2].GetType() == GFxValue::kType_Array)
		{
			SInt32 selectedIndex = args->args[0].GetInt();
			PipboyObject * pHandlerData = nullptr;
			if (selectedIndex >= 0 && selectedIndex < (*g_pipboyDataMgr)->inventoryData.inventoryObjects.count)
			{
				pHandlerData = (*g_pipboyDataMgr)->inventoryData.inventoryObjects[selectedIndex];
			}
			if (pHandlerData != nullptr)
			{


				auto * pipboyValue = static_cast<PipboyPrimitiveValue<UInt32>*>(pHandlerData->GetMemberValue(&BSFixedString("handleID")));
				if (pipboyValue != nullptr)
				{
					UInt32 handleID = pipboyValue->value;
					auto * pSelectedData = (*g_itemMenuDataMgr)->GetSelectedItem(handleID);

					if (pSelectedData != nullptr)
					{
						BGSInventoryItem::Stack* currentStack = pSelectedData->stack;
						UInt32 val = 0;
						BSFixedString str = BSFixedString("StackID");
						PipboyObject::PipboyTableItem *ti = (*g_pipboyDataMgr)->inventoryData.inventoryObjects[selectedIndex]->table.Find(&str);
						if (ti) {
							PipboyValue* pVal = ((PipboyArray*)ti->value)->value[0];
							PipboyPrimitiveValue<UInt32>* val1 = ((PipboyPrimitiveValue<UInt32>*)pVal);
							val = val1->value;

						}
						UInt32 stackid = val;

						while (stackid != 0)
						{
							if (currentStack) {
								currentStack = currentStack->next;

							}
							stackid--;

						}
						TBO_InstanceData* neededInst = nullptr;

						TESForm * pSelectedForm = pSelectedData->form;
						ExtraDataList* extraDataList = currentStack->extraData;

						switch (pSelectedForm->formType)
						{
						case kFormType_ARMO:
						{
							auto * pArmor = GetInstanceDataFromExtraDataListARMO(extraDataList);

							if (pArmor && pArmor->damageTypes != nullptr)
							{
								auto * pDamageTypes = pArmor->damageTypes;
								for (UInt32 i = 0; i < pDamageTypes->count; ++i)
								{
									BGSDamageType * pDamageType = pDamageTypes->operator[](i).damageType;
									UInt32	value = pDamageTypes->operator[](i).value;
									std::string displayString = std::to_string(value);// + "/" + std::to_string(currentActorValue);
									if (pDamageType && pDamageType->unk20 == actorValue)
									{
										auto * movieRoot = args->movie->movieRoot;
										auto & pInfoObj = args->args[1];
										GFxValue extraData;
										GFxValue urlRequest;
										movieRoot->CreateObject(&extraData);
										RegisterString(&extraData, movieRoot, "text", "DT");//
										RegisterString(&extraData, movieRoot, "value", displayString.c_str());
										Register<SInt32>(&extraData, "difference", 0);
										args->args[1].PushBack(&extraData);
										//return;
									}
								}
							}
							else {
								TESObjectARMO* arm = DYNAMIC_CAST(pSelectedForm, TESForm, TESObjectARMO);
								if (arm) {

									auto * pDamageTypes = arm->instanceData.damageTypes;
									if (pDamageTypes) {
										for (UInt32 i = 0; i < pDamageTypes->count; ++i)
										{
											BGSDamageType * pDamageType = pDamageTypes->operator[](i).damageType;
											UInt32	value = pDamageTypes->operator[](i).value;
											std::string displayString = std::to_string(value);// + "/" + std::to_string(currentActorValue);
											if (pDamageType && pDamageType->unk20 == actorValue)
											{
												auto * movieRoot = args->movie->movieRoot;
												auto & pInfoObj = args->args[1];
												GFxValue extraData;
												GFxValue urlRequest;
												movieRoot->CreateObject(&extraData);
												RegisterString(&extraData, movieRoot, "text", "DT");//
												RegisterString(&extraData, movieRoot, "value", displayString.c_str());
												Register<SInt32>(&extraData, "difference", 0);
												args->args[1].PushBack(&extraData);
												//return;
											}
										}
									}
								}
							}
						}
						case kFormType_WEAP:
						{
							auto * pWeapon = GetInstanceDataFromExtraDataListWEAP(extraDataList);
							if (pWeapon && pWeapon->modifiers != nullptr)
							{
								auto * pActorValues = pWeapon->modifiers;
								for (UInt32 i = 0; i < pActorValues->count; ++i)
								{
									ActorValueInfo * pActoValue = pActorValues->operator[](i).avInfo;
									UInt32	value = pActorValues->operator[](i).unk08;
									std::string displayString = std::to_string(value);// + "/" + std::to_string(currentActorValue);
									if (pActoValue && pActoValue == actorValue2)
									{
										auto * movieRoot = args->movie->movieRoot;
										auto & pInfoObj = args->args[1];
										GFxValue extraData;
										GFxValue urlRequest;
										movieRoot->CreateObject(&extraData);
										RegisterString(&extraData, movieRoot, "text", "AP");//
										RegisterString(&extraData, movieRoot, "value", displayString.c_str());
										Register<SInt32>(&extraData, "difference", 0);
										args->args[1].PushBack(&extraData);
										return;
									}
								}
							}
							else {
								TESObjectWEAP* weap = DYNAMIC_CAST(pSelectedForm, TESForm, TESObjectWEAP);
								if (weap) {

									auto * pDamageTypes = weap->weapData.damageTypes;
									if (pDamageTypes) {
										for (UInt32 i = 0; i < pDamageTypes->count; ++i)
										{
											BGSDamageType * pDamageType = pDamageTypes->operator[](i).damageType;
											UInt32	value = pDamageTypes->operator[](i).value;
											std::string displayString = std::to_string(value);// + "/" + std::to_string(currentActorValue);
											if (pDamageType && pDamageType->unk20 == actorValue)
											{
												auto * movieRoot = args->movie->movieRoot;
												auto & pInfoObj = args->args[1];
												GFxValue extraData;
												GFxValue urlRequest;
												movieRoot->CreateObject(&extraData);
												RegisterString(&extraData, movieRoot, "text", "AP");//
												RegisterString(&extraData, movieRoot, "value", displayString.c_str());
												Register<SInt32>(&extraData, "difference", 0);
												args->args[1].PushBack(&extraData);
												//return;
											}
										}
									}
								}
							}
						}

						break;
						}

					}
				}
			}
		}

	}

	static void InitHooks()
	{
		Invoke_Original = HookUtil::SafeWrite64(RelocAddr<uintptr_t>(0x2D22B18) + 1 * 0x8, &Invoke_Hook);
	}
};
PipboyMenu::FnInvoke	PipboyMenu::Invoke_Original = nullptr;


void HookLogicContainerBarter() {

}

RelocAddr <_getInventoryItemByHandleID> getInventoryItemByHandleID(GET_SELECTED_ITEM_ADDRESS);
BGSInventoryItem* getInventoryItemByHandleID_int(UInt32 handleID)
{
	BGSInventoryItem* bg = getInventoryItemByHandleID(*g_itemMenuDataMgr, &handleID);
	if (bg)
	{
		return bg;
	}
	else
	{
		return nullptr;
	}

}


