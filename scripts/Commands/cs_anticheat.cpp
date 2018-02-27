/***
 *  @project: Firestorm Freelance
 *  @author: Meltie2013
 *  @copyright: 2017 - 2018
 */

#include "AdvancedAnticheatMgr.h"

#include "Chat.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "RBAC.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "World.h"

class anticheat_commandscript : public CommandScript
{
public:
    anticheat_commandscript() : CommandScript("anticheat_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> anticheatCommandTable =
        {
            { "global", rbac::RBAC_PERM_COMMAND_ANTICHEAT_GLOBAL, true,  &HandleAntiCheatGlobalCommand, "" },
            { "player", rbac::RBAC_PERM_COMMAND_ANTICHEAT_PLAYER, true,  &HandleAntiCheatPlayerCommand, "" },
            { "handle", rbac::RBAC_PERM_COMMAND_ANTICHEAT_HANDLE, true,  &HandleAntiCheatHandleCommand, "" },
            { "warn",   rbac::RBAC_PERM_COMMAND_ANTICHEAT_WARN,   true,  &HandleAnticheatWarnCommand,   "" }
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "anticheat", rbac::RBAC_PERM_COMMAND_ANTICHEAT, true, nullptr, "", anticheatCommandTable }
        };
        return commandTable;
    }

    static bool HandleAnticheatWarnCommand(ChatHandler* handler, char const* args)
    {
        if (!sWorld->getBoolConfig(CONFIG_ADVANCED_ANTICHEAT_ENABLE))
        {
            handler->SendSysMessage(LANG_COMMAND_ACDISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* pTarget = nullptr;
        char* command = strtok(const_cast<char*>(args), " ");

        if (command)
        {
            std::string strCommand(command);
            normalizePlayerName(strCommand);

            pTarget = ObjectAccessor::FindPlayerByName(strCommand); //get player by name
        }
        else
            pTarget = handler->getSelectedPlayer();

        if (!pTarget)
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ChatHandler(pTarget->GetSession()).SendSysMessage(LANG_COMMAND_ACWARN);
        return true;
    }

    static bool HandleAntiCheatPlayerCommand(ChatHandler* handler, char const* args)
    {
        if (!sWorld->getBoolConfig(CONFIG_ADVANCED_ANTICHEAT_ENABLE))
        {
            handler->SendSysMessage(LANG_COMMAND_ACDISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = nullptr;
        ObjectGuid playerGuid;
        handler->extractPlayerTarget(const_cast<char*>(args), &player, &playerGuid);

        if (playerGuid.IsEmpty())
        {
            handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        float average = sAdvancedAnticheatMgr->GetAverage(playerGuid);
        uint32 total_reports = sAdvancedAnticheatMgr->GetTotalReports(playerGuid);
        uint32 speed_reports = sAdvancedAnticheatMgr->GetTypeReports(playerGuid, REPORT_PLAYER_TYPE_SPEED);
        uint32 fly_reports = sAdvancedAnticheatMgr->GetTypeReports(playerGuid, REPORT_PLAYER_TYPE_FLY);
        uint32 jump_reports = sAdvancedAnticheatMgr->GetTypeReports(playerGuid, REPORT_PLAYER_TYPE_JUMP);
        uint32 waterwalk_reports = sAdvancedAnticheatMgr->GetTypeReports(playerGuid, REPORT_PLAYER_TYPE_WATERWALK);
        uint32 teleportplane_reports = sAdvancedAnticheatMgr->GetTypeReports(playerGuid, REPORT_PLAYER_TYPE_TELEPORT_PLANE);
        uint32 climb_reports = sAdvancedAnticheatMgr->GetTypeReports(playerGuid, REPORT_PLAYER_TYPE_CLIMB);

        handler->PSendSysMessage(LANG_COMMAND_ACPLAYER_INFO, player->GetName().c_str());
        handler->PSendSysMessage(LANG_COMMAND_ACPLAYER_STATS, average, total_reports);
        handler->PSendSysMessage(LANG_COMMAND_ACPLAYER_SPEEDFLYJUMP, speed_reports, fly_reports, jump_reports);
        handler->PSendSysMessage(LANG_COMMAND_ACPLAYER_WATERPLANE, waterwalk_reports, teleportplane_reports);
        handler->PSendSysMessage(LANG_COMMAND_ACPLAYER_CLIMB, climb_reports);
        return true;
    }

    static bool HandleAntiCheatHandleCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        std::string strCommand(args);

        if (strCommand == "on")
        {
            sWorld->setBoolConfig(CONFIG_ADVANCED_ANTICHEAT_ENABLE, true);
            handler->SendSysMessage(LANG_COMMAND_ACHANDLEENABLED);
            sAdvancedAnticheatMgr->LoadSettings();
            return true;
        }
        else if (strCommand == "off")
        {
            sWorld->setBoolConfig(CONFIG_ADVANCED_ANTICHEAT_ENABLE, false);
            handler->SendSysMessage(LANG_COMMAND_ACHANDLEDISABLED);
            sAdvancedAnticheatMgr->LoadSettings();
            return true;
        }

        handler->SendSysMessage(LANG_USE_BOL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleAntiCheatGlobalCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (!sWorld->getBoolConfig(CONFIG_ADVANCED_ANTICHEAT_ENABLE))
        {
            handler->SendSysMessage(LANG_COMMAND_ACDISABLED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        return sAdvancedAnticheatMgr->AnticheatGlobalCommand(handler);
    }
};

void AddSC_anticheat_commandscript()
{
    new anticheat_commandscript();
}
