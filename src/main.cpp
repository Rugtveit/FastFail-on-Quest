#include <dlfcn.h>
#include "../extern/beatsaber-hook/shared/utils/utils.h"
#include "../extern/beatsaber-hook/shared/utils/logging.hpp"
#include "../extern/beatsaber-hook/include/modloader.hpp"
#include "../extern/beatsaber-hook/shared/utils/typedefs.h"
#include "../extern/beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "../extern/beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "../extern/beatsaber-hook/shared/config/config-utils.hpp"



static ModInfo modInfo;

static Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

bool modEnabled = true;

Vector3 eulerAngles = {0.0f, 0.0f, 0.0f};
enum LevelEndAction
{
    None,
    Quit,
    Restart,
    LostConnection,
    RoomDestroyed
}

enum LevelEndStateType
{
    None,
    Cleared,
    Failed
}

MAKE_HOOK_OFFSETLESS(StandardLevelFailedController_HandleLevelFailed, void, Il2CppObject* self)
{
    if(!modEnabled) return StandardLevelFailedController_HandleLevelFailed(self);
    //base.transform.eulerAngles = new Vector3(0f, this._environmentSpawnRotation.targetRotation, 0f);
    Il2CppObject* environmentSpawnRotation = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_environmentSpawnRotation"));
    eulerAngles.y = CRASH_UNLESS(il2cpp_utils::GetPropertyValue<float>(environmentSpawnRotation, "targetRotation"));
    Il2CppObject* baseTransform = CRASH_UNLESS(il2cpp_utils::GetPropertyValue(self, "transform"));
    CRASH_UNLESS(il2cpp_utils::SetPropertyValue(baseTransform, "eulerAngles", eulerAngles));
    
    //LevelCompletionResults.LevelEndAction levelEndAction = this._initData.autoRestart ? LevelCompletionResults.LevelEndAction.Restart : LevelCompletionResults.LevelEndAction.None;
    Il2CppObject* initData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_initData"));
    bool autoRestart = CRASH_UNLESS(il2cpp_utils::GetFieldValue<bool>(initData, "autoRestart"));
    int levelEndAction = autoRestart ? LevelEndAction.Restart : LevelEndAction.None; 
    
    //LevelCompletionResults levelCompletionResults = this._prepareLevelCompletionResults.FillLevelCompletionResults(LevelCompletionResults.LevelEndStateType.Failed, levelEndAction);
    Il2CppObject* prepareLevelCompletionResults = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_prepareLevelCompletionResults"));
    Il2CppObject* levelCompletionResults = CRASH_UNLESS(il2cpp_utils::RunMethod(prepareLevelCompletionResults, "FillLevelCompletionResults", (int)LevelEndStateType.Failed, levelEndAction));

    //this._gameSongController.FailStopSong();
    Il2CppObject* gameSongController = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_gameSongController"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(gameSongController, "FailStopSong"));

    //this._beatmapObjectSpawnController.StopSpawning();
    Il2CppObject* beatmapObjectSpawnController = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_beatmapObjectSpawnController"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(beatmapObjectSpawnController, "StopSpawning"));

    //this._beatmapObjectManager.DissolveAllObjects();
    Il2CppObject* beatmapObjectManager = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_beatmapObjectManager"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(beatmapObjectManager, "DissolveAllObjects"));

    //this._levelFailedTextEffect.ShowEffect();
    Il2CppObject* levelFailedTextEffect = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_levelFailedTextEffect"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(levelFailedTextEffect, "ShowEffect"));

    //this._standardLevelSceneSetupData.Finish(levelCompletionResults);
    Il2CppObject* standardLevelSceneSetupData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "standardLevelSceneSetupData"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(standardLevelSceneSetupData, "Finish", levelCompletionResults));

}



// This function is called when the mod is loaded for the first time, immediately after il2cpp_init.
extern "C" void load()
{
    getLogger().debug("Installing FastFail!");
    INSTALL_HOOK_OFFSETLESS(StandardLevelFailedController_HandleLevelFailed, il2cpp_utils::FindMethodUnsafe("","StandardLevelFailedController","HandleLevelFailed", 0));
    getLogger().debug("Installed FastFail!");
}
