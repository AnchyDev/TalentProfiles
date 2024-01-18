#include "TalentProfiles.h"
#include "TalentProfilesHandler.h"

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

        if (!sTalentProfiles->HasTalentProfiles(player))
        {
            ChatHandler(player->GetSession()).SendSysMessage("You do not have any talent profiles!");
            CloseGossipMenuFor(player);
            return false;
        }

        auto talentProfiles = sTalentProfiles->GetTalentProfiles(player);
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
        TalentProfile* profile = sTalentProfiles->GetProfileByIndex(player, profileIndex);

        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, Acore::StringFormatFmt("Profile: {}", profile->name), GOSSIP_SENDER_MAIN, 5000 + profileIndex);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Activate Profile", GOSSIP_SENDER_MAIN, 7000 + profileIndex, "Are you sure you want to overwrite your current talents?", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Delete Profile", GOSSIP_SENDER_MAIN, 6000 + profileIndex, "Are you sure you want to delete this talent profile?", 0, false);

        SendGossipMenuFor(player, 555444, go->GetGUID());
    }

    if (action >= 6000 && action < 7000)
    {
        uint32 profileIndex = action - 6000;
        TalentProfile* profile = sTalentProfiles->GetProfileByIndex(player, profileIndex);

        if (sTalentProfiles->TryDeleteProfile(player, profile->name))
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
        TalentProfile* profile = sTalentProfiles->GetProfileByIndex(player, profileIndex);

        if (sTalentProfiles->TryActivateTalents(player, profile))
        {
            ChatHandler(player->GetSession()).SendSysMessage("Activated profile.");
        }
        else
        {
            ChatHandler(player->GetSession()).SendSysMessage("There was an issue while trying to activate that profile.");
        }

        CloseGossipMenuFor(player);
    }

    return true;
}

bool TalentProfilesGossipScript::OnGossipSelectCode(Player* player, GameObject* go, uint32 /*sender*/, uint32 action, const char* code)
{
    if (action == 1)
    {
        std::string name(code);

        if (!sTalentProfiles->TryCreateProfile(player, name))
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

void SC_AddTalentProfilesScripts()
{
    new TalentProfilesGossipScript();
}
