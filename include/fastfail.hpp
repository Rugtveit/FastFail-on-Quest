#include <dlfcn.h>
#include "../extern/beatsaber-hook/shared/utils/utils.h"
#include "../extern/beatsaber-hook/shared/utils/logging.hpp"
#include "../extern/beatsaber-hook/include/modloader.hpp"
#include "../extern/beatsaber-hook/shared/utils/typedefs.h"
#include "../extern/beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "../extern/beatsaber-hook/shared/utils/il2cpp-functions.hpp"

class fastfail : Il2CppObject
{
    public:
        void Update();
        void Awake();
        void Destroy();
        bool autoSkip;
        bool _hasFailed = false;
    private:
        Il2CppObject* _standardLevelGameplayManager; //StandardLevelGameplayManager
        Il2CppObject* _standardLevelFailedController; //StandardLevelFailedController
        Il2CppObject* _missionLevelFailedController; //MissionLevelFailedController
        Il2CppObject* _standardLevelSceneSetupData; //StandardLevelScenesTransitionSetupDataSO
        Il2CppObject* _standardInitData; //StandardLevelFailedController.InitData
        Il2CppObject* _missionLevelSceneSetupData; //MissionLevelScenesTransitionSetupDataSO
        Il2CppObject* _missionInitData; //MissionLevelFailedController.InitData
        Il2CppObject* _missionObjectiveCheckersManager; //MissionObjectiveCheckersManager
        Il2CppObject* _prepareLevelCompletionResults; //PrepareLevelCompletionResults
        Il2CppObject* _vrControllersInputManager; //VRControllersInputManager
        bool _standardLevel;
        bool _skipped = false;
};