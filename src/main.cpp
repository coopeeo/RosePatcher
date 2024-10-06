// Nova is real I think
// Note: used some of Inkay, the Pretendo Network plugin source code as a base
// and to make use of the notfications. Credit to Pretendo for developing Inkay
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

#include "Notification.h"
#include "utils/logger.h"

#include "rose_config_txt.h" // included at runtime
#include "rootca_der.h" // included at runtime

WUPS_PLUGIN_NAME("Rosé Patcher");
WUPS_PLUGIN_DESCRIPTION("Patcher for Project Rosé's Nintendo TVii revival service.");
WUPS_PLUGIN_VERSION("v0.1-alpha");
WUPS_PLUGIN_AUTHOR("Project Rosé Team");
WUPS_PLUGIN_LICENSE("GPLv2");

WUPS_USE_STORAGE("rosepatcher");
WUPS_USE_WUT_DEVOPTAB();

#define VINO_TITLE_ID_JP 0x000500301001300A
#define VINO_CLIENT_ID_JP "547e315c1966905040e2d48dff24439a"

#define VINO_TITLE_ID_US 0x000500301001310A
#define VINO_CLIENT_ID_US "87a44dad436407e4ec47ad42ed68bf8c"

#define VINO_TITLE_ID_EU 0x000500301001320A
#define VINO_CLIENT_ID_EU "8bc9387d0797e003c3210acfae01e109"

#define VINO_CONFIG_PATH "/vol/content/vino_config.txt"
#define VINO_CA_PATH "/vol/content/rootca/Go_Daddy_Root_Certificate_Authority_-_G2.der"
#define VINO_CA_2_PATH "/vol/content/rootca/Go_Daddy_Class_2_Certification_Authority.der"

#define FAKE_INDEPENDANT_SERVICE_TOKEN_CONFIG_ID "fake_independant_service_token"
#define FAKE_INDEPENDANT_SERVICE_TOKEN_DEFUALT_VALUE false

#define PATCH_CA_CONFIG_ID "patch_ca"
#define PATCH_CA_DEFUALT_VALUE true

#define CONNECT_TO_ROSE_CONFIG_ID "connect_to_rose"
#define CONNECT_TO_ROSE_DEFUALT_VALUE true

#define REPLACE_DLM_CONFIG_ID "replace_download_management"
#define REPLACE_DLM_DEFAULT_VALUE false

FSMountSource mSource;
char mPath[128] = "";

bool hasPatchedAIST = false;

static std::optional<FSFileHandle> config_handle{};
static std::optional<FSFileHandle> ca_handle{};
static std::optional<FSFileHandle> ca2_handle{};

// Settings
bool patchAIST = FAKE_INDEPENDANT_SERVICE_TOKEN_DEFUALT_VALUE;
bool patchCA = PATCH_CA_DEFUALT_VALUE;
bool connectToRose = CONNECT_TO_ROSE_DEFUALT_VALUE;
bool replaceDownloadManagement = REPLACE_DLM_DEFAULT_VALUE;
bool needRelaunch = false;

// patch INDEPENDANT SERVICE TOKEN setting event
void patchAISTChanged(ConfigItemBoolean *item, bool newValue) {
  if (newValue != patchAIST) {
    WUPSStorageAPI::Store(FAKE_INDEPENDANT_SERVICE_TOKEN_CONFIG_ID, newValue);
  }

  patchAIST = newValue;
}
// Patch root CA setting event
void patchCAChanged(ConfigItemBoolean *item, bool newValue) {
  if (patchCA != newValue) {
    WUPSStorageAPI::Store(PATCH_CA_CONFIG_ID, newValue);
  }

  patchCA = newValue;
}

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

// Open event for the Aroma config menu
WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
  WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

  try {
    // Add setting items
    root.add(WUPSConfigItemBoolean::Create(
        FAKE_INDEPENDANT_SERVICE_TOKEN_CONFIG_ID, "Fake Independent Service Token Patch Enabled",
        FAKE_INDEPENDANT_SERVICE_TOKEN_DEFUALT_VALUE, patchAIST, patchAISTChanged));
    root.add(WUPSConfigItemBoolean::Create(
        PATCH_CA_CONFIG_ID, "Root CA Patch Enabled",
        PATCH_CA_DEFUALT_VALUE, patchCA,
        patchCAChanged));
    root.add(WUPSConfigItemBoolean::Create(
        CONNECT_TO_ROSE_CONFIG_ID, "Config Patch Enabled",
        CONNECT_TO_ROSE_DEFUALT_VALUE, connectToRose, connectToRoseChanged));
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
// Close event for the Aroma config menu
void ConfigMenuClosedCallback() {
  WUPSStorageAPI::SaveStorage();
  if (needRelaunch) {
    _SYSLaunchTitleWithStdArgsInNoSplash(OSGetTitleID(), nullptr);
    needRelaunch = false;
  }
}

bool isVinoClientID(const char *client_id) {
  return strcmp(client_id, VINO_CLIENT_ID_JP) == 0 ||
         strcmp(client_id, VINO_CLIENT_ID_US) == 0 ||
         strcmp(client_id, VINO_CLIENT_ID_EU) == 0;
}

bool isVinoTitleID(uint32_t title_id) {
  return title_id == VINO_TITLE_ID_JP || 
         title_id == VINO_TITLE_ID_US ||
         title_id == VINO_TITLE_ID_EU;
}

INITIALIZE_PLUGIN() {
  // Initialize libraries
  WHBLogUdpInit();
  WHBLogCafeInit();
  FunctionPatcher_InitLibrary();

  // Add the config
  WUPSConfigAPIOptionsV1 configOptions = {.name = "Rose Patcher"};
  if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback,
                         ConfigMenuClosedCallback) !=
      WUPSCONFIG_API_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("Failed to init config api");
  }

  // Add get saved values
  WUPSStorageError storageRes;
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           FAKE_INDEPENDANT_SERVICE_TOKEN_CONFIG_ID, patchAIST,
           FAKE_INDEPENDANT_SERVICE_TOKEN_DEFUALT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           PATCH_CA_CONFIG_ID, patchCA,
           PATCH_CA_DEFUALT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
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

  // Check if NotificationModule library is initialized
  if (NotificationModule_InitLibrary() != NOTIFICATION_MODULE_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("NotificationModule_InitLibrary failed :(");
  }

  if (connectToRose) {
    ShowNotification("Rosé patch enabled");
  } else {
    ShowNotification("Rosé patch disabled");
  }
}

DEINITIALIZE_PLUGIN() {
  WHBLogUdpDeinit();
  WHBLogCafeDeinit();
  NotificationModule_DeInitLibrary();
  FunctionPatcher_DeInitLibrary();
}

ON_APPLICATION_START() {
  WHBLogUdpInit();
  WHBLogCafeInit();

  hasPatchedAIST = false;
  DEBUG("Rosé Patcher: An application has started");
}

// Patch Wii U Menu and Home Menu's Download Management buttons
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

// Patch Vino Config
DECL_FUNCTION(FSStatus, FSOpenFile_VINO, FSClient *client, FSCmdBlock *block,
              const char *path, const char *mode, FSFileHandle *handle,
              FSErrorFlag errorMask) {

  DEBUG("Wii wants to open file: %s", path);

  if (connectToRose && strcmp(VINO_CONFIG_PATH, path) == 0) {
    FSStatus res = real_FSOpenFile_VINO(client, block, path, mode, handle, errorMask);
    config_handle = *handle;
    return res;
  }
  
  if (patchCA && strcmp(VINO_CA_PATH, path) == 0) {
    FSStatus res = real_FSOpenFile_VINO(client, block, path, mode, handle, errorMask);
    ca_handle = *handle;
    return res;
  }

  if (patchCA && strcmp(VINO_CA_2_PATH, path) == 0) {
    FSStatus res = real_FSOpenFile_VINO(client, block, path, mode, handle, errorMask);
    ca2_handle = *handle;
    return res;
  }

  return real_FSOpenFile_VINO(client, block, path, mode, handle, errorMask);
}

DECL_FUNCTION(FSStatus, FSReadFile_VINO, FSClient *client, FSCmdBlock *block, uint8_t *buffer, uint32_t size, uint32_t count,
              FSFileHandle handle, uint32_t unk1, uint32_t flags) {
    if (size != 1) {
        DEBUG_FUNCTION_LINE("Falied to patch vino config. Size is not 1");
    }

    if (config_handle && *config_handle == handle) {
        DEBUG_FUNCTION_LINE("Trying to read vino config detected, returning patched data.");
        strlcpy((char *) buffer, (const char *) rose_config_txt, size * count);
        return (FSStatus) count;
    }

    if (ca_handle && *ca_handle == handle) {
        DEBUG_FUNCTION_LINE("Trying to read vino certificate detected, returning patched data.");
        strlcpy((char *) buffer, (const char *) rootca_der, size * count);
        return (FSStatus) count;
    }

    if (ca2_handle && *ca2_handle == handle) {
        DEBUG_FUNCTION_LINE("Trying to read vino certificate 2 detected, returning patched data.");
        strlcpy((char *) buffer, (const char *) rootca_der, size * count);
        return (FSStatus) count;
    }

    return real_FSReadFile_VINO(client, block, buffer, size, count, handle, unk1, flags);
}

DECL_FUNCTION(FSStatus, FSCloseFile_VINO, FSClient *client, FSCmdBlock *block, FSFileHandle handle, FSErrorFlag errorMask) {
    if (handle == config_handle) {
        DEBUG("Closing Vino config file and resetting handle");
        config_handle.reset();
    }
    if (handle == ca_handle) {
        DEBUG("Closing Vino ca file and resetting handle");
        ca_handle.reset();
    }
    if (handle == ca2_handle) {
        DEBUG("Closing Vino ca2 file and resetting handle");
        ca2_handle.reset();
    }
    return real_FSCloseFile_VINO(client, block, handle, errorMask);
}

// Patch AcquireIndependentServiceToken to fake success for Vino (via FSInit)
DECL_FUNCTION(int, AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4, uint8_t* token, const char* client_id) 
{ // function is patched in the FSInit_Vino function

    if (client_id && isVinoClientID(client_id) && !hasPatchedAIST) {
        hasPatchedAIST = true;
        DEBUG_FUNCTION_LINE("Faking service sucess for '%s' (should be Vino)", client_id);
        return 0;
    } 

    return real_AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4(token, client_id);
}

DECL_FUNCTION(void, FSInit_VINO)
{
    OSDynLoad_Module NN_ACT = 0;
    uint32_t AISTaddressVIR = 0;
    uint32_t AISTaddress = 0;


    // Call the original function
    real_FSInit_VINO();

    if (!patchAIST) {
        DEBUG_FUNCTION_LINE("AcquireIndependentServiceToken patch is disabled, skipping...");
        return;
    }

    if (hasPatchedAIST) {
        DEBUG_FUNCTION_LINE("FSInit_VINO has already been patched, and patched AcquireIndependentServiceToken");
        return;
    }

    // Notify about the patch
    DEBUG("Rosé Patcher: Trying to patch AcquireIndependentServiceToken via FSInit\n");

    // Acquire the nn_act module
    if(OSDynLoad_Acquire("nn_act.rpl", &NN_ACT) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE("failed to acquire nn_act module");
        return;
    }

    // Find the AcquireIndependentServiceToken function (well their addresses)

      // Get the physical address
    if(OSDynLoad_FindExport(NN_ACT, OS_DYNLOAD_EXPORT_FUNC, "AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4", (void**)&AISTaddressVIR) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE("Failed to find AcquireIndependentServiceToken function in nn_act");
        return;
    }

      // Get the virtual address
    AISTaddress = OSEffectiveToPhysical(AISTaddressVIR);

    // Print the results of physical and virtual addresses from the AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4 function
    DEBUG("AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4 results\n"
                 "Rosé Patcher:   Physical Address: %d\n"
                 "Rosé Patcher:   Virtual Address: %d\n"
                 "Rosé Patcher: -- END OF AcquireIndependentServiceToken ADDRESS RESULTS --",
                 &AISTaddress, &AISTaddressVIR);

    // Make function replacement data
    function_replacement_data_t AISTpatch = REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(
      AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4,
      AISTaddress,
      AISTaddressVIR,
      FP_TARGET_PROCESS_TVII);

    // Patch the function
    PatchedFunctionHandle AISTpatchHandle = 0;

    bool AISTpatchSuccess = false;
    
    if(FunctionPatcher_AddFunctionPatch(&AISTpatch, &AISTpatchHandle, &AISTpatchSuccess) != FunctionPatcherStatus::FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE("Failed to add patch.");
        return;
    }

    if (AISTpatchSuccess == false) {
        DEBUG_FUNCTION_LINE("Failed to add patch.");
        return;
    }

    // Notify about the patch success
    DEBUG("Patched AcquireIndependentServiceToken via FSInit");
}




WUPS_MUST_REPLACE_FOR_PROCESS(FSInit_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSInit, WUPS_FP_TARGET_PROCESS_TVII);
WUPS_MUST_REPLACE(_SYSSwitchTo, WUPS_LOADER_LIBRARY_SYSAPP, _SYSSwitchTo);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(_SYSSwitchToOverlayFromHBM, 0x2E47373C, 0x0E47373C, WUPS_FP_TARGET_PROCESS_HOME_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSOpenFile, WUPS_FP_TARGET_PROCESS_TVII);
WUPS_MUST_REPLACE_FOR_PROCESS(FSReadFile_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSReadFile, WUPS_FP_TARGET_PROCESS_TVII);
WUPS_MUST_REPLACE_FOR_PROCESS(FSCloseFile_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSCloseFile, WUPS_FP_TARGET_PROCESS_TVII);
