#pragma once
#include <wups.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItem.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemStub.h>

#define CONNECT_TO_ROSE_CONFIG_ID "connect_to_rose"
#define CONNECT_TO_ROSE_DEFUALT_VALUE true

#define TVII_ICON_HBM_PATCH_COFNIG_ID "tvii_icon_hbm_patch"
#define TVII_ICON_HBM_PATCH_DEFAULT_VALUE true

#define TVII_ICON_WUM_PATCH_COFNIG_ID "tvii_icon_WUM_patch"
#define TVII_ICON_WUM_PATCH_DEFAULT_VALUE true



namespace config {

    extern bool connectToRose;
    extern bool tviiIconHBM;
    extern bool tviiIconWUM;
    extern bool needRelaunch;

    void connectToRoseChanged(ConfigItemBoolean *item, bool newValue);
    void tviiIconHBMChanged(ConfigItemBoolean *item, bool newValue);
    void tviiIconWUMChanged(ConfigItemBoolean *item, bool newValue);

    WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle);
    
    void ConfigMenuClosedCallback();
    void InitializeConfig();
} // namespace config
