#include "f4se/PluginAPI.h"
#include "f4se/GameAPI.h"
#include "f4se/GameData.h"
#include "f4se/GameReferences.h"
#include "f4se/GameForms.h"
#include "f4se/GameMenus.h"
#include "f4se/GameRTTI.h"
#include "f4se_common\SafeWrite.h"
#include <shlobj.h>
#include <string>
#include "xbyak/xbyak.h"
#include "f4se_common\BranchTrampoline.h"
#include "main.h"
#include "ScaleformF4NV.h"
#include "f4se_common/f4se_version.h"
#include "f4se/PapyrusVM.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusEvents.h"
#include "f4se\GameExtraData.h"
#include "Globals.h"
#include "Utils.h"
#include "f4se/Translation.h"
#include "f4se/ScaleformLoader.h"
#include "f4se/CustomMenu.h"
#include "f4se\GameTypes.h"
#include "PipboyExtraData.h"
#include "ContainerExtraData.h"
#include "DamageThreshold.h"

bool formulasFound = false;;

std::string mName = "DamageThresholdFramework";
std::string espName = "DamageThresholdFramework.esm";
std::string espOptName = "DTFOptionalPlugin.esp";

ActorValueInfo* DT;
ActorValueInfo* DR;
ActorValueInfo* AP;
ActorValueInfo* HP;
ActorValueInfo* HasLivingAnatomyPerk;
ActorValueInfo* UnarmedDamage;
BGSDamageType* DtDamType;
BGSDamageType* DrDamType;
BGSKeyword* Ignorekwd;
BGSKeyword* CreatureKeyword;
ActorValueInfo* DRCalculation;
BGSKeyword* UnarmedKeyword;
BGSKeyword* CanUnarmedBlockKeyword;
TESGlobal* MinDTPenetrate;


PluginHandle			    g_pluginHandle = kPluginHandle_Invalid;
F4SEMessagingInterface		* g_messaging = nullptr;
F4SEPapyrusInterface   *g_papyrus = NULL;

F4SEScaleformInterface		*g_scaleform = NULL;
F4SESerializationInterface	*g_serialization = NULL;




BSTEventDispatcher<void*>* GetGlobalEventDispatcher(BSTGlobalEvent* globalEvents, const char * dispatcherName)
{
	for (int i = 0; i < globalEvents->eventSources.count; i++) {
		const char* name = GetObjectClassName(globalEvents->eventSources[i]) + 15;    // ?$EventSource@V
		if (strstr(name, dispatcherName) == name) {
			return &globalEvents->eventSources[i]->eventDispatcher;
		}
	}
	return nullptr;
}
#define GET_EVENT_DISPATCHER(EventName) (BSTEventDispatcher<EventName>*) GetGlobalEventDispatcher(*g_globalEvents, #EventName);
//
//STATIC_ASSERT(sizeof(BSTEventSource<void*>) == 0x70);



void logMessage(std::string aString)
{
	_MESSAGE(("[" + mName + "] " + aString).c_str());
}


void ShowNotification(std::string asNotificationText) {
	CallGlobalFunctionNoWait1<BSFixedString>("Debug", "Notification", BSFixedString(asNotificationText.c_str()));
}



#pragma region DTStuff 


class HitEventSink : public BSTEventSink<TESHitEvent>
{
public:
	virtual	EventResult	ReceiveEvent(TESHitEvent * evn, void * dispatcher)
	{

		HitHandle(evn);
		return kEvent_Continue;
	};
};
HitEventSink hitEventSink;



class EnemyHealthBarHandler : public BSTEventSink<HUDEnemyHealthDisplayEvent>
{
public:
	virtual ~EnemyHealthBarHandler() { };

	virtual    EventResult    ReceiveEvent(HUDEnemyHealthDisplayEvent * evn, void * dispatcher) override
	{
		EnemyHealthHandle(evn);
		return kEvent_Continue;
	}
};
EnemyHealthBarHandler hudEnemyHealthDisplaySink;

struct VATSCommandTargetEvent
{
	UInt32                targetRefHandle;        // 00
};

class VATSTargetEvent : public BSTEventSink<VATSCommandTargetEvent>
{
public:
	virtual ~VATSTargetEvent() { };

	virtual    EventResult    ReceiveEvent(VATSCommandTargetEvent * evn, void * dispatcher) override
	{
		VatsTargetHandle(evn->targetRefHandle);
		return kEvent_Continue;
	}
};
VATSTargetEvent vatsTargetSink;





double eval(std::string expr)
{
	std::string xxx; // Get Rid of Spaces
	for (int i = 0; i < expr.length(); i++)
	{
		if (expr[i] != ' ')
		{
			xxx += expr[i];
		}
	}

	//Fuck parenthesis but let it be here
	std::string tok = "";
	for (int i = 0; i < xxx.length(); i++)
	{
		if (xxx[i] == '(')
		{
			int iter = 1;
			std::string token;
			i++;
			while (true)
			{
				if (xxx[i] == '(')
				{
					iter++;
				}
				else if (xxx[i] == ')')
				{
					iter--;
					if (iter == 0)
					{
						i++;
						break;
					}
				}
				token += xxx[i];
				i++;
			}
			//_MESSAGE(" << token << ")" << " == " << to_string(eval(token));
			tok += std::to_string(eval(token));
		}
		tok += xxx[i];
	}

	for (int i = 0; i < tok.length(); i++)
	{
		if (tok[i] == '+')
		{
			return eval(tok.substr(0, i)) + eval(tok.substr(i + 1, tok.length() - i - 1));
		}
		else if (tok[i] == '-')
		{
			return eval(tok.substr(0, i)) - eval(tok.substr(i + 1, tok.length() - i - 1));
		}
	}

	for (int i = 0; i < tok.length(); i++)
	{
		if (tok[i] == '*')
		{
			return eval(tok.substr(0, i)) * eval(tok.substr(i + 1, tok.length() - i - 1));
		}
		else if (tok[i] == '/')
		{
			return eval(tok.substr(0, i)) / eval(tok.substr(i + 1, tok.length() - i - 1));
		}
	}

	return std::stod(tok.c_str()); // daabrune eg dedanaqachi
}

float GetValueFromFormula(TESObjectARMO* armorFormInstance) {
	std::string param1 = "";
	std::string param2 = "";
	std::string param3 = "";
	std::string param4 = "";
	std::string op1 = "";
	std::string op2 = "";
	std::string op3 = "";

	if (Param1Type == ParamType::BaseArmorDR) {
		UInt16 v = armorFormInstance->instanceData.armorRating;
		param1 = std::to_string(v);
	}
	else if (Param1Type == ParamType::CapsValue) {
		UInt32 v = armorFormInstance->instanceData.value;
		param1 = std::to_string(v);
	}
	else if (Param1Type == ParamType::Weight) {
		float v = armorFormInstance->instanceData.weight;
		param1 = std::to_string(v);
	}
	else {
		param1 = std::to_string(Param1Val);
	}

	if (Param2Type == ParamType::BaseArmorDR) {
		UInt16 v = armorFormInstance->instanceData.armorRating;
		param2 = std::to_string(v);
	}
	else if (Param2Type == ParamType::CapsValue) {
		UInt32 v = armorFormInstance->instanceData.value;
		param2 = std::to_string(v);
	}
	else if (Param2Type == ParamType::Weight) {
		float v = armorFormInstance->instanceData.weight;
		param2 = std::to_string(v);
	}
	else {

		param2 = std::to_string(Param2Val);
	}

	if (Param3Type == ParamType::BaseArmorDR) {
		UInt16 v = armorFormInstance->instanceData.armorRating;
		param3 = std::to_string(v);
	}
	else if (Param3Type == ParamType::CapsValue) {
		UInt32 v = armorFormInstance->instanceData.value;
		param3 = std::to_string(v);
	}
	else if (Param3Type == ParamType::Weight) {
		float v = armorFormInstance->instanceData.weight;
		param3 = std::to_string(v);
	}
	else {
		param3 = std::to_string(Param3Val);
	}

	if (Param4Type == ParamType::BaseArmorDR) {
		UInt16 v = armorFormInstance->instanceData.armorRating;
		param4 = std::to_string(v);
	}
	else if (Param4Type == ParamType::CapsValue) {
		UInt32 v = armorFormInstance->instanceData.value;
		param4 = std::to_string(v);
	}
	else if (Param4Type == ParamType::Weight) {
		float v = armorFormInstance->instanceData.weight;
		param4 = std::to_string(v);
	}
	else {
		param4 = std::to_string(Param4Val);
	}

	if (Operation1Type == OperationType::Add) {
		op1 = "+";
	}
	else if (Operation1Type == OperationType::Subtract) {
		op1 = "-";
	}
	else if (Operation1Type == OperationType::Multiply) {
		op1 = "*";
	}
	else {
		_MESSAGE("Problem with op");
	}

	if (Operation2Type == OperationType::Add) {
		op2 = "+";
	}
	else if (Operation2Type == OperationType::Subtract) {
		op2 = "-";
	}
	else if (Operation2Type == OperationType::Multiply) {
		op2 = "*";
	}
	else {
		_MESSAGE("Problem with op");
	}

	if (Operation3Type == OperationType::Add) {
		op3 = "+";
	}
	else if (Operation3Type == OperationType::Subtract) {
		op3 = "-";
	}
	else if (Operation3Type == OperationType::Multiply) {
		op3 = "*";
	}
	else {
		_MESSAGE("Problem with op");
	}

	std::string concString = param1 + op1 + param2 + op2 + param3 + op3 + param4;
	/*
	char tab2[1024];
	strncpy(tab2, concString.c_str(), sizeof(tab2));
	tab2[sizeof(tab2) - 1] = 0;*/


	float val = eval(concString);
	return val;
}

#pragma endregion
int armorcounter = 0;
void ArmorsDTAdder() {
	auto mod = (*g_dataHandler)->LookupLoadedModByName(espOptName.c_str());

	if (mod) {
		logMessage("Optional plugin enabled");
		tArray<TESObjectARMO*> armorForms = (*g_dataHandler)->arrARMO;
		TESObjectARMO* currArmr;
		for (size_t i = 0; i < armorForms.count; i++)
		{
			bool checker = false;
			currArmr = armorForms[i];
			if (currArmr) {
				UInt32 armrDR = currArmr->instanceData.armorRating;
				if (currArmr && !HasKeyword(currArmr, Ignorekwd)) {
					tArray<TESObjectARMO::InstanceData::DamageTypes>* a = currArmr->instanceData.damageTypes;
					if (a != nullptr) {
						for (size_t i = 0; i < a->count; i++)
						{
							TESObjectARMO::InstanceData::DamageTypes curr;
							a->GetNthItem(i, curr);
							if (curr.damageType == DtDamType) {
								checker = true;
							}
						}
						if (!checker) {
							TESObjectARMO::InstanceData::DamageTypes c;
							UInt16 dr = currArmr->instanceData.armorRating;
							float wgtht = currArmr->instanceData.weight;

							float fin = dr * 0.2;

							if (formulasFound) {
								fin = GetValueFromFormula(currArmr);
							}
							UInt32 ceiled = ceil(fin);

							c.damageType = DtDamType;
							c.value = ceiled;
							currArmr->instanceData.damageTypes->Push(c);
							armorcounter++;
							if (dr >= 1000) {
								logMessage("More than thousand dr, the value was:");
								logMessage(std::to_string(dr).c_str());
								logMessage(currArmr->fullName.name.c_str());
								currArmr->formID;
								ModInfo* mi = currArmr->GetLastModifiedMod();
								if (mi) {
									logMessage(mi->name);
								}
							}
						}
					}
					else {
						void * ptr = Heap_Allocate(sizeof(tArray<TESObjectARMO::InstanceData::DamageTypes*>));
						if (ptr) {
							tArray<TESObjectARMO::InstanceData::DamageTypes> * pInstance = new (ptr) tArray<TESObjectARMO::InstanceData::DamageTypes>();
							currArmr->instanceData.damageTypes = pInstance;
							TESObjectARMO::InstanceData::DamageTypes d;
							UInt16 dr = currArmr->instanceData.armorRating;
							float fin = dr * 0.2;

							if (formulasFound) {
								fin = GetValueFromFormula(currArmr);
							}
							UInt32 ceiled = ceil(fin);
							if (ceiled > 100) {

							}
							d.damageType = DtDamType;
							d.value = ceiled;
							currArmr->instanceData.damageTypes->Push(d);
							armorcounter++;
							if (dr >= 1000) {
								logMessage("More than thousand dr, the value was:");
								logMessage(std::to_string(dr).c_str());
								logMessage(currArmr->fullName.name.c_str());
								ModInfo* mi = currArmr->GetLastModifiedMod();
								if (mi) {
									logMessage(mi->name);
								}
							}
						}
					}
				}
			}
		}
		logMessage("Armors succesfully handled, number of armor forms with dt:");
		logMessage(std::to_string(armorcounter));
		armorcounter = 0;
		//
		/*tArray<TESObjectWEAP*> weapons = (*g_dataHandler)->arrWEAP;
		TESObjectWEAP* currweap = nullptr;
		for (size_t i = 0; i < weapons.count; i++)
		{
		currweap = weapons[i];
		if (currweap) {
		if (currweap->fullName.name) {
		std::string nam = currweap->fullName.name;
		std::transform(nam.begin(), nam.end(), nam.begin(), ::tolower);
		if (HasKeyword(currweap, UnarmedKeyword) && !HasKeyword(currweap, CanUnarmedBlockKeyword)) {
		bool found = false;
		tArray<TBO_InstanceData::ValueModifier>* modifs = currweap->weapData.modifiers;
		if (modifs) {
		for (size_t i = 0; i < modifs->count; i++)
		{
		if (modifs->entries) {
		if (modifs->entries[i].avInfo == AP) {
		found = true;
		}
		}
		}
		}
		if (!found) {
		if (modifs) {
		TBO_InstanceData::ValueModifier topush;
		topush.avInfo = AP;
		UInt32 ceiled = ceil(5 + (currweap->weapData.baseDamage * 0.1));
		topush.unk08 = ceiled;
		modifs->Push(topush);
		}
		else {
		void * ptr = Heap_Allocate(sizeof(tArray<TBO_InstanceData::ValueModifier*>));
		tArray<TBO_InstanceData::ValueModifier> * pMod = new (ptr) tArray<TBO_InstanceData::ValueModifier>();
		currweap->weapData.modifiers = pMod;
		TBO_InstanceData::ValueModifier topush;
		topush.avInfo = AP;
		UInt32 ceiled = ceil(5 + (currweap->weapData.baseDamage * 0.1));
		topush.unk08 = ceiled;
		currweap->weapData.modifiers->Push(topush);
		}
		}
		}
		}
		}
		}*/
		//

		tArray<TESNPC*> actorBases = (*g_dataHandler)->arrNPC_;

		TESNPC* currnpc;
		//_MESSAGE(std::to_string(actorBases.count).c_str());
		int counter = 0;
		for (size_t i = 0; i < actorBases.count; i++)
		{
			currnpc = actorBases[i];
			if (currnpc) {
				float currdr = currnpc->actorValueOwner.GetBase(DR);
				float fin = currdr * 0.2;
				UInt32 ceiled = ceil(fin);
				currnpc->actorValueOwner.SetBase(DT, ceiled);
				if (currnpc->race.race) {
					if (HasKeyword(currnpc->race.race, CreatureKeyword) || HasKeyword(currnpc, CreatureKeyword)) {
						if (currnpc->propertySheet.sheet) {
							for (size_t i = 0; i < currnpc->propertySheet.sheet->count; i++)
							{
								BGSPropertySheet::AVIFProperty avifProp;
								currnpc->propertySheet.sheet->GetNthItem(i, avifProp);
								if (avifProp.actorValue) {
									if (avifProp.actorValue == UnarmedDamage) {
										float val = avifProp.value;
										BGSPropertySheet::AVIFProperty newAvifProp;
										newAvifProp.actorValue = AP;

										float fin2 = avifProp.value * 0.1;
										UInt32 ceiled = ceil(fin2);
										newAvifProp.value = ceiled;
										currnpc->propertySheet.sheet->Push(newAvifProp);

									}
								}
							}
						}
					}
				}
				armorcounter++;
			}


		}
		logMessage("NPC Forms handled, number of forms handled:");
		logMessage(std::to_string(armorcounter));
		armorcounter = 0;


		tArray<TESLevCharacter*> leveledBases = (*g_dataHandler)->arrLVLN;
		TESLevCharacter* currLeveled;


		for (size_t i = 0; i < leveledBases.count; i++)
		{
			currLeveled = leveledBases[i];
			if (currLeveled) {
				if (currLeveled->leveledList.entries) {
					TESForm* f = currLeveled->leveledList.entries->form;
					if (f) {
						if (f->formType == FormType::kFormType_NPC_) {
							TESNPC *curract = DYNAMIC_CAST(f, TESForm, TESNPC);
							if (curract) {
								float currdr = curract->actorValueOwner.GetBase(DR);
								float fin = currdr * 0.2;
								UInt32 ceiled = ceil(fin);
								curract->actorValueOwner.SetBase(DT, ceiled);
								if (curract->race.race) {
									if (HasKeyword(curract->race.race, CreatureKeyword) || HasKeyword(curract, CreatureKeyword)) {
										if (curract->propertySheet.sheet) {
											for (size_t i = 0; i < curract->propertySheet.sheet->count; i++)
											{
												BGSPropertySheet::AVIFProperty avifProp;
												if (curract->propertySheet.sheet) {
													curract->propertySheet.sheet->GetNthItem(i, avifProp);
													if (avifProp.actorValue) {
														if (avifProp.actorValue == UnarmedDamage) {
															float val = avifProp.value;
															BGSPropertySheet::AVIFProperty newAvifProp;
															newAvifProp.actorValue = AP;

															float fin2 = avifProp.value * 0.1;
															if (fin2 > 0) {
															}
															UInt32 ceiled = ceil(fin2);
															newAvifProp.value = ceiled;
															curract->propertySheet.sheet->Push(newAvifProp);
														}
													}
												}
											}
										}
									}
								}
								armorcounter++;
							}
						}
					}
				}
			}
		}
		logMessage("LeveledNPC Forms handled, number of forms handled:");
		logMessage(std::to_string(armorcounter));
		armorcounter = 0;
	}
	else {
		logMessage("Optional plugin not enabled");
	}
}


bool oncePerSession = false;

bool RegisterAfterLoadEvents() {
	auto eventDispatcher4 = GET_EVENT_DISPATCHER(VATSCommandTargetEvent);
	if (eventDispatcher4) {
		eventDispatcher4->AddEventSink(&vatsTargetSink);
	}
	else {
		return false;
	}

	auto eventDispatcher2 = GET_EVENT_DISPATCHER(HUDEnemyHealthDisplayEvent);
	if (eventDispatcher2) {
		eventDispatcher2->AddEventSink(&hudEnemyHealthDisplaySink);
	}
	else {
		return false;
	}
	auto eventDispatcher = GetEventDispatcher<TESHitEvent>();
	if (eventDispatcher) {
		eventDispatcher->eventSinks.Push(&hitEventSink);
	}
	else {
		return false;
	}
	return true;
}

class TESLoadGameHandler : public BSTEventSink<TESLoadGameEvent>
{
public:
	virtual ~TESLoadGameHandler() { };
	virtual    EventResult    ReceiveEvent(TESLoadGameEvent * evn, void * dispatcher) override
	{
		HideTargetHPDT();
		if (!oncePerSession) {
			if (!RegisterAfterLoadEvents()) {
				_WARNING("Events weren't registered correctly, mod won't work");
			}
			if (MinDTPenetrate) {
				MinDTPenetrateValue = MinDTPenetrate->value;
			}
			oncePerSession = true;
		}
		return kEvent_Continue;
	}
};



void GameDataReady()
{
	static auto pLoadGameHandler = new TESLoadGameHandler();
	GetEventDispatcher<TESLoadGameEvent>()->AddEventSink(pLoadGameHandler);

}



bool Defaults() {
	bool result = true;
	auto mod = (*g_dataHandler)->LookupLoadedModByName(espName.c_str());
	if (mod) {
		DT = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier(espName + "|F99"));
		if (!DT) {
			result = false;
			logMessage("Unable to get default DT");

		}
		MinDTPenetrate = reinterpret_cast<TESGlobal*>(GetFormFromIdentifier(espName + "|173E"));
		if (!MinDTPenetrate) {
			return false;
		}
		//armorpiercing
		AP = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier(espName + "|F9A"));
		if (!AP) {
			result = false;
			logMessage("Unable to get default ap");
		}

		DtDamType = reinterpret_cast<BGSDamageType*>(GetFormFromIdentifier(espName + "|173C"));
		if (!DtDamType) {
			result = false;
			logMessage("Unable to get default dt damage type");
		}

		Ignorekwd = reinterpret_cast<BGSKeyword*>(GetFormFromIdentifier(espName + "|FA0"));
		if (!Ignorekwd) {
			result = false;
			logMessage("Unable to get default ignore keyword");
		}
		HasLivingAnatomyPerk = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier(espName + "|173B"));
		if (!HasLivingAnatomyPerk) {
			result = false;
			logMessage("Unable to get default living anatomy avif");
		}
	}
	else {
		logMessage("Unable to find DamageThresholdFramework.esm loaded");
	}
	return result;
}


bool GetForms() {
	if (DTFGetConfigFormulas("", "")) {
		formulasFound = true;
	}
	logMessage("Trying to get forms");
	//damagethreshold
	DR = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier("Fallout4.esm|2E3"));
	if (!DR) {
		return false;
	}
	DrDamType = reinterpret_cast<BGSDamageType*>(GetFormFromIdentifier("Fallout4.esm|60A87"));
	if (!DrDamType) {
		return false;
	}
	HP = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier("Fallout4.esm|2D4"));
	if (!HP) {
		_MESSAGE("Unable to get HP, mod wont work");
		return false;
	}
	if (!LoadSettings()) {
		logMessage("Unable to load any settings files, using default values from DamageThresholdFramework.esm");
		if (!Defaults()) {
			logMessage("Unable to use default values, mod wont work");
		}
	}
	else {
		logMessage("Loading DTF Forms From" + OverrideEspName);
		auto mod = (*g_dataHandler)->LookupLoadedModByName(espName.c_str());
		DT = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier(OverrideEspName + "|" + oDT));
		if (!DT) {
			return false;
			logMessage("Unable to get dt from:" + OverrideEspName + ". Make sure form id is specified correctly in .ini");
		}
		MinDTPenetrate = reinterpret_cast<TESGlobal*>(GetFormFromIdentifier(espName + "|173E"));
		if (!MinDTPenetrate) {
			return false;
		}
		//armorpiercing
		AP = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier(OverrideEspName + "|" + oAP));
		if (!AP) {
			return false;
			logMessage("Unable to get ap from:" + OverrideEspName + ". Make sure form id is specified correctly in .ini");
		}

		DtDamType = reinterpret_cast<BGSDamageType*>(GetFormFromIdentifier(OverrideEspName + "|" + oDType));
		if (!DtDamType) {
			return false;
			logMessage("Unable to get dt dam type from:" + OverrideEspName + ". Make sure form id is specified correctly in .ini");
		}

		Ignorekwd = reinterpret_cast<BGSKeyword*>(GetFormFromIdentifier(OverrideEspName + "|" + oIgnKwd));
		if (!Ignorekwd) {
			return false;
			logMessage("Unable to get ignore kyeword from:" + OverrideEspName + ". Make sure form id is specified correctly in .ini");
		}
		HasLivingAnatomyPerk = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier(OverrideEspName + "|" + oLA));
		if (!HasLivingAnatomyPerk) {
			return false;
			logMessage("Unable to get living anatomy avif from:" + OverrideEspName + ". Make sure form id is specified correctly in .ini");
		}
	}

	auto mod = (*g_dataHandler)->LookupLoadedModByName(espOptName.c_str());
	if (mod) {
		//get optional esp forms from here
		CreatureKeyword = reinterpret_cast<BGSKeyword*>(GetFormFromIdentifier("Fallout4.esm|13795"));
		UnarmedDamage = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier("Fallout4.esm|2DF"));
		DRCalculation = reinterpret_cast<ActorValueInfo*>(GetFormFromIdentifier(espOptName + "|4FED"));
		UnarmedKeyword = reinterpret_cast<BGSKeyword*>(GetFormFromIdentifier("Fallout4.esm|5240E"));
		CanUnarmedBlockKeyword = reinterpret_cast<BGSKeyword*>(GetFormFromIdentifier("Fallout4.esm|230326"));
		if (!DRCalculation || !UnarmedDamage || !CreatureKeyword || !UnarmedKeyword || !CanUnarmedBlockKeyword) {
			return false;
		}
		ArmorsDTAdder();
	}

	return true;
}

void Hooks() {
	/*ToCall();
	PipboyMenu::InitHooks();
	ContainerMenu::InitHooks();
	BarterMenu::InitHooks();*/
	//ExamineMenu::InitHooks();
}


void F4SEMessageHandler(F4SEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case F4SEMessagingInterface::kMessage_GameDataReady:
	{
		bool isReady = reinterpret_cast<bool>(msg->data);
		if (isReady)
		{

			//auto f4nv = (*g_dataHandler)->LookupLoadedModByName("F4NVMaster.esm");
			//if (!f4nv) {

			if (!GetForms()) {
				_WARNING("Error 003, mod doesn't work");
			}
			else {
				GameDataReady();
			}

			//}
			//else {
			//	_MESSAGE("F4NVMaster.esm detected, this mod is disabled");
			//}


		}
		break;
	}
	case F4SEMessagingInterface::kMessage_NewGame:
	{
		HideTargetHPDT();
		if (!oncePerSession) {
			if (!RegisterAfterLoadEvents()) {
				_WARNING("Events weren't registered correctly, mod won't work");
			}
			if (MinDTPenetrate) {
				MinDTPenetrateValue = MinDTPenetrate->value;
			}
			oncePerSession = true;
		}
		break;
	}
	}
}

extern "C"
{

	bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
	{
		if (f4se->runtimeVersion != RUNTIME_VERSION_1_10_163) {
			_WARNING("WARNING: Unsupported runtime version %08X. This DLL is built for v1.10.163 only.", f4se->runtimeVersion);
			MessageBox(NULL, (LPCSTR)("Unsupported runtime version (expected v1.10.163). \n" + mName + " will be disabled.").c_str(), (LPCSTR)mName.c_str(), MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		gLog.OpenRelative(CSIDL_MYDOCUMENTS, (const char*)("\\My Games\\Fallout4\\F4SE\\" + mName + ".log").c_str());
		logMessage("v1.0");
		logMessage("query");
		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = mName.c_str();
		info->version = 1;

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = f4se->GetPluginHandle();

		g_scaleform = (F4SEScaleformInterface *)f4se->QueryInterface(kInterface_Scaleform);
		if (!g_scaleform) {
			_FATALERROR("couldn't get scaleform interface");
			return false;
		}
		else {
			_MESSAGE("got it");
		}


		g_messaging = (F4SEMessagingInterface *)f4se->QueryInterface(kInterface_Messaging);
		if (!g_messaging)
		{
			_FATALERROR("couldn't get messaging interface");
			return false;
		}
	}

	bool F4SEPlugin_Load(const F4SEInterface *f4se)
	{
		if (g_scaleform)
		{
			g_scaleform->Register(mName.c_str(), RegisterScaleform);
			logMessage("Scaleform Register Succeeded");
		}
		if (g_messaging)
			g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);

		if (!g_localTrampoline.Create(1024 * 64, nullptr)) {
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}

		if (!g_branchTrampoline.Create(1024 * 64)) {
			_ERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
			return false;
		}
		if (!g_localTrampoline.Create(1024 * 64, nullptr))
		{
			_ERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
			return false;
		}
		InitDamageThresholdHooks();
		PipboyMenu::InitHooks();
		InitContainersHook();
		logMessage("load");
		return true;

	}
};
