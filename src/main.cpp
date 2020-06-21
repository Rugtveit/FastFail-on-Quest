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


std::string getSceneStr(Scene scene);
void OnGameSceneLoaded();
std::string game = "GameCore";

//This is to do same as BS_Utils.Utilities.BSEvents.gameSceneLoaded
MAKE_HOOK_OFFSETLESS(SceneManagerOnActiveSceneChanged, void, Scene previousActiveScene, Scene newActiveScene)
{
    std::string newActiveSceneName = getSceneStr(newActiveScene);
    if(newActiveSceneName == game) OnGameSceneLoaded();
    SceneManagerOnActiveSceneChanged(previousActiveScene, newActiveScene);
}

fastfail failSkip;
MAKE_HOOK_OFFSETLESS(BlocksBlade_Start, void, Il2CppObject* self)
{
    failSkip.autoSkip = Config.autoSkip;
    failSkip.Destroy();
    failSkip.Awake();
}

MAKE_HOOK_OFFSETLESS(MissionLevelFailedController_HandleLevelFailed, void, Il2CppObject* self)
{
    failSkip._hasFailed = true;
    MissionLevelFailedController_HandleLevelFailed(self);
}

MAKE_HOOK_OFFSETLESS(StandardLevelFailedController_HandleLevelFailed, void, Il2CppObject* self)
{
    failSkip._hasFailed = true;
    StandardLevelFailedController_HandleLevelFailed(self);
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

void SaveConfig();
bool LoadConfig();
// This function is called when the mod is loaded for the first time, immediately after il2cpp_init.
extern "C" void load()
{
    if(!LoadConfig()) SaveConfig();
    getLogger().debug("Installing FastFail!");
    INSTALL_HOOK_OFFSETLESS(BlocksBlade_Start, il2cpp_utils::FindMethodUnsafe("", "BlocksBlade", "Start", 0));
    INSTALL_HOOK_OFFSETLESS(BlocksBlade_Update, il2cpp_utils::FindMethodUnsafe("", "BlocksBlade", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(SceneManagerOnActiveSceneChanged, il2cpp_utils::FindMethodUnsafe("UnityEngine.SceneManagement", "SceneManager", "Internal_ActiveSceneChanged", 2));
    INSTALL_HOOK_OFFSETLESS(StandardLevelFailedController_HandleLevelFailed, il2cpp_utils::FindMethodUnsafe("", "StandardLevelFailedController", "HandleLevelFailed", 0));
    INSTALL_HOOK_OFFSETLESS(MissionLevelFailedController_HandleLevelFailed, il2cpp_utils::FindMethodUnsafe("", "MissionLevelFailedController", "HandleLevelFailed", 0));
    getLogger().debug("Installed FastFail!");
}

std::string getSceneStr(Scene scene)
{
    Il2CppString* sceneStr = CRASH_UNLESS(il2cpp_utils::RunMethod<Il2CppString*>(il2cpp_utils::GetClassFromName("UnityEngine.SceneManagement", "Scene"), "GetNameInternal", scene.m_Handle));
    if(sceneStr != nullptr) return to_utf8(csstrtostr(sceneStr));
    return nullptr;
}

void OnGameSceneLoaded()
{
    if(!Config.enabled) return;
    // Creating GameObject with BlocksBlade since CustomTypes isn't a thing right now so we just hook up to it and use it as CustomType
    Il2CppObject* failSkipGO = CRASH_UNLESS(il2cpp_utils::NewUnsafe("UnityEngine", "GameObject", il2cpp_utils::createcsstr("FailSkip Behavior")));
    CRASH_UNLESS(il2cpp_utils::RunMethod(failSkipGO, "AddComponent", il2cpp_utils::GetSystemType("", "BlocksBlade"))); 
}

void SaveConfig()
{
    getConfig().config.RemoveAllMembers();
    getConfig().config.SetObject();
    auto& allocator = getConfig().config.GetAllocator();
    getConfig().config.AddMember("enabled", Config.enabled, allocator);
    getConfig().config.AddMember("autoSkip", Config.autoSkip, allocator);
    getConfig().Write();
}

bool LoadConfig()
{
    getConfig().Load();
    if(getConfig().config.HasMember("enabled") && getConfig().config["enabled"].IsBool()) Config.enabled = getConfig().config["enabled"].GetBool(); 
    else return false;
    if(getConfig().config.HasMember("autoSkip") && getConfig().config["autoSkip"].IsBool()) Config.autoSkip = getConfig().config["autoSkip"].GetBool(); 
    else return false;

    return true;
}