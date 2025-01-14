/*
 *      Copyright (C) 2005-2014 Team XBMC
 *      http://xbmc.org
 *
 *      Copyright (C) 2014-2015 Aracnoz
 *      http://github.com/aracnoz/xbmc
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIDialogLAVAudio.h"
#include "Application.h"
#include "URL.h"
#include "dialogs/GUIDialogYesNo.h"
#include "filesystem/File.h"
#include "guilib/LocalizeStrings.h"
#include "profiles/ProfilesManager.h"
#include "settings/Settings.h"
#include "settings/lib/Setting.h"
#include "settings/lib/SettingsManager.h"
#include "utils/LangCodeExpander.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogSelect.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "input/Key.h"
#include "utils/XMLUtils.h"
#include "Filters/RendererSettings.h"
#include "PixelShaderList.h"
#include "cores/playercorefactory/PlayerCoreFactory.h"
#include "Filters/LAVAudioSettings.h"
#include "Filters/LAVVideoSettings.h"
#include "Filters/LAVSplitterSettings.h"
#include "utils/CharsetConverter.h"
#include "settings/MediaSettings.h"
#include "settings/Settings.h"
#include "settings/lib/Setting.h"
#include "settings/lib/SettingsManager.h"
#include "addons/Skin.h"
#include "GraphFilters.h"

#define LAVAUDIO_PROPERTYPAGE      "lavaudio.propertypage"
#define LAVAUDIO_TRAYICON          "lavaudio.trayicon"
#define LAVAUDIO_DRCENABLED        "lavaudio.drcenabled"
#define LAVAUDIO_DRCLEVEL          "lavaudio.drclevel"
#define LAVAUDIO_BITSTREAM_AC3     "lavaudio.bitstreamac3"
#define LAVAUDIO_BITSTREAM_EAC3    "lavaudio.bitstreameac3"
#define LAVAUDIO_BITSTREAM_TRUEHD  "lavaudio.bitstreamtruehd"
#define LAVAUDIO_BITSTREAM_DTS     "lavaudio.bitstreamdts"
#define LAVAUDIO_BITSTREAM_DTSHD   "lavaudio.bitstreamdtshd"
#define LAVAUDIO_DTSHDFRAMING      "lavaudio.dtshdframing"
#define LAVAUDIO_AUTOSYNCAV        "lavaudio.autosyncav"
#define LAVAUDIO_EXPANDMONO        "lavaudio.expandmono"
#define LAVAUDIO_EXPAND61          "lavaudio.expand61"
#define LAVAUDIO_51LEGACY          "lavaudio.51legacy"
#define LAVAUDIO_OUTSTANDARD       "lavaudio.outstandard"
#define LAVAUDIO_MIXINGENABLED     "lavaudio.mixingenabled"
#define LAVAUDIO_MIXINGLAYOUT      "lavaudio.mixinglayout"
#define LAVAUDIO_MIXING_DONTMIX    "lavaudio.mixingdontmix"
#define LAVAUDIO_MIXING_NORMALIZE  "lavaudio.mixingnormalize"
#define LAVAUDIO_MIXING_CLIP       "lavaudio.mixingclip"
#define LAVAUDIO_MIXINGMODE        "lavaudio.mixingmode"
#define LAVAUDIO_MIXINGCENTER      "lavaudio.mixingcenter"
#define LAVAUDIO_MIXINGSURROUND    "lavaudio.mixingsurround"
#define LAVAUDIO_MIXINGLFE         "lavaudio.mixinglfe"
#define LAVAUDIO_RESET             "lavaudio.reset"

using namespace std;

CGUIDialogLAVAudio::CGUIDialogLAVAudio()
  : CGUIDialogSettingsManualBase(WINDOW_DIALOG_LAVAUDIO, "VideoOSDSettings.xml")
{
  m_allowchange = true;
}


CGUIDialogLAVAudio::~CGUIDialogLAVAudio()
{ }

void CGUIDialogLAVAudio::OnInitWindow()
{
  CGUIDialogSettingsManualBase::OnInitWindow();

  HideUnused();
}

void CGUIDialogLAVAudio::OnDeinitWindow(int nextWindowID)
{
  CGUIDialogSettingsManualBase::OnDeinitWindow(nextWindowID);
}

void CGUIDialogLAVAudio::Save()
{
}

void CGUIDialogLAVAudio::SetupView()
{
  CGUIDialogSettingsManualBase::SetupView();

  SetHeading(55078);
}

void CGUIDialogLAVAudio::FlagsToBool(int flags)
{
  m_dontMix   = (flags == 1 || flags == 3 || flags == 5 || flags == 7);
  m_normalize = (flags == 2 || flags == 3 || flags == 6 || flags == 7);
  m_clip      = (flags == 4 || flags == 5 || flags == 6 || flags == 7);
}

int CGUIDialogLAVAudio::BoolToFlags()
{
  int a, b, c;
  m_dontMix ? a = 1 : a = 0;
  m_normalize ? b = 2 : b = 0;
  m_clip ? c = 4 : c = 0;
  return a + b + c;
}


void CGUIDialogLAVAudio::InitializeSettings()
{
  CGUIDialogSettingsManualBase::InitializeSettings();

  bool usePopup = g_SkinInfo->HasSkinFile("DialogSlider.xml");

  CSettingCategory *category = AddCategory("dsplayerlavaudio", -1);
  if (category == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  // get all necessary setting groups
  CSettingGroup *groupProperty = AddGroup(category);
  if (groupProperty == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  // get all necessary setting groups
  CSettingGroup *group = AddGroup(category);
  if (group == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  CSettingGroup *groupBitstream = AddGroup(category);
  if (groupBitstream == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  // get all necessary setting groups
  CSettingGroup *groupOptions = AddGroup(category);
  if (groupOptions == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  // get all necessary setting groups
  CSettingGroup *groupDRC = AddGroup(category);
  if (groupDRC == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  // get all necessary setting groups
  CSettingGroup *groupMixer = AddGroup(category);
  if (groupMixer == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  CSettingGroup *groupSettings = AddGroup(category);
  if (groupSettings == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  CSettingGroup *groupEncoding = AddGroup(category);
  if (groupEncoding == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }
  CSettingGroup *groupReset = AddGroup(category);
  if (groupReset == NULL)
  {
    CLog::Log(LOGERROR, "CGUIDialogLAVAudio: unable to setup settings");
    return;
  }

  // Get settings from the current running filter
  IBaseFilter *pBF;
  CGraphFilters::Get()->GetInternalFilter(LAVAUDIO, &pBF);
  CGraphFilters::Get()->GetLavSettings(LAVAUDIO, pBF);

  StaticIntegerSettingOptions entries;
  CLavSettings &lavSettings = CMediaSettings::GetInstance().GetCurrentLavSettings();

  // BUTTON
  AddButton(groupProperty, LAVAUDIO_PROPERTYPAGE, 80013, 0);

  // TRAYICON
  AddToggle(group, LAVAUDIO_TRAYICON, 80001, 0, lavSettings.audio_bTrayIcon);

  // BITSTREAM
  AddToggle(groupBitstream, LAVAUDIO_BITSTREAM_AC3, 81003, 0, lavSettings.audio_bBitstream[0]);
  AddToggle(groupBitstream, LAVAUDIO_BITSTREAM_EAC3, 81004, 0, lavSettings.audio_bBitstream[1]);
  AddToggle(groupBitstream, LAVAUDIO_BITSTREAM_TRUEHD, 81005, 0, lavSettings.audio_bBitstream[2]);
  AddToggle(groupBitstream, LAVAUDIO_BITSTREAM_DTS, 81006, 0, lavSettings.audio_bBitstream[3]);
  AddToggle(groupBitstream, LAVAUDIO_BITSTREAM_DTSHD, 81007, 0, lavSettings.audio_bBitstream[4]);
  AddToggle(groupBitstream, LAVAUDIO_DTSHDFRAMING, 81008, 0, lavSettings.audio_bDTSHDFraming);

  // OPTIONS
  AddToggle(groupOptions, LAVAUDIO_AUTOSYNCAV, 81009, 0, lavSettings.audio_bAutoAVSync);
  AddToggle(groupOptions, LAVAUDIO_OUTSTANDARD, 81010, 0, lavSettings.audio_bOutputStandardLayout);
  AddToggle(groupOptions, LAVAUDIO_EXPANDMONO, 81011, 0, lavSettings.audio_bExpandMono);
  AddToggle(groupOptions, LAVAUDIO_EXPAND61, 81012, 0, lavSettings.audio_bExpand61);
  AddToggle(groupOptions, LAVAUDIO_51LEGACY, 81031, 0, lavSettings.audio_b51Legacy);

  //DRC
  AddToggle(groupDRC, LAVAUDIO_DRCENABLED, 81001, 0, lavSettings.audio_bDRCEnabled);
  AddSlider(groupDRC, LAVAUDIO_DRCLEVEL, 81002, 0, lavSettings.audio_iDRCLevel, "%i%%", 0, 1, 100);

  // MIXER
  AddToggle(groupMixer, LAVAUDIO_MIXINGENABLED, 81013, 0, lavSettings.audio_bMixingEnabled);
  entries.clear();
  entries.push_back(make_pair(81015, 4));
  entries.push_back(make_pair(81016, 3));
  entries.push_back(make_pair(81017, 1539));
  entries.push_back(make_pair(81018, 63));
  entries.push_back(make_pair(81019, 1807));
  entries.push_back(make_pair(81020, 1599));
  AddList(groupMixer, LAVAUDIO_MIXINGLAYOUT, 81014, 0, lavSettings.audio_dwMixingLayout, entries, 81014);
  AddSlider(groupMixer, LAVAUDIO_MIXINGCENTER, 81021, 0, DWToFloat(lavSettings.audio_dwMixingCenterLevel), "%1.2f", 0.0f, 0.01f, 1.00f);
  AddSlider(groupMixer, LAVAUDIO_MIXINGSURROUND, 81022, 0, DWToFloat(lavSettings.audio_dwMixingSurroundLevel), "%1.2f", 0.0f, 0.01f, 1.00f);
  AddSlider(groupMixer, LAVAUDIO_MIXINGLFE, 81023, 0, DWToFloat(lavSettings.audio_dwMixingLFELevel), "%1.2f", 0.0f, 0.01f, 1.00f);

  // SETTINGS
  FlagsToBool(lavSettings.audio_dwMixingFlags);
  AddToggle(groupSettings, LAVAUDIO_MIXING_DONTMIX, 81024, 0, m_dontMix);
  AddToggle(groupSettings, LAVAUDIO_MIXING_NORMALIZE, 81025, 0, m_normalize);
  AddToggle(groupSettings, LAVAUDIO_MIXING_CLIP, 81026, 0, m_clip);

  // ENCODINGS
  entries.clear();
  entries.push_back(make_pair(81028, 0));
  entries.push_back(make_pair(81029, 1));
  entries.push_back(make_pair(81030, 2));
  AddList(groupEncoding, LAVAUDIO_MIXINGMODE, 81027, 0, lavSettings.audio_dwMixingMode, entries, 81027);

  // BUTTON RESET
  if (!g_application.m_pPlayer->IsPlayingVideo())
    AddButton(groupReset, LAVAUDIO_RESET, 10041, 0);
}

void CGUIDialogLAVAudio::OnSettingChanged(const CSetting *setting)
{
  if (setting == NULL)
    return;

  CLavSettings &lavSettings = CMediaSettings::GetInstance().GetCurrentLavSettings();

  CGUIDialogSettingsManualBase::OnSettingChanged(setting);
  const std::string &settingId = setting->GetId();

  if (settingId == LAVAUDIO_TRAYICON)
    lavSettings.audio_bTrayIcon = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_DRCENABLED)
    lavSettings.audio_bDRCEnabled = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_DRCLEVEL)
    lavSettings.audio_iDRCLevel = static_cast<int>(static_cast<const CSettingInt*>(setting)->GetValue());
  if (settingId == LAVAUDIO_BITSTREAM_AC3)
    lavSettings.audio_bBitstream[0] = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_BITSTREAM_EAC3)
    lavSettings.audio_bBitstream[1] = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_BITSTREAM_TRUEHD)
    lavSettings.audio_bBitstream[2] = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_BITSTREAM_DTS)
    lavSettings.audio_bBitstream[3] = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_BITSTREAM_DTSHD)
    lavSettings.audio_bBitstream[4] = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_DTSHDFRAMING)
    lavSettings.audio_bDTSHDFraming = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_AUTOSYNCAV)
    lavSettings.audio_bAutoAVSync = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_EXPANDMONO)
    lavSettings.audio_bExpandMono = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_EXPAND61)
    lavSettings.audio_bExpand61 = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_51LEGACY)
    lavSettings.audio_b51Legacy = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_OUTSTANDARD)
    lavSettings.audio_bOutputStandardLayout = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_MIXINGENABLED)
    lavSettings.audio_bMixingEnabled = static_cast<BOOL>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_MIXINGLAYOUT)
    lavSettings.audio_dwMixingLayout = static_cast<int>(static_cast<const CSettingInt*>(setting)->GetValue());
  if (settingId == LAVAUDIO_MIXING_DONTMIX)
    m_dontMix = static_cast<bool>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_MIXING_NORMALIZE)
    m_normalize = static_cast<bool>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_MIXING_CLIP)
    m_clip = static_cast<bool>(static_cast<const CSettingBool*>(setting)->GetValue());
  if (settingId == LAVAUDIO_MIXING_DONTMIX || settingId == LAVAUDIO_MIXING_NORMALIZE || settingId == LAVAUDIO_MIXING_CLIP)
    lavSettings.audio_dwMixingFlags = BoolToFlags();
  if (settingId == LAVAUDIO_MIXINGMODE)
    lavSettings.audio_dwMixingMode = static_cast<int>(static_cast<const CSettingInt*>(setting)->GetValue());
  if (settingId == LAVAUDIO_MIXINGCENTER)
    lavSettings.audio_dwMixingCenterLevel = FloatToDw(static_cast<float>(static_cast<const CSettingNumber*>(setting)->GetValue()));
  if (settingId == LAVAUDIO_MIXINGSURROUND)
    lavSettings.audio_dwMixingSurroundLevel = FloatToDw(static_cast<float>(static_cast<const CSettingNumber*>(setting)->GetValue()));
  if (settingId == LAVAUDIO_MIXINGLFE)
    lavSettings.audio_dwMixingLFELevel = FloatToDw(static_cast<float>(static_cast<const CSettingNumber*>(setting)->GetValue()));

  HideUnused();

  // Get current running filter
  IBaseFilter *pBF;
  CGraphFilters::Get()->GetInternalFilter(LAVAUDIO, &pBF);

  // Set settings changes into the running filter
  CGraphFilters::Get()->SetLavSettings(LAVAUDIO, pBF);

  // Save new settings into DSPlayer DB
  CGraphFilters::Get()->SaveLavSettings(LAVAUDIO);
}

void CGUIDialogLAVAudio::OnSettingAction(const CSetting *setting)
{
  if (setting == NULL)
    return;

  CGUIDialogSettingsManualBase::OnSettingAction(setting);
  const std::string &settingId = setting->GetId();

  if (settingId == LAVAUDIO_PROPERTYPAGE)
  {
    CGraphFilters::Get()->ShowInternalPPage(LAVAUDIO, true);
    this->Close();
  }

  if (settingId == LAVAUDIO_RESET)
  {
    if (!CGUIDialogYesNo::ShowAndGetInput(10041, 10042, 0, 0))
      return;

    CGraphFilters::Get()->EraseLavSetting(LAVAUDIO);
    this->Close();
  }
}

void CGUIDialogLAVAudio::HideUnused()
{
  if (!m_allowchange)
    return;

  m_allowchange = false;

  bool bValue;

  CSetting *setting;

  // HIDE / SHOW

  // DRC
  setting = m_settingsManager->GetSetting(LAVAUDIO_DRCENABLED);
  bValue = static_cast<const CSettingBool*>(setting)->GetValue();
  SetVisible(LAVAUDIO_DRCLEVEL, bValue);

  // MIXING
  setting = m_settingsManager->GetSetting(LAVAUDIO_MIXINGENABLED);
  bValue = static_cast<const CSettingBool*>(setting)->GetValue();
  SetVisible(LAVAUDIO_MIXINGLAYOUT, bValue);
  SetVisible(LAVAUDIO_MIXINGCENTER, bValue);
  SetVisible(LAVAUDIO_MIXINGSURROUND, bValue);
  SetVisible(LAVAUDIO_MIXINGLFE, bValue);

  m_allowchange = true;
}

void CGUIDialogLAVAudio::SetVisible(CStdString id, bool visible)
{
  CSetting *setting = m_settingsManager->GetSetting(id);
  if (setting->IsEnabled() && visible)
    return;

  setting->SetVisible(true);
  setting->SetEnabled(visible);
}


