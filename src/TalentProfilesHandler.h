#ifndef MODULE_TALENT_PROFILES_HANDLER_H
#define MODULE_TALENT_PROFILES_HANDLER_H

#include "Player.h"

#include <vector>
#include <map>

struct TalentInfo
{
    uint32 spellId;
    uint32 talentId;
};

struct ActionBarInfo
{
    uint32 actionId;
    uint32 actionType;
    uint32 slot;
};

struct TalentProfile
{
    std::string name;
    uint32 remainingPoints;
    std::vector<TalentInfo> talents;
    std::vector<ActionBarInfo> actions;
};

class TalentProfilesHandler
{
private:
    TalentProfilesHandler() { }

public:
    static TalentProfilesHandler* GetInstance()
    {
        static TalentProfilesHandler instance;

        return &instance;
    }

public:
    void LoadProfiles();
    bool TryCreateProfile(Player* /*player*/, std::string /*name*/);
    bool TryDeleteProfile(Player* /*player*/, std::string /*name*/);
    bool TryActivateTalents(Player* /*player*/, TalentProfile* /*profile*/);
    bool HasTalentProfiles(Player* /*player*/);
    std::map<std::string, TalentProfile>* GetTalentProfiles(Player* /*player*/);
    TalentProfile* GetProfileByIndex(Player* /*player*/, uint32 /*index*/);

    std::vector<TalentInfo> GetTalents(Player* /*player*/);
    std::vector<TalentInfo> GetTalents(std::string /*encodedString*/);
    std::vector<ActionBarInfo> GetActions(Player* /*player*/);

private:
    std::map<uint64, std::map<std::string, TalentProfile>> TalentProfiles;
};

#define sTalentProfiles TalentProfilesHandler::GetInstance()

#endif // MODULE_TALENT_PROFILES_HANDLER_H
