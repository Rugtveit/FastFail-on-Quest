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



Vector3 eulerAngles = {0.0f, 0.0f, 0.0f};

enum LevelEndAction
{
    None,
    Quit,
    Restart,
    LostConnection,
    RoomDestroyed
};

Il2CppObject* standardLevelFailedController = nullptr; 

Il2CppObject* getLevelCompletionResults(Il2CppObject* self)
{
    //LevelCompletionResults.LevelEndAction levelEndAction = this._initData.autoRestart ? LevelCompletionResults.LevelEndAction.Restart : LevelCompletionResults.LevelEndAction.None;
    Il2CppObject* initData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_initData"));
    bool autoRestart = CRASH_UNLESS(il2cpp_utils::GetFieldValue<bool>(initData, "autoRestart"));
    int levelEndAction = autoRestart ? LevelEndAction::Restart : LevelEndAction::None; 
    
    //LevelCompletionResults levelCompletionResults = this._prepareLevelCompletionResults.FillLevelCompletionResults(LevelCompletionResults.LevelEndStateType.Failed, levelEndAction);
    Il2CppObject* prepareLevelCompletionResults = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_prepareLevelCompletionResults"));
    Il2CppObject* levelCompletionResults = CRASH_UNLESS(il2cpp_utils::RunMethod(prepareLevelCompletionResults, "FillLevelCompletionResults", 2, levelEndAction)); // 2 = LevelEndStateType.Failed,
    return levelCompletionResults;
}

bool modEnabled = true;
bool autoSkip = false;
bool standardLevel = false;
bool failed = false; // Figure out a way to make it not failed anymore!!
bool skipped = false;
Il2CppObject* vrControllersInputManager = nullptr;
bool menuButtonPressed(Il2CppObject* self)
{
    if(vrControllersInputManager == nullptr) vrControllersInputManager = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_vrControllersInputManager"));
    bool pressed = CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(vrControllersInputManager, "MenuButtonDown"));
    return pressed;

}
MAKE_HOOK_OFFSETLESS(VRController_Update, void, Il2CppObject* self)
{
    if(autoSkip || !failed || !modEnabled) return VRController_Update(self);
    VRController_Update(self);
    if(standardLevel && menuButtonPressed(self) && !skipped)
    {
        getLogger().info("Pressed!");
        Il2CppObject* levelCompletionResults = getLevelCompletionResults(standardLevelFailedController);
        Il2CppObject* standardLevelSceneSetupData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(standardLevelFailedController, "_standardLevelSceneSetupData"));
        CRASH_UNLESS(il2cpp_utils::RunMethod(standardLevelSceneSetupData, "Finish", levelCompletionResults));
        skipped = true;
    }
}

MAKE_HOOK_OFFSETLESS(StandardLevelFailedController_HandleLevelFailed, void, Il2CppObject* self)
{
    failed = true;
    standardLevel = true;
    standardLevelFailedController = self;
    if(!modEnabled || !autoSkip) return StandardLevelFailedController_HandleLevelFailed(self);
    //base.transform.eulerAngles = new Vector3(0f, this._environmentSpawnRotation.targetRotation, 0f);
    Il2CppObject* environmentSpawnRotation = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_environmentSpawnRotation"));
    eulerAngles.y = CRASH_UNLESS(il2cpp_utils::GetPropertyValue<float>(environmentSpawnRotation, "targetRotation"));
    Il2CppObject* baseTransform = CRASH_UNLESS(il2cpp_utils::GetPropertyValue(self, "transform"));
    CRASH_UNLESS(il2cpp_utils::SetPropertyValue(baseTransform, "eulerAngles", eulerAngles));
    
    Il2CppObject* levelCompletionResults = getLevelCompletionResults(self);

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
    Il2CppObject* standardLevelSceneSetupData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(self, "_standardLevelSceneSetupData"));
    CRASH_UNLESS(il2cpp_utils::RunMethod(standardLevelSceneSetupData, "Finish", levelCompletionResults));
}

MAKE_HOOK_OFFSETLESS(StandardLevelScenesTransitionSetupDataSO_Finish, void, Il2CppObject* self, Il2CppObject* levelCompletionResults)
{
    failed = false;
    StandardLevelScenesTransitionSetupDataSO_Finish(self, levelCompletionResults);
}
extern "C" void setup(ModInfo &info)
{
    info.id = "FastFail";
    info.version = "0.1.0";
    modInfo = info;
    getConfig();
    getLogger().info("Completed setup!");
    getLogger().info("Modloader name: %s", Modloader::getInfo().name.c_str());
}  

// This function is called when the mod is loaded for the first time, immediately after il2cpp_init.
extern "C" void load()
{
    getLogger().debug("Installing FastFail!");
    INSTALL_HOOK_OFFSETLESS(StandardLevelFailedController_HandleLevelFailed, il2cpp_utils::FindMethodUnsafe("","StandardLevelFailedController","HandleLevelFailed", 0));
    INSTALL_HOOK_OFFSETLESS(VRController_Update, il2cpp_utils::FindMethodUnsafe("", "VRController", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(StandardLevelScenesTransitionSetupDataSO_Finish, il2cpp_utils::FindMethodUnsafe("", "StandardLevelScenesTransitionSetupDataSO", "Finish", 1));
    getLogger().debug("Installed FastFail!");
}
