/***
 *  @project: Firestorm Freelance
 *  @author: Meltie2013
 *  @copyright: 2017 - 2018
 */

#include "AdvancedAnticheatMgr.h"

#include "Chat.h"
#include "DatabaseEnv.h"
#include "Language.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldSession.h"

// check climb angles
static float const CLIMB_ANGLE = 1.9f;

// prevents very large parameter
float tangent(float x);

AdvancedAnticheatMgr::AdvancedAnticheatMgr() : _enabled(false) { }

AdvancedAnticheatMgr::~AdvancedAnticheatMgr()
{
    for (AnticheatCheck* check : _checks)
        delete check;
}

AdvancedAnticheatMgr* AdvancedAnticheatMgr::Instance()
{
    static AdvancedAnticheatMgr Instance;
    return &Instance;
}

void AdvancedAnticheatMgr::LoadSettings()
{
    for (AnticheatCheck* check : _checks)
        delete check;

    _checks.clear();

    _enabled = sWorld->getBoolConfig(CONFIG_ADVANCED_ANTICHEAT_ENABLE);
    if (!_enabled)
        return;

    uint32 enabledCheckMask = sWorld->getIntConfig(CONFIG_ADVANCED_ANTICHEAT_DETECTIONS_ENABLED);
}

template <ReportTypes type>
void AnticheatCheckBase<type>::HackReport(Player* player, AdvancedAnticheatData* playerData) const
{
    bool sendReport = false;
    uint32 reportDelay = sWorld->getIntConfig(CONFIG_ADVANCED_ANTICHEAT_PLAYER_REPORT_DELAY);

    if (type != REPORT_PLAYER_TYPE_JUMP)
    {
        uint32 actualTime = getMSTime();

        if (!playerData->GetTempReportsTimer(type))
            playerData->SetTempReportsTimer(actualTime, type);

        if (!playerData->GetLastReportTimer())
            playerData->SetLastReportTimer(actualTime);

        sendReport = getMSTimeDiff(playerData->GetLastReportTimer(), actualTime) >= reportDelay;

        if (getMSTimeDiff(playerData->GetTempReportsTimer(type), actualTime) < 3000)
        {
            playerData->SetTempReports(playerData->GetTempReports(type) + 1, type);
            if (playerData->GetTempReports(type) < 3)
                return;
        }
        else
        {
            playerData->SetTempReportsTimer(actualTime, type);
            playerData->SetTempReports(1, type);
            return;
        }
    }

    if (!playerData->GetTotalReports())
        playerData->SetReportTime(getMSTime());

    playerData->SetTotalReports(playerData->GetTotalReports() + 1);
    playerData->SetTypeReports(type, playerData->GetTypeReports(type) + 1);

    uint32 diffTime = getMSTimeDiff(playerData->GetReportTime(), getMSTime()) / IN_MILLISECONDS;

    if (diffTime > 0)
    {
        float average = float(playerData->GetTotalReports()) / float(diffTime);
        playerData->SetAverage(average);
    }

    if (sWorld->getIntConfig(CONFIG_ADVANCED_ANTICHEAT_MAX_REPORTS_FOR_DAILY_REPORT) < playerData->GetTotalReports())
    {
        if (!playerData->GetDailyReportState())
        {
            SQLTransaction trans = CharacterDatabase.BeginTransaction();
            PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_AC_PLAYER_DAILY_REPORTS);
        
            stmt->setUInt32(0, player->GetGUID().GetCounter());
            trans->Append(stmt);

            uint8 index = 0;
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_AC_PLAYER_DAILY_REPORTS);
            stmt->setUInt32(index, player->GetGUID().GetCounter());
            stmt->setFloat(++index, playerData->GetAverage());
            stmt->setUInt32(++index, playerData->GetTotalReports());

            for (uint8 i = 0; i < MAX_REPORT_PLAYER_TYPES; ++i)
                stmt->setUInt32(++index, playerData->GetTypeReports(static_cast<ReportTypes>(i)));

            stmt->setUInt32(++index, playerData->GetReportTime());
            trans->Append(stmt);

            CharacterDatabase.CommitTransaction(trans);
            playerData->SetDailyReportState(true);
        }
    }

    if (sendReport && playerData->GetTotalReports() > sWorld->getIntConfig(CONFIG_ADVANCED_ANTICHEAT_REPORTS_INGAME_NOTIFICATION))
    {
        playerData->SetLastReportTimer(getMSTime());

        WorldPacket buffer;
        std::string strength = Trinity::StringFormat(sObjectMgr->GetTrinityStringForDBCLocale(LANG_AC_CHEAT_REPORT),
            player->GetName().c_str(), player->GetName().c_str());
        ChatHandler::BuildChatPacket(buffer, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, nullptr, nullptr, strength);
        sWorld->SendGlobalGMMessage(&buffer);
    }
}

void AdvancedAnticheatMgr::StartHackDetection(Player* player, MovementInfo const& movementInfo, uint16 opcode)
{
    if (!_enabled)
        return;

    if (player->CanBeGameMaster())
        return;

    AdvancedAnticheatData& plData = _playerMap[player->GetGUID()];
    if (player->IsInFlight() || player->GetTransport() || player->GetVehicle())
    {
        plData.SetLastOpcode(opcode);
        return;
    }

    for (AnticheatCheck* check : _checks)
        if (check->OnCheck(player, &plData, movementInfo, opcode))
            check->HackReport(player, &plData);

    plData.SetLastOpcode(opcode);
}

void AdvancedAnticheatMgr::PlayerLogin(Player* player, PreparedQueryResult result)
{
    AdvancedAnticheatData& pData = _playerMap[player->GetGUID()];

    if (result)
        pData.SetDailyReportState(true);
}

void AdvancedAnticheatMgr::PlayerLogout(ObjectGuid const& guid)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_AC_PLAYER_REPORTS);
    stmt->setUInt32(0, guid.GetCounter());
    CharacterDatabase.Execute(stmt);

    _playerMap.erase(guid);
}

void AdvancedAnticheatMgr::SavePlayerData(ObjectGuid const& guid, SQLTransaction& trans)
{
    AdvancedAnticheatData const& playerData = _playerMap[guid];
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_AC_PLAYER_REPORTS);
    stmt->setUInt32(0, guid.GetCounter());
    trans->Append(stmt);

    uint8 index = 0;
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_AC_PLAYER_REPORTS);
    stmt->setUInt32(index, guid.GetCounter());
    stmt->setFloat(++index, playerData.GetAverage());
    stmt->setUInt32(++index, playerData.GetTotalReports());

    for (uint8 i = 0; i < MAX_REPORT_PLAYER_TYPES; ++i)
        stmt->setUInt32(++index, playerData.GetTypeReports(static_cast<ReportTypes>(i)));

    //stmt->setUInt32(++index, playerData.GetReportTime());
    trans->Append(stmt);
}

bool AdvancedAnticheatMgr::AnticheatGlobalCommand(ChatHandler* handler)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    // Mutex protected
    {
        //boost::shared_lock<boost::shared_mutex> lock(*HashMapHolder<Player>::GetLock());

        HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
        for (auto const& pair : m)
            SavePlayerData(pair.second->GetGUID(), trans);
    }
    CharacterDatabase.DirectCommitTransaction(trans);

    QueryResult resultDB = CharacterDatabase.Query("select guid, average, total_reports from players_reports_status where total_reports > 0 order by average asc limit 5");
    if (!resultDB)
    {
        handler->SendSysMessage(LANG_NO_PLAYERS_FOUND);
        handler->SetSentErrorMessage(true);
        return false;
    }
    else
    {
        handler->SendSysMessage(LANG_COMMAND_ACGLOBAL_BAR);
        handler->SendSysMessage(LANG_COMMAND_ACGLOBAL_AVERAGE_INFO);
        do
        {
            Field* fields = resultDB->Fetch();

            ObjectGuid::LowType lowGuid = fields[0].GetUInt32();
            float average = fields[1].GetFloat();
            uint32 total_reports = fields[2].GetUInt32();

            if (Player* player = ObjectAccessor::FindPlayerByLowGUID(lowGuid))
                handler->PSendSysMessage(LANG_COMMAND_ACGLOBAL_REPORT_STATS, player->GetName().c_str(), total_reports, average);
        }
        while (resultDB->NextRow());
    }

    return true;
}

void AdvancedAnticheatMgr::ResetDailyReportStates()
{
    for (auto& plData : _playerMap)
        plData.second.SetDailyReportState(false);
}
