#pragma once
#include "f4se\PapyrusVM.h"
#include "f4se\PapyrusInterfaces.h"
#include "Globals.h"

template <typename T>
T GetVirtualFunction(void* baseObject, int vtblIndex) {
	uintptr_t* vtbl = reinterpret_cast<uintptr_t**>(baseObject)[0];
	return reinterpret_cast<T>(vtbl[vtblIndex]);
}

template <typename T>
T GetOffset(const void* baseObject, int offset) {
	return *reinterpret_cast<T*>((uintptr_t)baseObject + offset);
}

typedef bool(*_IKeywordFormBase_HasKeyword)(IKeywordFormBase* keywordFormBase, BGSKeyword* keyword, UInt32 unk3);

TESForm * GetFormFromIdentifier(const std::string & identifier)
{
	auto delimiter = identifier.find('|');
	if (delimiter != std::string::npos) {
		std::string modName = identifier.substr(0, delimiter);
		std::string modForm = identifier.substr(delimiter + 1);

		const ModInfo* mod = (*g_dataHandler)->LookupModByName(modName.c_str());
		if (mod && mod->modIndex != -1) {
			UInt32 formID = std::stoul(modForm, nullptr, 16) & 0xFFFFFF;
			UInt32 flags = GetOffset<UInt32>(mod, 0x334);
			if (flags & (1 << 9)) {
				// ESL
				formID &= 0xFFF;
				formID |= 0xFE << 24;
				formID |= GetOffset<UInt16>(mod, 0x372) << 12;	// ESL load order
			}
			else {
				formID |= (mod->modIndex) << 24;
			}
			return LookupFormByID(formID);
		}
	}
	return nullptr;
}



BSFixedString GetDisplayName(ExtraDataList* extraDataList, TESForm * kbaseForm)
{
	TESForm * baseForm = kbaseForm;

	if (baseForm)
	{
		if (extraDataList)
		{
			BSExtraData * extraData = extraDataList->GetByType(ExtraDataType::kExtraData_TextDisplayData);
			if (extraData)
			{
				ExtraTextDisplayData * displayText = DYNAMIC_CAST(extraData, BSExtraData, ExtraTextDisplayData);
				if (displayText)
				{
					return *CALL_MEMBER_FN(displayText, GetReferenceName)(baseForm);
				}
			}
		}

		TESFullName* pFullName = DYNAMIC_CAST(baseForm, TESForm, TESFullName);
		if (pFullName)
			return pFullName->name;
	}

	return BSFixedString();
}

//
bool HasKeyword(TESForm* form, BGSKeyword* keyword) {
	IKeywordFormBase* keywordFormBase = DYNAMIC_CAST(form, TESForm, IKeywordFormBase);
	if (keywordFormBase) {
		auto HasKeyword_Internal = GetVirtualFunction<_IKeywordFormBase_HasKeyword>(keywordFormBase, 1);
		if (HasKeyword_Internal(keywordFormBase, keyword, 0)) {
			return true;
		}
	}
	return false;
}

TESObjectWEAP::InstanceData* GetInstanceDataFromExtraDataListWEAP(ExtraDataList* extraDataList) {
	TESObjectWEAP::InstanceData* currweapInst = nullptr;

	TBO_InstanceData* neededInst = nullptr;
	if (extraDataList) {
		BSExtraData * extraData = extraDataList->GetByType(ExtraDataType::kExtraData_InstanceData);
		if (extraData) {
			ExtraInstanceData * objectModData = DYNAMIC_CAST(extraData, BSExtraData, ExtraInstanceData);
			if (objectModData)
				neededInst = objectModData->instanceData;
		}
		else {
			//_MESSAGE("Item dont have instance data");
		}
	}
	currweapInst = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(neededInst, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	return currweapInst;
}
TESObjectARMO::InstanceData* GetInstanceDataFromExtraDataListARMO(ExtraDataList* extraDataList) {
	TESObjectARMO::InstanceData* currweapInst = nullptr;

	TBO_InstanceData* neededInst = nullptr;
	if (extraDataList) {
		BSExtraData * extraData = extraDataList->GetByType(ExtraDataType::kExtraData_InstanceData);
		if (extraData) {
			ExtraInstanceData * objectModData = DYNAMIC_CAST(extraData, BSExtraData, ExtraInstanceData);
			if (objectModData)
				neededInst = objectModData->instanceData;
		}
		else {
			//_MESSAGE("Item dont have instance data");
		}
	}
	currweapInst = (TESObjectARMO::InstanceData*)Runtime_DynamicCast(neededInst, RTTI_TBO_InstanceData, RTTI_TESObjectARMO__InstanceData);
	return currweapInst;
}

//
BSFixedString GetFormDescription(TESForm * thisForm)
{
	if (!thisForm)
		return BSFixedString();

	TESDescription * pDescription = DYNAMIC_CAST(thisForm, TESForm, TESDescription);
	if (pDescription) {
		BSString str;
		CALL_MEMBER_FN(pDescription, Get)(&str, nullptr);
		return str.Get();
	}

	return BSFixedString();
}
