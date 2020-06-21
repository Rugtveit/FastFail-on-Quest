#include "../include/FailSkip.hpp"

Il2CppObject* FindObjectsOfTypeAllFirst(std::string_view nameSpace, std::string_view className)
{
    Array<Il2CppObject*>* ObjectArray = CRASH_UNLESS(il2cpp_utils::RunMethod<Array<Il2CppObject*>*>("UnityEngine", "Resources", "FindObjectsOfTypeAll", il2cpp_utils::GetSystemType(nameSpace, className)));
    if(ObjectArray != nullptr) return ObjectArray->values[0];
    return nullptr;
}

void failskip::hasFailed(failskip* obj, Il2CppObject* asyncOp)
{
    obj->_hasFailed = true;
}
Il2CppObject* PauseMenuManager = nullptr;

void failskip::Awake()
{
    _skipped = false;
    _hasFailed = false;
    _standardLevel = false;
    // Use the appropriate level failed event
    _standardLevelGameplayManager = FindObjectsOfTypeAllFirst("", "StandardLevelGameplayManager");
    if(_standardLevelGameplayManager != nullptr)
    {
        const MethodInfo* addlevelFailedEvent = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe("", "StandardLevelGameplayManager", "add_levelFailedEvent", 0));
        auto addFailedAction = il2cpp_utils::MakeAction(addlevelFailedEvent, 0, this, hasFailed);
        CRASH_UNLESS(il2cpp_utils::RunMethod("", "StandardLevelGameplayManager", "add_levelFailedEvent", addFailedAction));
        _standardLevel = true;
    } 
    else 
    {
        _missionLevelGameplayManager = FindObjectsOfTypeAllFirst("", "MissionLevelGameplayManager");
        if(_missionLevelGameplayManager != nullptr)
        {
            const MethodInfo* addlevelFailedEvent = CRASH_UNLESS(il2cpp_utils::FindMethodUnsafe("", "MissionLevelGameplayManager", "add_levelFailedEvent", 0));
            auto addFailedAction = il2cpp_utils::MakeAction(addlevelFailedEvent, 0, this, hasFailed);
            CRASH_UNLESS(il2cpp_utils::RunMethod("", "MissionLevelGameplayManager", "add_levelFailedEvent", addFailedAction));
            _standardLevel = false; 
        }
    }

    // Get all the necessary fields
    _standardLevelFailedController = FindObjectsOfTypeAllFirst("", "StandardLevelFailedController");
    if(_standardLevelFailedController != nullptr)
    {
        _standardLevelSceneSetupData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_standardLevelFailedController, "_standardLevelSceneSetupData"));
        _standardInitData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_standardLevelSceneSetupData, "_initData"));
        _prepareLevelCompletionResults = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_standardLevelFailedController,"_prepareLevelCompletionResults"));
    }
    else 
    {
        _missionLevelFailedController = FindObjectsOfTypeAllFirst("", "MissionLevelFailedController");
        if(_missionLevelFailedController != nullptr)
        {
            _missionLevelSceneSetupData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_missionLevelFailedController, "_missionLevelSceneSetupData"));
            _missionInitData = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_missionLevelSceneSetupData, "_initData"));
            _missionObjectiveCheckersManager = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_missionLevelFailedController, "_missionObjectiveCheckersManager"));
            _prepareLevelCompletionResults = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_missionLevelFailedController,"_prepareLevelCompletionResults"));
        }
    }
    PauseMenuManager = FindObjectsOfTypeAllFirst("", "PauseMenuManager");
    _vrControllersInputManager = CRASH_UNLESS(il2cpp_utils::GetFieldValue(PauseMenuManager, "_vrControllersInputManager"));
}

void failskip::Update()
{
    if(_hasFailed && (autoSkip || CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(_vrControllersInputManager, "MenuButtonDown"))) && !_skipped)
    {
        if(_standardLevel)
        {
            CRASH_UNLESS(il2cpp_utils::RunMethod(_standardLevelFailedController, "StopAllCoroutines"));
            bool autoRestart = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_standardInitData, "autoRestart"));
            int levelEndAction = autoRestart ? 2 : 0; // autoRestart ? LevelEndAction.Restart : LevelEndAction.None;
            Il2CppObject* levelCompletionResults = CRASH_UNLESS(il2cpp_utils::RunMethod(_prepareLevelCompletionResults, "FillLevelCompletionResults", 2, levelEndAction));
            CRASH_UNLESS(il2cpp_utils::RunMethod(_standardLevelSceneSetupData, "Finish", levelCompletionResults));

        }
        else
        {
            CRASH_UNLESS(il2cpp_utils::RunMethod(_missionLevelFailedController, "StopAllCoroutines"));
            bool autoRestart = CRASH_UNLESS(il2cpp_utils::GetFieldValue(_missionInitData, "autoRestart"));
            int levelEndAction = autoRestart ? 2 : 0; // autoRestart ? LevelEndAction.Restart : LevelEndAction.None;
            Il2CppObject* levelCompletionResults = CRASH_UNLESS(il2cpp_utils::RunMethod(_prepareLevelCompletionResults, "FillLevelCompletionResults", 2, levelEndAction));
            
            Array<Il2CppObject*>* results = CRASH_UNLESS(il2cpp_utils::RunMethod<Array<Il2CppObject*>*>(_missionObjectiveCheckersManager, "GetResults"));
            Il2CppObject* missionCompletionReuslts = CRASH_UNLESS(il2cpp_utils::NewUnsafe("", "MissionCompletionResults", levelCompletionResults, results));
            CRASH_UNLESS(il2cpp_utils::RunMethod(_missionLevelSceneSetupData, "Finish", missionCompletionReuslts));
        }
        _skipped = true;
    }
}