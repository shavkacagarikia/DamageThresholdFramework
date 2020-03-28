#pragma once
class GFxMovieRoot;
class GFxMovieView;
class GFxValue;

struct KeybindInfo;

void SetState(bool value, bool value2);

void SetStatePlayer(bool value, bool value2);

void SetStateBoth(bool value, bool value2, bool value3, bool value4);


void UpdateTargetHPDT(UInt32 hp, UInt32 dt);

bool RegisterScaleform(GFxMovieView* view, GFxValue* f4se_root);

void HideTargetHPDT();
void ShowTargetHPDT();

bool LoadSettings();

bool DTFGetConfigFormulas(const char * section, const char * key);