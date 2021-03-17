/*
 * Copyright (C) 2021+ ChromieCraft <www.chromiecraft.com>
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "Common.h"
#include "Config.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "World.h"

#define CHROMIE_CONF_STABLE_MAX_PLAYER_LEVEL "Chromie.Stable.MaxPlayerLevel"
#define CHROMIE_STABLE_CAP sConfigMgr->GetIntDefault(CHROMIE_CONF_STABLE_MAX_PLAYER_LEVEL, 19)
#define CHROMIE_BETA_CAP sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)

#define GOSSIP_TEXT_EXP         14736
#define GOSSIP_XP_OFF           "I no longer wish to gain experience."
#define GOSSIP_XP_ON            "I wish to start gaining experience again."

#define SELECT_TESTER_QUERY "SELECT `guid` FROM `chromie_beta_testers` WHERE `isBetaTester` = 1 AND `guid` = %u"
#define INSERT_TESTER_QUERY "INSERT IGNORE INTO `chromie_beta_testers` (`guid`, `isBetaTester`, `comment`) VALUES (%u, 1, CONCAT(NOW(), ' - %s'))"

bool canUnlockExp(Player* player)
{
    // If the player level is equal or higher than the CHROMIE_BETA_CAP, do NOT allow
    if (player->getLevel() >= CHROMIE_BETA_CAP)
    {
        return false;
    }

    // If the player level is lower than CHROMIE_STABLE_CAP, allow to unlock exp
    if (player->getLevel() < CHROMIE_STABLE_CAP)
    {
        return true;
    }

    // Otherwise, allow only if the player is BETA TESTER
    auto result = CharacterDatabase.PQuery(SELECT_TESTER_QUERY, player->GetGUID());

    if (!result)
    {
        return false;
    }

    return result->GetRowCount() > 0;
}

// NOTE: this is almost the same as in AC (npc_experience without _chromie)
// we just added the canUnlockExp check inside it
class NpcExperienceChromie : public CreatureScript
{
public:
    NpcExperienceChromie() : CreatureScript("npc_experience_chromie") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_XP_OFF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_XP_ON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        SendGossipMenuFor(player, GOSSIP_TEXT_EXP, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);
        bool noXPGain = player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        bool doSwitch = false;
        auto toggleXpCost = sWorld->getIntConfig(CONFIG_TOGGLE_XP_COST);

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1://xp off
            {
                if (!noXPGain)//does gain xp
                    doSwitch = true;//switch to don't gain xp
            }
                break;
            case GOSSIP_ACTION_INFO_DEF + 2://xp on
            {
                if (noXPGain)//doesn't gain xp
                    doSwitch = true;//switch to gain xp
            }
                break;
        }
        if (doSwitch)
        {
            if (!player->HasEnoughMoney(toggleXpCost))
            {
                player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
            }
            else if (noXPGain)
            {
                if (canUnlockExp(player))
                {
                    // UNLOCK EXP
                    player->ModifyMoney(-toggleXpCost);
                    player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
                }
            }
            else
            {
                // LOCK EXP
                player->ModifyMoney(-toggleXpCost);
                player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            }
        }
        player->PlayerTalkClass->SendCloseGossip();
        return true;
    }
};

class AutoLockExp: public PlayerScript
{
public:
    AutoLockExp() : PlayerScript("auto_lock_exp") {}

    void OnLevelChanged(Player* player, uint8 oldlevel) override
    {
        if (oldlevel == CHROMIE_STABLE_CAP - 1 || oldlevel == CHROMIE_BETA_CAP - 1)
        {
            player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        }
    }

    bool ShouldBeRewardedWithMoneyInsteadOfExp(Player* player) override
    {
        // NOTE: this is in order to reward the Player with money instead of exp
        // even if the player has not reached the global server cap

        if (player->getLevel() == CHROMIE_STABLE_CAP
            && player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN))
        {
            return true;
        }

        return false;
    }
};

#define CHROMIE_STRING_START 90000
#define TEXT_LEVEL_TOO_LOW CHROMIE_STRING_START+0
#define TEXT_ALREADY_TESTER CHROMIE_STRING_START+1
#define TEXT_TESTER_SUCCESS CHROMIE_STRING_START+2
#define TEXT_ONLY_TESTERS_ALLOWED CHROMIE_STRING_START+3

class ChromieCommandScript : public CommandScript
{
public:
    ChromieCommandScript() : CommandScript("beta_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> betaCommandTable =
        {
            { "activate",   SEC_PLAYER, false,  &HandleBetaActivateCommand,        "" },
        };
        static std::vector<ChatCommand> xpCommandTable =
        {
            { "on",    SEC_PLAYER, false,  &HandleXpOnCommand,         "" },
            { "off",   SEC_PLAYER, false,  &HandleXpOffCommand,        "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "beta",   SEC_PLAYER, false,nullptr,  "", betaCommandTable },
            { "xp",     SEC_PLAYER, false,nullptr,  "", xpCommandTable },
        };
        return commandTable;
    }

    static bool HandleXpOnCommand(ChatHandler* handler, char const* /*args*/)
    {
        auto player = handler->GetSession()->GetPlayer();

        if (!player)
        {
            return false;
        }

        if (canUnlockExp(player))
        {
            player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        }
        else if (player->getLevel() < CHROMIE_BETA_CAP)
        {
            handler->PSendSysMessage(TEXT_ONLY_TESTERS_ALLOWED);
            return true;
        }

        return true;
    }

    static bool HandleXpOffCommand(ChatHandler* handler, char const* /*args*/)
    {
        auto player = handler->GetSession()->GetPlayer();

        if (!player)
        {
            return false;
        }

        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        return true;
    }

    static bool HandleBetaActivateCommand(ChatHandler* handler, char const* /*args*/)
    {
        auto player = handler->GetSession()->GetPlayer();

        if (!player)
        {
            return false;
        }

        if (player->getLevel() < CHROMIE_STABLE_CAP)
        {
            handler->PSendSysMessage(TEXT_LEVEL_TOO_LOW, CHROMIE_STABLE_CAP);
            return true;
        }

        auto result = CharacterDatabase.PQuery(SELECT_TESTER_QUERY, player->GetGUID());

        if (result && result->GetRowCount() > 0)
        {
            handler->PSendSysMessage(TEXT_ALREADY_TESTER);
            return true;
        }

        CharacterDatabase.PQuery(INSERT_TESTER_QUERY, player->GetGUID(), player->GetName());

        if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN))
        {
            player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        }

        handler->PSendSysMessage(TEXT_TESTER_SUCCESS);

        return true;
    }
};


void AddChromieXpScripts() {
    new NpcExperienceChromie();
    new AutoLockExp();
    new ChromieCommandScript();
}

