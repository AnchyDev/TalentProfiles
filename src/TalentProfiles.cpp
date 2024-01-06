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

            TalentProfile profile = it->second;

            AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormatFmt("{}: {} talents.", profile.name, profile.talents.size()), GOSSIP_SENDER_MAIN, 5000 + index);
        }

        SendGossipMenuFor(player, 555444, go->GetGUID());
    }

    if (action >= 5000 && action < 6000)
    {
        uint32 profileIndex = action - 5000;
        TalentProfile* profile = GetProfileByIndex(player, profileIndex);

        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormatFmt("{}|n", profile->name), GOSSIP_SENDER_MAIN, 5000 + profileIndex);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Activate Profile", GOSSIP_SENDER_MAIN, 7000 + profileIndex, "Are you sure you want to overwrite your current talents?", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Delete Profile", GOSSIP_SENDER_MAIN, 6000 + profileIndex, "Are you sure you want to delete this talent profile?", 0, false);

        SendGossipMenuFor(player, 555444, go->GetGUID());

        ChatHandler(player->GetSession()).SendSysMessage(Acore::StringFormatFmt("Activated Profile: {}", profile->name));
    }

    if (action >= 6000 && action < 7000)
    {
        uint32 profileIndex = action - 6000;
        TalentProfile* profile = GetProfileByIndex(player, profileIndex);

        if (TryDeleteProfile(player, profile->name))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Deleted profile.");
        }
        else
        {
            ChatHandler(player->GetSession()).SendSysMessage("There was an issue while trying to delete that profile.");
        }

        CloseGossipMenuFor(player);
    }

    if (action >= 7000 && action < 8000)
    {
        uint32 profileIndex = action - 7000;
        TalentProfile* profile = GetProfileByIndex(player, profileIndex);

        TryActivateTalents(player, profile);

        CloseGossipMenuFor(player);
    }

    return true;
}

bool TalentProfilesGossipScript::OnGossipSelectCode(Player* player, GameObject* go, uint32 /*sender*/, uint32 action, const char* code)
{
    if (action == 1)
    {
        std::string name(code);

        if (!TryCreateProfile(player, name))
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

bool TalentProfilesGossipScript::TryCreateProfile(Player* player, std::string name)
{
    auto talentProfiles = GetTalentProfiles(player);
    if (!talentProfiles)
    {
        std::map<std::string, TalentProfile> profileMap;
        TalentProfiles.emplace(player->GetGUID().GetRawValue(), profileMap);

        talentProfiles = GetTalentProfiles(player);
    }

    if (talentProfiles->find(name) != talentProfiles->end())
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
        auto playerTalent = talent.second;

        TalentInfo talentInfo;
        talentInfo.spellId = spellId;
        talentInfo.talent = *playerTalent;

        talentProfile.talents.push_back(talentInfo);
    }

    talentProfiles->emplace(name, talentProfile);

    return true;
}

bool TalentProfilesGossipScript::TryDeleteProfile(Player* player, std::string name)
{
    auto talentProfiles = GetTalentProfiles(player);
    if (!talentProfiles)
    {
        return false;
    }

    auto it = talentProfiles->find(name);
    if (it == talentProfiles->end())
    {
        return false;
    }

    talentProfiles->erase(it);

    return true;
}

bool TalentProfilesGossipScript::TryActivateTalents(Player* player, TalentProfile* profile)
{
    if (!player || !profile)
    {
        return false;
    }

    player->resetTalents(true);

    for (auto& talent : profile->talents)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(talent.talent.talentID);

        if (!talentInfo)
        {
            ChatHandler(player->GetSession()).SendSysMessage("An error occured while trying to activate talents, resetting..");
            player->resetTalents(true);
            return false;
        }

        for (int8 rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
        {
            auto spellId = talentInfo->RankID[rank];
            if (spellId == talent.spellId)
            {
                player->LearnTalent(talent.talent.talentID, rank, false);
            }
        }
    }

    player->SendTalentsInfoData(false);

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

std::map<std::string, TalentProfile>* TalentProfilesGossipScript::GetTalentProfiles(Player* player)
{
    auto it = TalentProfiles.find(player->GetGUID().GetRawValue());
    if (it == TalentProfiles.end())
    {
        return nullptr;
    }

    return &it->second;
}

TalentProfile* TalentProfilesGossipScript::GetProfileByIndex(Player* player, uint32 index)
{
    auto profiles = GetTalentProfiles(player);
    uint32 i = 0;

    for (auto it = profiles->begin(); it != profiles->end(); ++it)
    {
        i++;

        if (i == index)
        {
            return &it->second;
        }
    }

    return nullptr;
}

void SC_AddTalentProfilesScripts()
{
    new TalentProfilesGossipScript();
}
