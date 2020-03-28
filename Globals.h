#pragma once
#include "f4se\GameForms.h"
#include "f4se\GameExtraData.h"
#include "f4se\GameObjects.h"

enum ParamType {
	BaseArmorDR,
	CapsValue,
	Weight,
	CustomValue,
	Undefined1
};

enum OperationType {
	Add,
	Subtract,
	Multiply,
	Undefined2
};



template<typename T, typename P>
T remove_if(T beg, T end, P pred)
{
	T dest = beg;
	for (T itr = beg;itr != end; ++itr)
		if (!pred(*itr))
			*(dest++) = *itr;
	return dest;
}

extern ActorValueInfo* DT;
extern ActorValueInfo* DR;
extern ActorValueInfo* AP;
extern ActorValueInfo* HP;
extern ActorValueInfo* HasLivingAnatomyPerk;
extern ActorValueInfo* UnarmedDamage;
extern BGSDamageType* DtDamType;
extern BGSDamageType* DrDamType;
extern BGSKeyword* Ignorekwd;
extern BGSKeyword* CreatureKeyword;
extern BGSKeyword* UnarmedKeyword;
extern BGSKeyword* CanUnarmedBlockKeyword;
extern ActorValueInfo* DRCalculation;
extern TESGlobal* MinDTPenetrate;


extern bool bIsDRCalculationEnabled;
extern bool bOverrideLivingAnatomy;


//Formulas
extern ParamType Param1Type;
extern OperationType Operation1Type;
extern ParamType Param2Type;
extern OperationType Operation2Type;
extern ParamType Param3Type;
extern OperationType Operation3Type;
extern ParamType Param4Type;
extern float Param1Val;
extern float Param2Val;
extern float Param3Val;
extern float Param4Val;


//Config
extern std::string OverrideEspName;
extern bool isOverrideEnabled;
//bool isOptionalPluginEnabled

extern std::string oDT;
extern std::string oAP;
extern std::string oDType;
extern std::string oLA;
extern std::string oIgnKwd;

extern float MinDTPenetrateValue; 