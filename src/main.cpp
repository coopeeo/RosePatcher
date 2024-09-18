// Nova is real I think
// Note: used some of Inkay, the Pretendo Network plugin source code as a base
// and to make use of the notfications. Credit to Pretendo for developing Inkay
#include <coreinit/debug.h>
#include <coreinit/filesystem.h>
#include <coreinit/title.h>
#include <sysapp/launch.h>
#include <sysapp/switch.h>
#include <vpad/input.h>

#include <notifications/notifications.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config_api.h>

#include "Notification.h"
#include "utils/logger.h"

WUPS_PLUGIN_NAME("Vino Config Patcher");
WUPS_PLUGIN_DESCRIPTION("TVii config patcher");
WUPS_PLUGIN_VERSION("v1.1");
WUPS_PLUGIN_AUTHOR("Glitchii and Fangal");
WUPS_PLUGIN_LICENSE("GPLv2");

WUPS_USE_STORAGE("vcp");
WUPS_USE_WUT_DEVOPTAB();

#define VINO_CONFIG_PATH "/vol/content/vino_config.txt"
#define VINO_CONFIG_SD_PATH "/vol/external01/TVii/vino_config.txt"

#define CONNECT_TO_LATTE_CONFIG_ID "connect_to_latte"
#define CONNECT_TO_LATTE_DEFUALT_VALUE true

#define REPLACE_DLM_CONFIG_ID "replace_download_management"
#define REPLACE_DLM_DEFAULT_VALUE false

FSMountSource mSource;
char mPath[128] = "";

bool connectToLatte = true;
bool replaceDownloadManagement = false;
bool needRelaunch = false;

void connectToLatteChanged(ConfigItemBoolean *item, bool newValue) {
  if (newValue != connectToLatte) {
    WUPSStorageAPI::Store(CONNECT_TO_LATTE_CONFIG_ID, newValue);
  }

  connectToLatte = newValue;
}

void replaceDownloadManagementChanged(ConfigItemBoolean *item, bool newValue) {
  if (replaceDownloadManagement != newValue) {
    WUPSStorageAPI::Store(REPLACE_DLM_CONFIG_ID, newValue);
  }

  replaceDownloadManagement = newValue;
  needRelaunch = true;
}

WUPSConfigAPICallbackStatus
ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
  WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

  try {
    root.add(WUPSConfigItemBoolean::Create(
        CONNECT_TO_LATTE_CONFIG_ID, "Vino Config Patch Enabled",
        CONNECT_TO_LATTE_DEFUALT_VALUE, connectToLatte, connectToLatteChanged));
    root.add(WUPSConfigItemBoolean::Create(
        REPLACE_DLM_CONFIG_ID, "Replace Download Management",
        REPLACE_DLM_DEFAULT_VALUE, replaceDownloadManagement,
        replaceDownloadManagementChanged));
  } catch (std::exception &e) {
    DEBUG_FUNCTION_LINE("Creating config menu failed: %s", e.what());
    return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
  }

  return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback() {
  WUPSStorageAPI::SaveStorage();
  if (needRelaunch) {
    _SYSLaunchTitleWithStdArgsInNoSplash(OSGetTitleID(), nullptr);
    needRelaunch = false;
  }
}

INITIALIZE_PLUGIN() {
  WHBLogUdpInit();
  WHBLogCafeInit();

  WUPSConfigAPIOptionsV1 configOptions = {.name = "vcp"};
  if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback,
                         ConfigMenuClosedCallback) !=
      WUPSCONFIG_API_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("Failed to init config api");
  }

  WUPSStorageError storageRes;
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           CONNECT_TO_LATTE_CONFIG_ID, connectToLatte,
           CONNECT_TO_LATTE_DEFUALT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           REPLACE_DLM_CONFIG_ID, replaceDownloadManagement,
           REPLACE_DLM_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }

  if (NotificationModule_InitLibrary() != NOTIFICATION_MODULE_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("NotificationModule_InitLibrary failed :(");
  }

  if (connectToLatte) {
    ShowNotification("TVii patch enabled");
  } else {
    ShowNotification("TVii patch disabled");
  }
}

DEINITIALIZE_PLUGIN() {
  WHBLogUdpDeinit();
  NotificationModule_DeInitLibrary();
}

ON_APPLICATION_START() {
  WHBLogUdpInit();
  WHBLogCafeInit();

  DEBUG_FUNCTION_LINE("VCP: hihi");
}

DECL_FUNCTION(int32_t, _SYSSwitchTo, SysAppPFID pfid) {
  SysAppPFID uPfid = pfid;

  VPADStatus status;
  VPADReadError err;

  VPADRead(VPAD_CHAN_0, &status, 1, &err);

  if (pfid == SYSAPP_PFID_DOWNLOAD_MANAGEMENT) {
    if (replaceDownloadManagement) {
      if (!(status.hold & VPAD_BUTTON_ZL)) {
        uPfid = SYSAPP_PFID_TVII;
      }
    }
  }

  return real__SYSSwitchTo(uPfid);
}

DECL_FUNCTION(int32_t, _SYSSwitchToOverlayFromHBM, SysAppPFID pfid) {
  SysAppPFID uPfid = pfid;

  VPADStatus status;
  VPADReadError err;

  VPADRead(VPAD_CHAN_0, &status, 1, &err);

  if (pfid == SYSAPP_PFID_DOWNLOAD_MANAGEMENT) {
    if (replaceDownloadManagement) {
      if (!(status.hold & VPAD_BUTTON_ZL)) {
        uPfid = SYSAPP_PFID_TVII;
      }
    }
  }

  return real__SYSSwitchToOverlayFromHBM(uPfid);
}

DECL_FUNCTION(FSStatus, FSOpenFile, FSClient *client, FSCmdBlock *block,
              const char *path, const char *mode, FSFileHandle *handle,
              FSErrorFlag errorMask) {
  if (connectToLatte) {
    if (strcmp(VINO_CONFIG_PATH, path) == 0) {
      // Applets need to mount the SD card to access files on it :P
      FSGetMountSource(client, block, FS_MOUNT_SOURCE_SD, &mSource,
                       FS_ERROR_FLAG_ALL);
      FSMount(client, block, &mSource, mPath, sizeof(mPath), FS_ERROR_FLAG_ALL);
      FSStatus res = real_FSOpenFile(client, block, VINO_CONFIG_SD_PATH, mode,
                                     handle, errorMask);
      if (res != FS_STATUS_OK) {
        OSFatal("--------------- Error ---------------\n\n"
                "Error loading vino_config.txt\n\nPlease check if this file is "
                "in the correct directory:\n"
                "sd:/TVii/vino_config.txt");
      }

      return res;
    }
  }

  return real_FSOpenFile(client, block, path, mode, handle, errorMask);
}

WUPS_MUST_REPLACE(_SYSSwitchTo, WUPS_LOADER_LIBRARY_SYSAPP, _SYSSwitchTo);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(_SYSSwitchToOverlayFromHBM, 0x2E47373C,
                                       0x0E47373C,
                                       WUPS_FP_TARGET_PROCESS_HOME_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile, WUPS_LOADER_LIBRARY_COREINIT,
                              FSOpenFile, WUPS_FP_TARGET_PROCESS_TVII);
