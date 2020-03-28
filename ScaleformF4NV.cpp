#include "f4se/ScaleformValue.h"
#include "f4se/ScaleformMovie.h"
#include "f4se/ScaleformCallbacks.h"
#include "f4se/PapyrusScaleformAdapter.h"
#include <fstream>  
#include "f4se/PapyrusEvents.h"
#include "f4se/PapyrusUtilities.h"
#include "main.h"
#include "f4se/GameData.h"
#include "f4se/GameRTTI.h"
#include "f4se/GameMenus.h"
#include "f4se/GameInput.h"
#include "f4se/InputMap.h"
#include "ScaleformF4NV.h"
#include "f4se\GameReferences.h"
#include "Globals.h"

ParamType Param1Type;
OperationType Operation1Type;
ParamType Param2Type;
OperationType Operation2Type;
ParamType Param3Type;
OperationType Operation3Type;
ParamType Param4Type;
float Param1Val;
float Param2Val;
float Param3Val;
float Param4Val;

std::string OverrideEspName;
bool isOverrideEnabled;
//bool isOptionalPluginEnabled

std::string oDT;
std::string oAP;
std::string oDType;
std::string oLA;
std::string oIgnKwd;


bool to_bool(std::string const& s) {
	return s != "0";
}

std::string DTFGetConfigOptionString(std::string name, const char * section, const char * key)
{
	std::string	result;

	const std::string & configPath = "./Data/DTF/" + name;
	if (!configPath.empty())
	{
		char	resultBuf[256];
		resultBuf[0] = 0;

		UInt32	resultLen = GetPrivateProfileString(section, key, NULL, resultBuf, sizeof(resultBuf), configPath.c_str());

		result = resultBuf;
	}

	return result;
}

UInt32 DTFGetConfigOptionInt(std::string name, const char * section, const char * key)
{
	std::string path;
	path = "./Data/DTF/" + name;
	UInt32 result;
	result = GetPrivateProfileInt(section, key, 0, path.c_str());
	return result;
}

void Getinis(std::vector<WIN32_FIND_DATA>* arr) {
	char* modSettingsDirectory = "Data\\DTF\\*.ini";

	HANDLE hFind;
	WIN32_FIND_DATA data;
	hFind = FindFirstFile(modSettingsDirectory, &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			arr->push_back(data);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
}


bool DTFGetConfigFormulas(const char * section, const char * key)
{
	bool Found = false;
	//In progress
	std::string	result;
	const std::string & configPath = "./Data/DTF/Formula/DamageThresholdFrameworkFormulas.ini";

	std::ifstream in(configPath);
	std::string str = "";
	while (std::getline(in, str)) {
		if (str.find("//") == std::string::npos) {
			if (str.find("Param1") != std::string::npos) {
				Found = true;
				remove_if(str.begin(), str.end(), isspace);
				unsigned first = str.find("[") + 1;
				unsigned last = str.find("]");
				std::string strNew = str.substr(first, last - first);
				if (strNew != "") {
					if (strNew.compare("BaseArmorAR") == 0) {
						Param1Type = ParamType::BaseArmorDR;
					}
					else if (strNew.compare("CapsValue") == 0) {
						Param1Type = ParamType::CapsValue;
					}
					else if (strNew.compare("Weight") == 0) {
						Param1Type = ParamType::Weight;
					}
					else {
						Param1Type = ParamType::CustomValue;
						Param1Val = std::stof(strNew);
						if (Param1Val > 1000 || Param1Val < 0) {
							Param1Val = 0;
							Param1Type = ParamType::Undefined1;
						}

					}
				}
			}
			else if (str.find("operation1") != std::string::npos) {
				Found = true;
				remove_if(str.begin(), str.end(), isspace);
				unsigned first = str.find("[") + 1;
				unsigned last = str.find("]");
				std::string strNew = str.substr(first, last - first);
				if (strNew != "") {
					if (strNew.compare("+") == 0) {
						Operation1Type = OperationType::Add;
					}
					else if (strNew.compare("-") == 0) {
						Operation1Type = OperationType::Subtract;
					}
					else if (strNew.compare("*") == 0) {
						Operation1Type = OperationType::Multiply;
					}
					else {
						Operation1Type = OperationType::Undefined2;

					}
				}
			}
			else if (str.find("Param2") != std::string::npos) {
				Found = true;
				remove_if(str.begin(), str.end(), isspace);
				unsigned first = str.find("[") + 1;
				unsigned last = str.find("]");
				std::string strNew = str.substr(first, last - first);
				if (strNew != "") {
					if (strNew.compare("BaseArmorAR") == 0) {
						Param2Type = ParamType::BaseArmorDR;
					}
					else if (strNew.compare("CapsValue") == 0) {
						Param2Type = ParamType::CapsValue;
					}
					else if (strNew.compare("Weight") == 0) {
						Param2Type = ParamType::Weight;
					}
					else {
						Param2Type = ParamType::CustomValue;
						Param2Val = std::stof(strNew);
						if (Param2Val > 1000 || Param2Val < 0) {
							Param2Val = 0;
							Param2Type = ParamType::Undefined1;
						}

					}
				}
			}
			else if (str.find("operation2") != std::string::npos) {
				Found = true;
				remove_if(str.begin(), str.end(), isspace);
				unsigned first = str.find("[") + 1;
				unsigned last = str.find("]");
				std::string strNew = str.substr(first, last - first);
				if (strNew != "") {
					if (strNew.compare("+") == 0) {
						Operation2Type = OperationType::Add;
					}
					else if (strNew.compare("-") == 0) {
						Operation2Type = OperationType::Subtract;
					}
					else if (strNew.compare("*") == 0) {
						Operation2Type = OperationType::Multiply;
					}
					else {
						Operation2Type = OperationType::Undefined2;

					}
				}
			}
			else if (str.find("Param3") != std::string::npos) {
				Found = true;
				remove_if(str.begin(), str.end(), isspace);
				unsigned first = str.find("[") + 1;
				unsigned last = str.find("]");
				std::string strNew = str.substr(first, last - first);
				if (strNew != "") {
					if (strNew.compare("BaseArmorAR") == 0) {
						Param3Type = ParamType::BaseArmorDR;
					}
					else if (strNew.compare("CapsValue") == 0) {
						Param3Type = ParamType::CapsValue;
					}
					else if (strNew.compare("Weight") == 0) {
						Param3Type = ParamType::Weight;
					}
					else {
						Param3Type = ParamType::CustomValue;
						Param3Val = std::stof(strNew);
						if (Param3Val > 1000 || Param3Val < 0) {
							Param3Val = 0;
							Param3Type = ParamType::Undefined1;
						}

					}
				}
			}
			else if (str.find("operation3") != std::string::npos) {
				Found = true;
				remove_if(str.begin(), str.end(), isspace);
				unsigned first = str.find("[") + 1;
				unsigned last = str.find("]");
				std::string strNew = str.substr(first, last - first);
				if (strNew != "") {
					if (strNew.compare("+") == 0) {
						Operation3Type = OperationType::Add;
					}
					else if (strNew.compare("-") == 0) {
						Operation3Type = OperationType::Subtract;
					}
					else if (strNew.compare("*") == 0) {
						Operation3Type = OperationType::Multiply;
					}
					else {
						Operation3Type = OperationType::Undefined2;

					}
				}
			}
			else if (str.find("Param4") != std::string::npos) {
				Found = true;
				remove_if(str.begin(), str.end(), isspace);
				unsigned first = str.find("[") + 1;
				unsigned last = str.find("]");
				std::string strNew = str.substr(first, last - first);
				if (strNew != "") {
					if (strNew.compare("BaseArmorAR") == 0) {
						Param4Type = ParamType::BaseArmorDR;
					}
					else if (strNew.compare("CapsValue") == 0) {
						Param4Type = ParamType::CapsValue;
					}
					else if (strNew.compare("Weight") == 0) {
						Param4Type = ParamType::Weight;
					}
					else {
						Param4Type = ParamType::CustomValue;
						Param4Val = std::stof(strNew);
						if (Param4Val > 1000 || Param4Val < 0) {
							Param4Val = 0;
							Param4Type = ParamType::Undefined1;
						}

					}
				}
			}
		}
	}

	return Found;
}

bool LoadSettings() {
	bool result = false;
	std::vector<WIN32_FIND_DATA> modSettingFiles;
	Getinis(&modSettingFiles);
	for (size_t i = 0; i < modSettingFiles.size(); i++)
	{
		isOverrideEnabled = to_bool(DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Main", "IsOverrideEnabled"));
		if (isOverrideEnabled) {
			std::string tempName = DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Main", "OverrideEspName");
			auto mod = (*g_dataHandler)->LookupLoadedModByName(tempName.c_str());
			if (mod) {
				OverrideEspName = DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Main", "OverrideEspName");

				oDT = DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Forms", "DT");
				oAP = DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Forms", "AP");
				oDType = DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Forms", "DtDamType");
				oLA = DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Forms", "HasLivingAnatomyPerk");
				oIgnKwd = DTFGetConfigOptionString(modSettingFiles[i].cFileName, "Forms", "Ignorekwd");

				result = true;
			}
			else {
				std::string m = tempName + " Is Not Enabled";
				_MESSAGE(m.c_str());
			}
		}
	}
	return result;
}


GFxMovieRoot* HUDMovieRoot = nullptr;


void SetState(bool value, bool value2)
{
	IMenu * pHUD = nullptr;
	static BSFixedString menuName("HUDMenu");
	if ((*g_ui) != nullptr && (pHUD = (*g_ui)->GetMenu(menuName), pHUD))
	{
		GFxMovieRoot * movieRoot = pHUD->movie->movieRoot;
		if (movieRoot != nullptr) {
			GFxValue arrArgs[1];
			GFxValue arrArgs2[1];
			GFxValue val1;
			GFxValue val2;

			arrArgs[0].SetBool(value);
			arrArgs2[0].SetBool(value2);

			if (movieRoot->GetVariable(&val1, "root.shield_loader.content.EShieldF_mc.visible")) {
				if (val1.GetBool() != value) {
					movieRoot->Invoke("root.shield_loader.content.HandleFShieldDisplayEnemy", nullptr, arrArgs, 1);
				}
				else if (val1.GetBool() && value) {
					movieRoot->Invoke("root.shield_loader.content.RestartTimerF", nullptr, nullptr, 0);
				}
			}
			if (movieRoot->GetVariable(&val2, "root.shield_loader.content.EShieldB_mc.visible") && val2.GetBool() != value2) {
				if (val2.GetBool() != value2) {
					movieRoot->Invoke("root.shield_loader.content.HandleBShieldDisplayEnemy", nullptr, arrArgs2, 1);
				}
				else if (val2.GetBool() && value) {
					movieRoot->Invoke("root.shield_loader.content.RestartTimerB", nullptr, nullptr, 0);
				}
			}

			//movieRoot->Invoke("root.shield_loader.content.HandleEShieldDisplayEnemy", nullptr, arrArgs, 2);
		}
	}

}

void SetStatePlayer(bool value, bool value2)
{
	IMenu * pHUD = nullptr;
	static BSFixedString menuName("HUDMenu");
	if ((*g_ui) != nullptr && (pHUD = (*g_ui)->GetMenu(menuName), pHUD))
	{
		GFxMovieRoot * movieRoot = pHUD->movie->movieRoot;
		if (movieRoot != nullptr) {
			GFxValue arrArgs[1];
			GFxValue arrArgs2[1];

			GFxValue val1;
			GFxValue val2;

			arrArgs[0].SetBool(value);
			arrArgs2[0].SetBool(value2);

			if (movieRoot->GetVariable(&val1, "root.shield_loader.content.PShieldF_mc.visible")) {
				if (val1.GetBool() != value) {
					movieRoot->Invoke("root.shield_loader.content.HandleFShieldDisplayPlayer", nullptr, arrArgs, 1);
				}
				else if (val1.GetBool() && value) {
					movieRoot->Invoke("root.shield_loader.content.RestartTimerPF", nullptr, nullptr, 0);
				}
				
			}
			if (movieRoot->GetVariable(&val2, "root.shield_loader.content.PShieldB_mc.visible") && val2.GetBool() != value2) {

				if (val2.GetBool() != value) {
					movieRoot->Invoke("root.shield_loader.content.HandleBShieldDisplayPlayer", nullptr, arrArgs2, 1);
				} 
				else if (val2.GetBool() && value) {
					movieRoot->Invoke("root.shield_loader.content.RestartTimerPB", nullptr, nullptr, 0);
				}

				
			}

			//movieRoot->Invoke("root.shield_loader.content.HandleEShieldDisplayPlayer", nullptr, arrArgs, 2);

		}
	}
}


void UpdateTargetHPDT(UInt32 hp, UInt32 dt) {
	IMenu * pHUD = nullptr;
	static BSFixedString menuName("HUDMenu");
	if ((*g_ui) != nullptr && (pHUD = (*g_ui)->GetMenu(menuName), pHUD))
	{
		GFxMovieRoot * movieRoot = pHUD->movie->movieRoot;
		if (movieRoot != nullptr) {
			GFxValue arrArgs[2];
			arrArgs[0].SetUInt(hp);
			arrArgs[1].SetUInt(dt);
			UInt32 a = arrArgs[0].GetUInt();
			UInt32 b = arrArgs[1].GetUInt();
			movieRoot->Invoke("root.livinganatomy_loader.content.UpdateTarget", nullptr, arrArgs, 2);

		}
	}
}

void ShowTargetHPDT() {
	IMenu * pHUD = nullptr;
	static BSFixedString menuName("HUDMenu");
	if ((*g_ui) != nullptr && (pHUD = (*g_ui)->GetMenu(menuName), pHUD))
	{
		GFxMovieRoot * movieRoot = pHUD->movie->movieRoot;
		if (movieRoot != nullptr) {
			movieRoot->SetVariable("root.livinganatomy_loader.content.visible", &GFxValue(true));

		}
	}
}

void HideTargetHPDT() {
	IMenu * pHUD = nullptr;
	static BSFixedString menuName("HUDMenu");
	if ((*g_ui) != nullptr && (pHUD = (*g_ui)->GetMenu(menuName), pHUD))
	{
		GFxMovieRoot * movieRoot = pHUD->movie->movieRoot;
		if (movieRoot != nullptr) {
			movieRoot->SetVariable("root.livinganatomy_loader.content.visible", &GFxValue(false));

		}
	}
}

class Scaleform_getDT : public GFxFunctionHandler
{
public:
	virtual void    Invoke(Args * args) {
		// here code to get u DT value
		auto val = (*g_player)->actorValueOwner.GetValue(DT);
		args->result->SetInt(val);
	}
};



bool RegisterScaleform(GFxMovieView * view, GFxValue * f4se_root)
{

	//RegisterFunction<DTF_OnModSettingChanged>(f4se_root, view->movieRoot, "onModSettingChanged");

	GFxMovieRoot* movieRoot = view->movieRoot;

	GFxValue currentSWFPath;
	const char* currentSWFPathString = nullptr;

	if (movieRoot->GetVariable(&currentSWFPath, "root.loaderInfo.url")) {
		currentSWFPathString = currentSWFPath.GetString();
	}
	else {
		_MESSAGE("WARNING: Scaleform registration failed.");
	}

	// Look for the menu that we want to inject into.
	if (strcmp(currentSWFPathString, "Interface/HUDMenu.swf") == 0) {
		GFxValue root; movieRoot->GetVariable(&root, "root");
		HUDMovieRoot = movieRoot;
		GFxValue la; movieRoot->CreateObject(&la);
		root.SetMember("livinganatomy", &la);
		GFxValue urlRequestLA, loaderLA;
		movieRoot->CreateObject(&loaderLA, "flash.display.Loader");
		movieRoot->CreateObject(&urlRequestLA, "flash.net.URLRequest", &GFxValue("DamageThresholdFramework/LivingAnatomy.swf"), 1);
		root.SetMember("livinganatomy_loader", &loaderLA);
		bool injectionSuccessLA = movieRoot->Invoke("root.livinganatomy_loader.load", nullptr, &urlRequestLA, 1);
		movieRoot->Invoke("root.TopCenterGroup_mc.EnemyHealthMeter_mc.Bracket_mc.addChild", nullptr, &loaderLA, 1);


		GFxValue  urlRequest, loader;
		movieRoot->CreateObject(&loader, "flash.display.Loader");
		movieRoot->CreateObject(&urlRequest, "flash.net.URLRequest", &GFxValue("DamageThresholdFramework/F4NVShield.swf"), 1);

		root.SetMember("shield_loader", &loader);


		bool injectionSuccess = movieRoot->Invoke("root.shield_loader.load", nullptr, &urlRequest, 1);
		movieRoot->Invoke("root.addChild", nullptr, &loader, 1);


	}
	GFxValue currentSWFPathPip;
	std::string currentSWFPathStringPip = "";
	if (movieRoot->GetVariable(&currentSWFPathPip, "root.loaderInfo.url")) {
		currentSWFPathStringPip = currentSWFPathPip.GetString();
		//_MESSAGE("hooking %s", currentSWFPathString.c_str());
		if (currentSWFPathStringPip.find("PipboyMenu.swf") != std::string::npos)
		{
			if (!movieRoot)
				return false;
			GFxValue loaderPip, urlRequestPip, rootPip;
			movieRoot->GetVariable(&rootPip, "root");
			movieRoot->CreateObject(&loaderPip, "flash.display.Loader");
			movieRoot->CreateObject(&urlRequestPip, "flash.net.URLRequest", &GFxValue("DamageThresholdFramework/dt_pipboy_bottom_bar_widget.swf"), 1);
			rootPip.SetMember("dt_pipboy_bottom_bar_widget_loader", &loaderPip);
			GFxValue codeObj;
			movieRoot->GetVariable(&codeObj, "root.Menu_mc.BGSCodeObj");
			if (!codeObj.IsUndefined()) {
				RegisterFunction<Scaleform_getDT>(&codeObj, movieRoot, "getDT");
			}
			bool pip = movieRoot->Invoke("root.dt_pipboy_bottom_bar_widget_loader.load", nullptr, &urlRequestPip, 1);
			bool pip2 = movieRoot->Invoke("root.Menu_mc.BottomBar_mc.Info_mc.addChild", nullptr, &loaderPip, 1);
			if (!pip || !pip2) {
				_MESSAGE("WARNING: injection failed.");
			}
		}

	}


	return true;
}