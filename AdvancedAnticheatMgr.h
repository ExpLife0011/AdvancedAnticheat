/***
 *  @project: Firestorm Freelance
 *  @author: Meltie2013
 *  @copyright: 2017 - 2018
 */

#ifndef ADVANCEDANTICHEATHMGR_H
#define ADVANCEDANTICHEATHMGR_H

#include "AdvancedAnticheatData.h"

#include "Common.h"
#include "SharedDefines.h"
#include "ScriptMgr.h"
#include "Player.h"

class ChatHandler;
class AnticheatCheck;

enum DetectionTypes
{
    SPEED_HACK_DETECTION            = 0x01,
    FLY_HACK_DETECTION              = 0x02,
    WALK_WATER_HACK_DETECTION       = 0x04,
    JUMP_HACK_DETECTION             = 0x08,
    TELEPORT_PLANE_HACK_DETECTION   = 0x10,
    CLIMB_HACK_DETECTION            = 0x20
};

typedef std::unordered_map<ObjectGuid, AdvancedAnticheatData> AnticheatPlayerDataMap;

class AdvancedAnticheatMgr
{
    friend class AnticheatCheck;

    AdvancedAnticheatMgr();
    ~AdvancedAnticheatMgr();

public:
    static AdvancedAnticheatMgr* Instance();

    void StartHackDetection(Player* player, MovementInfo const& movementInfo, uint16 opcode);

    void PlayerLogin(Player* player, PreparedQueryResult result);
    void PlayerLogout(ObjectGuid const& guid);
    void SavePlayerData(ObjectGuid const& guid, SQLTransaction& trans);

    inline uint32 GetTotalReports(ObjectGuid const& guid) { return _playerMap[guid].GetTotalReports(); }
    inline float GetAverage(ObjectGuid const& guid) { return _playerMap[guid].GetAverage(); }
    inline uint32 GetTypeReports(ObjectGuid const& guid, ReportTypes type) { return _playerMap[guid].GetTypeReports(type); }

    bool AnticheatGlobalCommand(ChatHandler* handler);

    void ResetDailyReportStates();

    void LoadSettings();

private:
    AnticheatPlayerDataMap _playerMap;
    std::vector<AnticheatCheck*> _checks;

    bool _enabled;
};

class AnticheatCheck
{
public:
    virtual ~AnticheatCheck() { }
    virtual bool OnCheck(Player* player, AdvancedAnticheatData* playerData, MovementInfo const& movementInfo, uint16 opcode = 0) const = 0;

    virtual void HackReport(Player* player, AdvancedAnticheatData* playerData) const = 0;
};

template <ReportTypes type>
class AnticheatCheckBase : public AnticheatCheck
{
public:
    bool OnCheck(Player* /*player*/, AdvancedAnticheatData* /*playerData*/, MovementInfo const& /*movementInfo*/, uint16 /*opcode = 0*/) const override
    {
        ASSERT(false && "AnticheatCheckBase::OnCheck called directly");
        return false;
    }

    void HackReport(Player* player, AdvancedAnticheatData* playerData) const final override;
};

#define sAdvancedAnticheatMgr AdvancedAnticheatMgr::Instance()

#endif
