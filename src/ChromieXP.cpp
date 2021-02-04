/*
 * Copyright (C) 2021+ ChromieCraft <www.chromiecraft.com>
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "Common.h"
#include "Config.h"
#include "ScriptMgr.h"
#include "World.h"

#define GOSSIP_TEXT_EXP         14736
#define GOSSIP_XP_OFF           "I no longer wish to gain experience."
#define GOSSIP_XP_ON            "I wish to start gaining experience again."

#define CHROMIE_CONF_STABLE_MAX_PLAYER_LEVEL "Chromie.Stable.MaxPlayerLevel"

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
                if (this->canUnlockExp(player))
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

private:
    bool canUnlockExp(Player* player)
    {
        if (player->getLevel() < sConfigMgr->GetIntDefault(CHROMIE_CONF_STABLE_MAX_PLAYER_LEVEL, 19))
        {
            return true;
        }

        auto result = CharacterDatabase.PQuery(
                "SELECT `guid` FROM `chromie_beta_testers` WHERE `isBetaTester` = 1 AND `guid` = %u",
                player->GetGUID()
        );

        if (!result)
        {
            return false;
        }

        return result->GetRowCount() > 0;
    }
};

class AutoLockExp: public PlayerScript
{
public:
    AutoLockExp() : PlayerScript("auto_lock_exp") {}

    void OnLevelChanged(Player* player, uint8 oldlevel) override
    {
        if (oldlevel == sConfigMgr->GetIntDefault(CHROMIE_CONF_STABLE_MAX_PLAYER_LEVEL, 19) - 1)
        {
            player->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        }
    }

    bool ShouldBeRewardedWithMoneyInsteadOfExp(Player* player) override
    {
        if (player->getLevel() == sConfigMgr->GetIntDefault(CHROMIE_CONF_STABLE_MAX_PLAYER_LEVEL, 19)
            && player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN))
        {
            return true;
        }

        return false;
    }
};

void AddChromieXpScripts() {
    new NpcExperienceChromie();
    new AutoLockExp();
}

