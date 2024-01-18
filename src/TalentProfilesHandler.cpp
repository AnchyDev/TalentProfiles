#include "TalentProfilesHandler.h"
#include "Chat.h"

void TalentProfilesHandler::LoadProfiles()
{
    auto qResult = CharacterDatabase.Query("SELECT * FROM character_talent_profiles");

    do
    {
        auto fields = qResult->Fetch();

        auto guid = fields[0].Get<uint64>();
        auto name = fields[1].Get<std::string>();

        auto encodedTalents = fields[2].Get<std::string>();
        auto talents = GetTalents(encodedTalents);

        auto actions = fields[3].Get<std::string>();

    } while (qResult->NextRow());
}

bool TalentProfilesHandler::TryCreateProfile(Player* player, std::string name)
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
    talentProfile.remainingPoints = player->GetFreeTalentPoints();

    talentProfile.talents = GetTalents(player);
    talentProfile.actions = GetActions(player);

    talentProfiles->emplace(name, talentProfile);

    return true;
}

bool TalentProfilesHandler::TryDeleteProfile(Player* player, std::string name)
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

bool TalentProfilesHandler::TryActivateTalents(Player* player, TalentProfile* profile)
{
    if (!player || !profile)
    {
        return false;
    }

    player->resetTalents(true);

    for (auto& talent : profile->talents)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(talent.talentId);

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
                player->LearnTalent(talent.talentId, rank, true);
            }
        }
    }

    // Clear actionbar
    for (uint8 i = 0; i <= MAX_ACTION_BUTTONS; i++)
    {
        player->removeActionButton(i);
    }

    // Repopulate actionbar
    for (auto& action : profile->actions)
    {
        player->addActionButton(action.slot, action.actionId, action.actionType);
    }

    player->SendActionButtons(1);

    player->SetFreeTalentPoints(profile->remainingPoints);
    player->SendTalentsInfoData(false);

    return true;
}

bool TalentProfilesHandler::HasTalentProfiles(Player* player)
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

std::map<std::string, TalentProfile>* TalentProfilesHandler::GetTalentProfiles(Player* player)
{
    auto it = TalentProfiles.find(player->GetGUID().GetRawValue());
    if (it == TalentProfiles.end())
    {
        return nullptr;
    }

    return &it->second;
}

TalentProfile* TalentProfilesHandler::GetProfileByIndex(Player* player, uint32 index)
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

std::vector<TalentInfo> TalentProfilesHandler::GetTalents(Player* player)
{
    std::vector<TalentInfo> talents;

    auto talentMap = player->GetTalentMap();
    for (auto talent : talentMap)
    {
        auto spellId = talent.first;
        auto playerTalent = talent.second;

        if (playerTalent->State == PLAYERSPELL_REMOVED ||
            !playerTalent->IsInSpec(player->GetActiveSpec()))
        {
            continue;
        }

        TalentInfo talentInfo;
        talentInfo.spellId = spellId;
        talentInfo.talentId = playerTalent->talentID;

        talents.push_back(talentInfo);
    }

    return talents;
}

std::vector<ActionBarInfo> TalentProfilesHandler::GetActions(Player* player)
{
    std::vector<ActionBarInfo> actions;

    for (uint8 i = 0; i <= MAX_ACTION_BUTTONS; i++)
    {
        auto actionButton = player->GetActionButton(i);

        if (!actionButton)
        {
            continue;
        }

        ActionBarInfo actionInfo;
        actionInfo.slot = i;
        actionInfo.actionId = actionButton->GetAction();
        actionInfo.actionType = actionButton->GetType();

        actions.push_back(actionInfo);
    }

    return actions;
}
