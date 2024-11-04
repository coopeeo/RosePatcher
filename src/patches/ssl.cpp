#include <wups.h>
#include <coreinit/dynload.h>
#include <nsysnet/nssl.h>

#include "ssl.hpp"

#include "../utils/Notification.hpp"
#include "../utils/logger.h"
#include "../utils/patch.hpp"
#include "../config.hpp"

#include "rootx1_der.h" // Included at runtime

namespace patches::ssl {
    void addCertificateToWebKit() {
        OSDynLoad_Module libwkc = 0;
        void* (*WKC_SSLRegisterRootCAByDER)(const char* cert, int cert_len) = nullptr;

        if (!config::connectToRose) {
            return;
        }

        OSDynLoad_Error ret = OSDynLoad_IsModuleLoaded("libwkc", &libwkc);

        if (ret != OS_DYNLOAD_OK || libwkc == 0) {
            DEBUG_FUNCTION_LINE("OSDynLoad_Acquire failed to get libwkc.rpl in tvii. (Most likely not loaded). With error: %d", ret);
            return;
        }

        if (OSDynLoad_FindExport(libwkc, OS_DYNLOAD_EXPORT_FUNC, "WKCWebKitSSLRegisterRootCAByDER__3WKCFPCci", (void **) &WKC_SSLRegisterRootCAByDER) != OS_DYNLOAD_OK) {
            DEBUG_FUNCTION_LINE("FindExport setDeveloperExtrasEnabled__Q2_3WKC11WKCSettingsFb failed.");
            return;
        }

        void* ret1 = WKC_SSLRegisterRootCAByDER((const char *)(rootx1_der), rootx1_der_size);
        // DEBUG("Added ISRG Root X1 certificate to WebKit. code: %d", ret1);
    }
}; // namespace ssl

DECL_FUNCTION(NSSLError, NSSLAddServerPKI, NSSLContextHandle context, NSSLServerCertId pki) {
    if (config::connectToRose) {
        NSSLError ret = NSSLAddServerPKIExternal(context, rootx1_der, rootx1_der_size, 0);
        // DEBUG("Added ISRG Root X1 certificate to NSSL context. code: %d", ret);
    }

    return real_NSSLAddServerPKI(context, pki);
}

WUPS_MUST_REPLACE_FOR_PROCESS(NSSLAddServerPKI, WUPS_LOADER_LIBRARY_NSYSNET, NSSLAddServerPKI, WUPS_FP_TARGET_PROCESS_TVII);