#include "main.hpp"

// Connect to Rose setting event
void connectToRoseChanged(ConfigItemBoolean *item, bool newValue) {
  if (newValue != connectToRose) {
    WUPSStorageAPI::Store(CONNECT_TO_ROSE_CONFIG_ID, newValue);
  }

  connectToRose = newValue;
}

// Replace Download Management setting event
void replaceDownloadManagementChanged(ConfigItemBoolean *item, bool newValue) {
  if (replaceDownloadManagement != newValue) {
    WUPSStorageAPI::Store(REPLACE_DLM_CONFIG_ID, newValue);
  }

  replaceDownloadManagement = newValue;
  needRelaunch = true;
}

void tviiIconHBMChanged(ConfigItemBoolean *item, bool newValue) {
  if (tviiIconHBM != newValue) {
    WUPSStorageAPI::Store(TVII_ICON_HBM_PATCH_COFNIG_ID, newValue);
  }

  tviiIconHBM = newValue;
}

void tviiIconWUMChanged(ConfigItemBoolean *item, bool newValue) {
  if (tviiIconWUM != newValue) {
    WUPSStorageAPI::Store(TVII_ICON_WUM_PATCH_COFNIG_ID, newValue);
  }

  tviiIconWUM = newValue;
  auto title = OSGetTitleID();
  if (title == 0x5001010040000 || title == 0x5001010040100 || title == 0x5001010040200) {
    needRelaunch = true;
  }
}

// Open event for the Aroma config menu
WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
  WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

  try {
    // Add setting items
    root.add(WUPSConfigItemBoolean::Create(
        CONNECT_TO_ROSE_CONFIG_ID, "Connect to Rosé",
        CONNECT_TO_ROSE_DEFUALT_VALUE, connectToRose, connectToRoseChanged));
    root.add(WUPSConfigItemBoolean::Create(
        REPLACE_DLM_CONFIG_ID, "Replace Download Management",
        REPLACE_DLM_DEFAULT_VALUE, replaceDownloadManagement,
        replaceDownloadManagementChanged));
    root.add(WUPSConfigItemBoolean::Create(
        TVII_ICON_HBM_PATCH_COFNIG_ID, "Add TVii Icon to the Home Menu",
        TVII_ICON_HBM_PATCH_DEFAULT_VALUE, tviiIconHBM, tviiIconHBMChanged));
    root.add(WUPSConfigItemBoolean::Create(
        TVII_ICON_WUM_PATCH_COFNIG_ID, "Add TVii Icon to the Wii U Menu",
        TVII_ICON_WUM_PATCH_DEFAULT_VALUE, tviiIconWUM, tviiIconWUMChanged));
  } catch (std::exception &e) {
    DEBUG_FUNCTION_LINE("Creating config menu failed: %s", e.what());
    return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
  }

  return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

// Close event for the Aroma config menu
void ConfigMenuClosedCallback() {
  WUPSStorageAPI::SaveStorage();
  if (needRelaunch) {
    _SYSLaunchTitleWithStdArgsInNoSplash(OSGetTitleID(), nullptr);
    needRelaunch = false;
  }
}

// Config stuff in plugin initialization
void InitializeConfig() {
    // Add the config
  WUPSConfigAPIOptionsV1 configOptions = {.name = "Rosé Patcher"};
  if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback,
                         ConfigMenuClosedCallback) !=
      WUPSCONFIG_API_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("Failed to init config api");
  }

  // Add get saved values
  WUPSStorageError storageRes;
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           CONNECT_TO_ROSE_CONFIG_ID, connectToRose,
           CONNECT_TO_ROSE_DEFUALT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           REPLACE_DLM_CONFIG_ID, replaceDownloadManagement,
           REPLACE_DLM_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           TVII_ICON_HBM_PATCH_COFNIG_ID, tviiIconHBM,
           TVII_ICON_HBM_PATCH_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           TVII_ICON_WUM_PATCH_COFNIG_ID, tviiIconWUM,
           TVII_ICON_WUM_PATCH_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
}