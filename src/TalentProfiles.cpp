#include "TalentProfiles.h"

bool TalentProfilesGossipScript::OnGossipHello(Player* player, GameObject* go)
{
    ClearGossipMenuFor(player);

    //|TInterface\\Icons\\Test_icon:24|t
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Create Profile", GOSSIP_SENDER_MAIN, 1, "Please enter a name for your talent profile.", 0, true);
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "View Profiles", GOSSIP_SENDER_MAIN, 2);

    SendGossipMenuFor(player, TALENT_PROFILES_GOSSIP_TEXT_ID, go->GetGUID());

    return true;
}

bool TalentProfilesGossipScript::OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action)
{
    if (action == 1)
    {
        CloseGossipMenuFor(player);
    }

    if (action == 2)
    {
        ClearGossipMenuFor(player);

        if (!HasTalentProfiles(player))
        {
            ChatHandler(player->GetSession()).SendSysMessage("You do not have any talent profiles!");
            CloseGossipMenuFor(player);
            return false;
        }

        auto talentProfiles = GetTalentProfiles(player);
        if (!talentProfiles)
        {
            ChatHandler(player->GetSession()).SendSysMessage("There was an error while retrieving your talent profiles.");
            CloseGossipMenuFor(player);
            return false;
        }

        uint32 index = 0;

        for (auto it = talentProfiles->begin(); it != talentProfiles->end(); ++it)
        {
            index++;

            auto profile = it->second;

            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormatFmt("{}: {} talents.", profile.name, profile.talents.size()), GOSSIP_SENDER_MAIN, 5000 + index);
        }

        SendGossipMenuFor(player, 555444, go->GetGUID());
    }

    return true;
}

bool TalentProfilesGossipScript::OnGossipSelectCode(Player* player, GameObject* go, uint32 /*sender*/, uint32 action, const char* code)
{
    if (action == 1)
    {
        if (!TryCreateProfile(player, code))
        {
            CloseGossipMenuFor(player);
            return false;
        }
        else
        {
            ChatHandler(player->GetSession()).SendSysMessage("Talent profile created!");
            OnGossipHello(player, go);
        }
    }

    return true;
}

bool TalentProfilesGossipScript::TryCreateProfile(Player* player, const char* name)
{
    auto talentProfiles = GetTalentProfiles(player);
    if (!talentProfiles)
    {
        std::map<const char*, TalentProfile> profileMap;
        TalentProfiles.emplace(player->GetGUID().GetRawValue(), profileMap);

        talentProfiles = GetTalentProfiles(player);
    }

    if (talentProfiles->find(name) != talentProfiles->end()) //TODO: Module fails here for whatever reason.
    {
        ChatHandler(player->GetSession()).SendSysMessage("A talent profile with that name already exists!");
        return false;
    }

    TalentProfile talentProfile;
    talentProfile.name = name;

    auto talentMap = player->GetTalentMap();
    for (auto talent : talentMap)
    {
        auto spellId = talent.first;
        talentProfile.talents.push_back(spellId);
    }

    talentProfiles->emplace(name, talentProfile);

    return true;
}

bool TalentProfilesGossipScript::HasTalentProfiles(Player* player)
{
    auto it = TalentProfiles.find(player->GetGUID().GetRawValue());
    if (it == TalentProfiles.end())
    {
        return false;
    }

    if (it->second.size() < 1)
    {
        return false;
    }

    return true;
}

std::map<const char*, TalentProfile>* TalentProfilesGossipScript::GetTalentProfiles(Player* player)
{
    auto it = TalentProfiles.find(player->GetGUID().GetRawValue());
    if (it == TalentProfiles.end())
    {
        return nullptr;
    }

    return &it->second;
}

void SC_AddTalentProfilesScripts()
{
    new TalentProfilesGossipScript();
}
