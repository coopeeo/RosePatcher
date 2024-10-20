#include "../main.hpp"
#include "../utils/Notification.hpp"
#include "../utils/logger.h"

// Patch Wii U Menu and Home Menu's Download Management buttons
DECL_FUNCTION(int32_t, _SYSSwitchTo, SysAppPFID pfid) {
  SysAppPFID uPfid = pfid;

  VPADStatus status;
  VPADReadError err;

  VPADRead(VPAD_CHAN_0, &status, 1, &err);

  WHBLogModuleInit();
  WHBLogUdpInit();
  WHBLogCafeInit();

  // hasPatchedAIST = 0;
  DEBUG_FUNCTION_LINE("Launching Applet via Wii U Menu");

  if (pfid == SYSAPP_PFID_DOWNLOAD_MANAGEMENT) {
    if (replaceDownloadManagement) {
      if (!(status.hold & VPAD_BUTTON_ZL)) {
        uPfid = SYSAPP_PFID_TVII;
      }
    }
  }

  if (uPfid != SYSAPP_PFID_TVII) {
    hasPatchedAIST = 0;
  }

  return real__SYSSwitchTo(uPfid);
}

DECL_FUNCTION(int32_t, _SYSSwitchToOverlayFromHBM, SysAppPFID pfid) {
  SysAppPFID uPfid = pfid;

  VPADStatus status;
  VPADReadError err;

  VPADRead(VPAD_CHAN_0, &status, 1, &err);

  // hasPatchedAIST = 0;
  DEBUG_FUNCTION_LINE("Launching Applet via HBM");

  if (pfid == SYSAPP_PFID_DOWNLOAD_MANAGEMENT) {
    if (replaceDownloadManagement) {
      if (!(status.hold & VPAD_BUTTON_ZL)) {
        uPfid = SYSAPP_PFID_TVII;
      }
    }
  }

  if (uPfid != SYSAPP_PFID_TVII) {
    hasPatchedAIST = 0;
  }

  return real__SYSSwitchToOverlayFromHBM(uPfid);
}

WUPS_MUST_REPLACE(_SYSSwitchTo, WUPS_LOADER_LIBRARY_SYSAPP, _SYSSwitchTo);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(_SYSSwitchToOverlayFromHBM, 0x200083c, 0x400083c, WUPS_FP_TARGET_PROCESS_HOME_MENU);
