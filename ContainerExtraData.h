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
#include "xbyak/xbyak.h"

template<typename T>
struct SimpleCollector
{
	UInt64                      unk00;
	tArray<T>                   itemsArray;
};

struct DamageInfo
{
	UInt32						type;
	float						damage;
};

struct InventoryItemStack
{
	UInt64						unk00;
	UInt64						unk08;
	UInt64						unk10;
	ExtraDataList				* extraList;
};

typedef SInt32(*_CalcDiffRating)(float value1, float value2);
RelocAddr <_CalcDiffRating> CalcDiffRating(0x0C0B750);

struct InvItemStackStruct
{
public:
	BGSInventoryItem *  invItem;
	UInt16              stackID;
};

typedef void(*_PopulateItemCardInfoList)(GFxValue* itemCard, BGSInventoryItem * itemForCard, UInt16 itemForCardstackID, SimpleCollector<InvItemStackStruct>* itemsForCompare);
RelocAddr <_PopulateItemCardInfoList> PopulateItemCardInfoList(0x0AED830);
_PopulateItemCardInfoList PopulateItemCardInfoList_Original;

struct ObjectInstanceData
{
	TESForm                        * form;
	TBO_InstanceData            * data;
};
using _CalcInstanceData = ObjectInstanceData * (*)(ObjectInstanceData & out, TESForm *, TBO_InstanceData *);
RelocAddr<_CalcInstanceData>            CalcInstanceData(0x2F7B50);



template<typename T>
inline void Register2(GFxValue * dst, const char * name, SInt32 value)
{
	GFxValue	fxValue;
	fxValue.SetInt(value);
	dst->SetMember(name, &fxValue);
}


inline void RegisterString2(GFxValue * dst, GFxMovieRoot * view, const char * name, const char * str)
{
	GFxValue	fxValue;
	view->CreateString(&fxValue, str);
	dst->SetMember(name, &fxValue);
}
class Objectinterface_x
{
public:
	void*   unk00;
	GFxMovieView*   view;
	void*   unk10;
};


template<typename T>

struct SimpleCollector2
{
	UInt64                      unk00;
	tArray<T>                   itemsArray;
};

struct DamageInfo2
{
	UInt32						type;
	float						damage;
};

struct InventoryItemStack2
{
	UInt64						unk00;
	UInt64						unk08;
	UInt64						unk10;
	ExtraDataList				* extraList;
};


BGSInventoryItem::Stack* GetStackByStackID(BGSInventoryItem * item, UInt16 stackID)
{
	BGSInventoryItem::Stack* stack = item->stack;
	if (!stack) return NULL;
	while (stackID != 0)
	{
		stack = stack->next;
		if (!stack) return NULL;
		stackID--;
	}
	return stack;
}


TBO_InstanceData* GetTBO_InstanceDataFromInventoryItem(BGSInventoryItem * item, UInt16 stackID) // 0x01A61F0
{
	BGSInventoryItem::Stack* stack = item->stack;
	if (!stack) return NULL;
	while (stackID != 0)
	{
		stack = stack->next;
		if (!stack) return NULL;
		stackID--;
	}
	if (stack)
	{
		ExtraInstanceData * eid = DYNAMIC_CAST(stack->extraData->GetByType(kExtraData_InstanceData), BSExtraData, ExtraInstanceData);
		if (eid) return eid->instanceData;
	}
	return NULL;
}


void processItemCardInfoList(GFxValue* itemCard, BGSInventoryItem * itemForCard, UInt16 itemForCardstackID, SimpleCollector<InvItemStackStruct>* itemsForCompare)
{
	GFxMovieView* view = ((Objectinterface_x*)itemCard->objectInterface)->view;
	if (!view) return;
	GFxMovieRoot* root = view->movieRoot;
	if (!root) return;
	ObjectInstanceData objInst{ nullptr, nullptr };
	CalcInstanceData(objInst, itemForCard->form, GetTBO_InstanceDataFromInventoryItem(itemForCard, itemForCardstackID));
	if (itemForCard && itemForCard->form) {
		if (itemForCard->form->formType == kFormType_WEAP)
		{
			TESObjectWEAP::InstanceData* weapData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(objInst.data, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
			if (weapData) {
				auto * actorValue2 = AP;
				if ((!actorValue2 || actorValue2->formType != kFormType_AVIF))
					return;

				if (weapData->modifiers != nullptr)
				{
					auto * pActorValues = weapData->modifiers;
					for (UInt32 i = 0; i < pActorValues->count; ++i)
					{
						ActorValueInfo * pActoValue = pActorValues->operator[](i).avInfo;
						UInt32	value = pActorValues->operator[](i).unk08;
						std::string displayString = std::to_string(value);// + "/" + std::to_string(currentActorValue);
						if (pActoValue && pActoValue == actorValue2)
						{
							GFxValue extraData;
							root->CreateObject(&extraData);
							RegisterString(&extraData, root, "text", "AP");//
							RegisterString(&extraData, root, "value", displayString.c_str());
							Register<SInt32>(&extraData, "difference", 0);

							itemCard->PushBack(&extraData);
						}
					}
				}
			}
			else {
				auto * actorValue2 = AP;
				if ((!actorValue2 || actorValue2->formType != kFormType_AVIF))
					return;

				TESObjectWEAP* weap = DYNAMIC_CAST(itemForCard->form, TESForm, TESObjectWEAP);
				if (!weap) return;
				auto * pDamageTypes = weap->weapData.damageTypes;
				if (pDamageTypes) {
					for (UInt32 i = 0; i < pDamageTypes->count; ++i)
					{

						BGSDamageType * pDamageType = pDamageTypes->operator[](i).damageType;
						UInt32	value = pDamageTypes->operator[](i).value;
						std::string displayString = std::to_string(value);// + "/" + std::to_string(currentActorValue);
						if (pDamageType && pDamageType->unk20 == actorValue2)
						{
							GFxValue extraData;
							root->CreateObject(&extraData);
							RegisterString(&extraData, root, "text", "AP");//
							RegisterString(&extraData, root, "value", displayString.c_str());
							Register<SInt32>(&extraData, "difference", 0);

							itemCard->PushBack(&extraData);
						}
					}
				}
			}
		}


		else if (itemForCard->form->formType == kFormType_ARMO)
		{
			auto * actorValue = DT;
			if ((!actorValue || actorValue->formType != kFormType_AVIF))
				return;
			TESObjectARMO::InstanceData* armoData = (TESObjectARMO::InstanceData*)Runtime_DynamicCast(objInst.data, RTTI_TBO_InstanceData, RTTI_TESObjectARMO__InstanceData);
			if (armoData) {
				auto * pDamageTypes = armoData->damageTypes;
				if (armoData->damageTypes) {
					for (UInt32 i = 0; i < pDamageTypes->count; ++i)
					{
						BGSDamageType * pDamageType = pDamageTypes->operator[](i).damageType;
						UInt32	value = pDamageTypes->operator[](i).value;
						std::string displayString = std::to_string(value);// + "/" + std::to_string(currentActorValue);
						if (pDamageType && pDamageType->unk20 == actorValue)
						{
							GFxValue extraData;
							root->CreateObject(&extraData);
							RegisterString(&extraData, root, "text", "DT");//
							RegisterString(&extraData, root, "value", displayString.c_str());
							Register<SInt32>(&extraData, "difference", 0);

							itemCard->PushBack(&extraData);
						}
					}
				}
			}
			else {
				TESObjectARMO* arm = DYNAMIC_CAST(itemForCard->form, TESForm, TESObjectARMO);
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
								GFxValue extraData;
								root->CreateObject(&extraData);
								RegisterString(&extraData, root, "text", "DT");//
								RegisterString(&extraData, root, "value", displayString.c_str());
								Register<SInt32>(&extraData, "difference", 0);

								itemCard->PushBack(&extraData);
							}
						}
					}
				}
			}
		}

	}
	// etc..
}


void PopulateItemCardInfoList_Hook(GFxValue* itemCard, BGSInventoryItem * itemForCard, UInt16 itemForCardstackID, SimpleCollector<InvItemStackStruct>* itemsForCompare) {

	PopulateItemCardInfoList_Original(itemCard, itemForCard, itemForCardstackID, itemsForCompare);
	if ((itemCard->type & 0x8F) == 9)
	{
		processItemCardInfoList(itemCard, itemForCard, itemForCardstackID, itemsForCompare);
	}
	else
	{
		GFxValue ItemCardInfoList;
		itemCard->GetMember("ItemCardInfoList", &ItemCardInfoList);
		processItemCardInfoList(&ItemCardInfoList, itemForCard, itemForCardstackID, itemsForCompare);
	}

}

void InitContainersHook() {
	{
		struct PopulateItemCardInfoList_Code : Xbyak::CodeGenerator {
			PopulateItemCardInfoList_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				mov(rax, rsp);
				mov(ptr[rax + 0x20], r9);

				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(PopulateItemCardInfoList.GetUIntPtr() + 7);
			}
		};

		void * codeBuf = g_localTrampoline.StartAlloc();
		PopulateItemCardInfoList_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		PopulateItemCardInfoList_Original = (_PopulateItemCardInfoList)codeBuf;

		g_branchTrampoline.Write6Branch(PopulateItemCardInfoList.GetUIntPtr(), (uintptr_t)PopulateItemCardInfoList_Hook);
	}
}