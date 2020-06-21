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
