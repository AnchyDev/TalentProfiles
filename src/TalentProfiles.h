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

struct TalentProfile
{
    std::string name;
    std::vector<uint32> talents;
};

class TalentProfilesGossipScript : public GameObjectScript
{
public:
    TalentProfilesGossipScript() : GameObjectScript("TalentProfilesGossipScript") { }

private:
    bool OnGossipHello(Player* /*player*/, GameObject* /*go*/) override;
    bool OnGossipSelect(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/) override;
    bool OnGossipSelectCode(Player* /*player*/, GameObject* /*go*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) override;

    bool TryCreateProfile(Player* /*player*/, const char* /*name*/);
    bool HasTalentProfiles(Player* /*player*/);
    std::map<const char*, TalentProfile>* GetTalentProfiles(Player* /*player*/);

private:
    std::map<uint64, std::map<const char*, TalentProfile>> TalentProfiles;
};

#endif // MODULE_TALENT_PROFILES_H
