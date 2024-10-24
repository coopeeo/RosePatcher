#include "../main.hpp"
#include "../utils/Notification.hpp"
#include "../utils/logger.h"

PatchedFunctionHandle AISTpatchHandleBetter = 0;

DECL_FUNCTION(int, AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4, uint8_t* token, const char* client_id) {

    if (client_id && isVinoClientID(client_id) && connectToRose) {
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
    bool isAlreadyPatched = false;

    real_FSInit_VINO();

    WHBLogModuleInit();
    WHBLogUdpInit();
    WHBLogCafeInit();

    if (!connectToRose) {
        DEBUG_FUNCTION_LINE("AcquireIndependentServiceToken patch is disabled, skipping...");
        return;
    }

    FunctionPatcher_IsFunctionPatched(AISTpatchHandleBetter, &isAlreadyPatched);

    if (isAlreadyPatched == true) {
        DEBUG_FUNCTION_LINE("FSInit_VINO has already been patched, and patched AcquireIndependentServiceToken (FAILED AT FUNCTIONPATCHER IS PATCHED CHECK)");
        FunctionPatcher_RemoveFunctionPatch(AISTpatchHandleBetter);
        //return;
    }

    // Notify about the patch
    DEBUG("Rosé Patcher: Trying to patch AcquireIndependentServiceToken via FSInit\n");

    // Acquire the nn_act module
    if(OSDynLoad_Acquire("nn_act.rpl", &NN_ACT) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE("Failed to acquire nn_act.rpl module.");
        return;
    }

    // Find the AcquireIndependentServiceToken function (well their addresses)

      // Get the physical address
    if(OSDynLoad_FindExport(NN_ACT, OS_DYNLOAD_EXPORT_FUNC, "AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4", (void**)&AISTaddressVIR) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE("Failed to find AcquireIndependentServiceToken function in nn_act.rpl");
        OSDynLoad_Release(NN_ACT);
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
        OSDynLoad_Release(NN_ACT);
        return;
    }

    AISTpatchHandleBetter = AISTpatchHandle;

    if (AISTpatchSuccess == false) {
        DEBUG_FUNCTION_LINE("Failed to add patch.");
        OSDynLoad_Release(NN_ACT);
        return;
    }

    // Notify about the patch success
    DEBUG("Patched AcquireIndependentServiceToken via FSInit");
    OSDynLoad_Release(NN_ACT);
}

DECL_FUNCTION(void, FSInit_VINO2) {
    real_FSInit_VINO2();
    WHBLogModuleInit();
    WHBLogUdpInit();
    WHBLogCafeInit();
    DEBUG_FUNCTION_LINE("FSInit_VINO2 has been called");
}

WUPS_MUST_REPLACE_FOR_PROCESS(FSInit_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSInit, WUPS_FP_TARGET_PROCESS_TVII);
//WUPS_MUST_REPLACE_FOR_PROCESS(FSInit_VINO2, WUPS_LOADER_LIBRARY_COREINIT, FSInit, WUPS_FP_TARGET_PROCESS_TVII);
