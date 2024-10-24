#pragma once

#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <coreinit/filesystem.h>
#include <coreinit/memorymap.h>
#include <coreinit/title.h>
#include <sysapp/launch.h>
#include <sysapp/switch.h>
#include <vpad/input.h>

#include <function_patcher/function_patching.h>

#include <notifications/notifications.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config_api.h>

#include "utils/Notification.hpp"
#include "utils/logger.h"

#define CONNECT_TO_ROSE_CONFIG_ID "connect_to_rose"
#define CONNECT_TO_ROSE_DEFUALT_VALUE true

#define TVII_ICON_HBM_PATCH_COFNIG_ID "tvii_icon_hbm_patch"
#define TVII_ICON_HBM_PATCH_DEFAULT_VALUE true

#define TVII_ICON_WUM_PATCH_COFNIG_ID "tvii_icon_WUM_patch"
#define TVII_ICON_WUM_PATCH_DEFAULT_VALUE true

// Settings
extern bool connectToRose;
extern bool tviiIconHBM;
extern bool tviiIconWUM;
extern bool needRelaunch;

// Connect to Rose setting event
void connectToRoseChanged(ConfigItemBoolean *item, bool newValue);

// TVii Icon HBM patch setting event
void tviiIconHBMChanged(ConfigItemBoolean *item, bool newValue);

// TVii Icon WUM patch setting event
void tviiIconWUMChanged(ConfigItemBoolean *item, bool newValue);

// Open event for the Aroma config menu
WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle);

// Close event for the Aroma config menu
void ConfigMenuClosedCallback();

void InitializeConfig();

bool isVinoClientID(const char *client_id);

bool isVinoTitleID(uint32_t title_id);


// Wii U Menu and HBM Patch functions
namespace patches {
    void perform_men_patches(bool enable);
    void perform_hbm_patches(bool enable);
} // namespace patches