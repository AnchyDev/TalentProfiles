#ifndef MODULE_TALENT_PROFILES_H
#define MODULE_TALENT_PROFILES_H

#include "Chat.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

#include <vector>
#include <map>

enum TalentProfilesConstants
{
    TALENT_PROFILES_GOSSIP_TEXT_ID = 555444
};

struct TalentInfo
{
    uint32 spellId;
    PlayerTalent talent;
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

class TalentProfilesGossipScript : public GameObjectScript
{
public:
    TalentProfilesGossipScript() : GameObjectScript("TalentProfilesGossipScript") { }

private:
    bool OnGossipHello(Player* /*player*/, GameObject* /*go*/) override;
    bool OnGossipSelect(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/) override;
    bool OnGossipSelectCode(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) override;

    bool TryCreateProfile(Player* /*player*/, std::string /*name*/);
    bool TryDeleteProfile(Player* /*player*/, std::string /*name*/);
    bool TryActivateTalents(Player* /*player*/, TalentProfile* /*profile*/);
    bool HasTalentProfiles(Player* /*player*/);
    std::map<std::string, TalentProfile>* GetTalentProfiles(Player* /*player*/);
    TalentProfile* GetProfileByIndex(Player* /*player*/, uint32 /*index*/);

    std::vector<TalentInfo> GetTalents(Player* /*player*/);
    std::vector<ActionBarInfo> GetActions(Player* /*player*/);

private:
    std::map<uint64, std::map<std::string, TalentProfile>> TalentProfiles;
};

#endif // MODULE_TALENT_PROFILES_H
