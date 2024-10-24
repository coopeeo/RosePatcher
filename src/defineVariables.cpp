#include "main.hpp"

#define VINO_TITLE_ID_JP 0x000500301001300A
#define VINO_CLIENT_ID_JP "547e315c1966905040e2d48dff24439a"

#define VINO_TITLE_ID_US 0x000500301001310A
#define VINO_CLIENT_ID_US "87a44dad436407e4ec47ad42ed68bf8c"

#define VINO_TITLE_ID_EU 0x000500301001320A
#define VINO_CLIENT_ID_EU "8bc9387d0797e003c3210acfae01e109"

// Settings
bool connectToRose = CONNECT_TO_ROSE_DEFUALT_VALUE;
bool tviiIconHBM = TVII_ICON_HBM_PATCH_DEFAULT_VALUE;
bool tviiIconWUM = TVII_ICON_WUM_PATCH_DEFAULT_VALUE;
bool needRelaunch = false;

// Vino client and title checks
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