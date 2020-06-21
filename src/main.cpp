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

std::string getSceneStr(Scene scene)
{
    Il2CppString* sceneStr = CRASH_UNLESS(il2cpp_utils::RunMethod<Il2CppString*>("UnityEngine.SceneManagement", "Scene", "GetNameInternal", scene.m_Handle));
    if(sceneStr != nullptr) return to_utf8(csstrtostr(sceneStr));
    return nullptr;
}

std::string game = "GameCore";
bool modEnabled = false;
void OnGameSceneLoaded();
//This is to do same as BS_Utils.Utilities.BSEvents.gameSceneLoaded
MAKE_HOOK_OFFSETLESS(SceneManagerOnActiveSceneChanged, void, Scene previousActiveScene, Scene newActiveScene)
{
    std::string previousActiveSceneName = getSceneStr(previousActiveScene);
    std::string newActiveSceneName = getSceneStr(newActiveScene);
    if(newActiveSceneName == game)
    {
        OnGameSceneLoaded();
    }
}


void OnGameSceneLoaded()
{
    if(!modEnabled) return nullptr;
    // Creating GameObject with BlocksBlade since CustomTypes isn't a thing right now so we just hook up to it and use it as CustomType
    Il2CppObject* failSkipGO = CRASH_UNLESS(il2cpp_utils::New("UnityEngine", "GameObject", il2cpp_utils::createcsstr("FailSkip Behavior")));
    CRASH_UNLESS(il2cpp_utils::RunMethod(failSkipGO, "AddComponent", il2cpp_utils::GetSystemType("", "BlocksBlade"))); 
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
    getLogger().debug("Installed FastFail!");
}
