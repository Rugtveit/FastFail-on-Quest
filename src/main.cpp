#include <dlfcn.h>
#include "../extern/beatsaber-hook/shared/utils/utils.h"
#include "../extern/beatsaber-hook/shared/utils/logging.hpp"
#include "../extern/beatsaber-hook/include/modloader.hpp"
#include "../extern/beatsaber-hook/shared/utils/typedefs.h"
#include "../extern/beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "../extern/beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "../extern/beatsaber-hook/shared/config/config-utils.hpp"
#include "../include/fastfail.hpp"



static ModInfo modInfo;
fastfail failSkip;

static Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

static struct Config_t 
{
    bool enabled = true;
    bool autoSkip = true;
} Config;





std::string getSceneStr(Scene scene)
{
    Il2CppString* sceneStr = CRASH_UNLESS(il2cpp_utils::RunMethod<Il2CppString*>(il2cpp_utils::GetClassFromName("UnityEngine.SceneManagement", "Scene"), "GetNameInternal", scene.m_Handle));
    if(sceneStr != nullptr) return to_utf8(csstrtostr(sceneStr));
    return nullptr;
}

std::string game = "GameCore";
bool modEnabled = true;
void OnGameSceneLoaded();
Il2CppObject* FindObjectsOfTypeAllFirst2(std::string_view nameSpace, std::string_view className)
{
    Array<Il2CppObject*>* ObjectArray = CRASH_UNLESS(il2cpp_utils::RunMethod<Array<Il2CppObject*>*>("UnityEngine", "Resources", "FindObjectsOfTypeAll", il2cpp_utils::GetSystemType(nameSpace, className)));
    if(ObjectArray != nullptr) return ObjectArray->values[0];
    return nullptr;
}
//This is to do same as BS_Utils.Utilities.BSEvents.gameSceneLoaded
MAKE_HOOK_OFFSETLESS(SceneManagerOnActiveSceneChanged, void, Scene previousActiveScene, Scene newActiveScene)
{
    SceneManagerOnActiveSceneChanged(previousActiveScene, newActiveScene);
    std::string newActiveSceneName = getSceneStr(newActiveScene);
    if(newActiveSceneName == game)
    {
        Il2CppObject* gameScenesManager = FindObjectsOfTypeAllFirst2("", "GameScenesManager");
        if(gameScenesManager != nullptr)
        {

        }
        OnGameSceneLoaded();
    }
}


void OnGameSceneLoaded()
{
    if(!modEnabled) return;
    // Creating GameObject with BlocksBlade since CustomTypes isn't a thing right now so we just hook up to it and use it as CustomType
    Il2CppObject* failSkipGO = CRASH_UNLESS(il2cpp_utils::NewUnsafe("UnityEngine", "GameObject", il2cpp_utils::createcsstr("FailSkip Behavior")));
    CRASH_UNLESS(il2cpp_utils::RunMethod(failSkipGO, "AddComponent", il2cpp_utils::GetSystemType("", "BlocksBlade"))); 
}

void hasFailed()
{
    failSkip._hasFailed = true;
}


MAKE_HOOK_OFFSETLESS(BlocksBlade_Start, void, Il2CppObject* self)
{
    getLogger().info("Works?");
    failSkip.hasFailed = hasFailed;
    failSkip.autoSkip = false;
    failSkip.Awake();
}

MAKE_HOOK_OFFSETLESS(BlocksBlade_Update, void, Il2CppObject* self)
{
    failSkip.Update();
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
    INSTALL_HOOK_OFFSETLESS(BlocksBlade_Start, il2cpp_utils::FindMethodUnsafe("", "BlocksBlade", "Start", 0));
    INSTALL_HOOK_OFFSETLESS(BlocksBlade_Update, il2cpp_utils::FindMethodUnsafe("", "BlocksBlade", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(SceneManagerOnActiveSceneChanged, il2cpp_utils::FindMethodUnsafe("UnityEngine.SceneManagement", "SceneManager", "Internal_ActiveSceneChanged", 2));
    getLogger().debug("Installed FastFail!");
}
