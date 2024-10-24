#include "main.hpp"
#include "utils/Notification.hpp"
#include "utils/logger.h"

WUPS_PLUGIN_NAME("Rosé Patcher");
WUPS_PLUGIN_DESCRIPTION("Patcher for Project Rosé's Nintendo TVii revival service.");
WUPS_PLUGIN_VERSION("v0.1-alpha");
WUPS_PLUGIN_AUTHOR("Project Rosé Team");
WUPS_PLUGIN_LICENSE("GPLv2");

WUPS_USE_STORAGE("rosepatcher");
WUPS_USE_WUT_DEVOPTAB();

INITIALIZE_PLUGIN() {
  // Initialize libraries
  WHBLogModuleInit();
  WHBLogUdpInit();
  WHBLogCafeInit();
  FunctionPatcher_InitLibrary();

  InitializeConfig();

  // Check if NotificationModule library is initialized
  if (NotificationModule_InitLibrary() != NOTIFICATION_MODULE_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("NotificationModule_InitLibrary failed");
  }

  if (connectToRose) {
    ShowNotification("Rosé patch enabled");
  } else {
    ShowNotification("Rosé patch disabled");
  }
}

DEINITIALIZE_PLUGIN() {

  WHBLogModuleDeinit();
  WHBLogUdpDeinit();
  WHBLogCafeDeinit();
  NotificationModule_DeInitLibrary();
  FunctionPatcher_DeInitLibrary();
}

ON_APPLICATION_START() {
  WHBLogModuleInit();
  WHBLogUdpInit();
  WHBLogCafeInit();

  auto title = OSGetTitleID();
  if (tviiIconWUM) {
    if (title == 0x5001010040000 || title == 0x5001010040100 || title == 0x5001010040200) {
      patches::perform_men_patches(true);
    }
  }
}