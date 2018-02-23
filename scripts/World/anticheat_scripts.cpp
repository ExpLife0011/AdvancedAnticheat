/***
 *  @project: Firestorm Freelance
 *  @author: Meltie2013
 *  @copyright: 2017 - 2018
 */

#include "AdvancedAnticheatMgr.h"

#include "DatabaseEnv.h"
#include "Log.h"

class advanced_anticheat_world : public WorldScript
{
public:
    advanced_anticheat_world() : WorldScript("advanced_anticheat_world") { }

    void OnStartup() override
    {
        TC_LOG_INFO("server.loading", "Deleting player reports...");

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        trans->Append("truncate table players_reports_status");
        trans->Append("delete dpr from daily_players_reports dpr left join characters c on c.guid = dpr.guid where c.guid is null");

        CharacterDatabase.DirectCommitTransaction(trans);
    }

    void OnConfigLoad(bool /* isReload */) override
    {
        sAdvancedAnticheatMgr->LoadSettings();
    }
};

void AddSC_Advanced_Anticheat()
{
    new advanced_anticheat_world();
}
