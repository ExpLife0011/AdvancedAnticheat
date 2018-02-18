/***
 *  @project: Firestorm Freelance
 *  @author: Meltie2013
 *  @copyright: 2017 - 2018
 */

#ifndef ADVANCEDANTICHEATDATA_H
#define ADVANCEDANTICHEATDATA_H

#include "Common.h"

enum ReportTypes
{
    REPORT_PLAYER_TYPE_SPEED          = 0,
    REPORT_PLAYER_TYPE_FLY            = 1,
    REPORT_PLAYER_TYPE_WATERWALK      = 2,
    REPORT_PLAYER_TYPE_JUMP           = 3,
    REPORT_PLAYER_TYPE_TELEPORT_PLANE = 4,
    REPORT_PLAYER_TYPE_CLIMB          = 5,

    MAX_REPORT_PLAYER_TYPES
};

struct AdvancedAnticheatData
{
    AdvancedAnticheatData() : _lastOpcode(0), _totalReports(0), _average(0.f), _reportTime(0), _lastReportTime(0), _hasDailyReport(false)
    {
        for (uint8 i = 0; i < MAX_REPORT_PLAYER_TYPES; ++i)
        {
            _typeReports[i] = 0;
            _tempReports[i] = 0;
            _tempReportsTimer[i] = 0;
        }
    }

    inline void SetLastOpcode(uint16 opcode) { _lastOpcode = opcode; }
    inline uint16 GetLastOpcode() const { return _lastOpcode; }

    inline void SetTotalReports(uint32 value)  { _totalReports = value; }
    inline uint32 GetTotalReports() const { return _totalReports; }

    inline void SetTypeReports(ReportTypes type, uint32 amount) { if (type >= MAX_REPORT_PLAYER_TYPES) return; _typeReports[type] = amount; }
    inline uint32 GetTypeReports(ReportTypes type) const { return type < MAX_REPORT_PLAYER_TYPES ? _typeReports[type] : 0; }

    inline void SetAverage(float value) { _average = value; }
    inline float GetAverage() const { return _average; }

    inline void SetReportTime(uint32 value) { _reportTime = value; }
    inline uint32 GetReportTime() { return _reportTime; }

    inline void SetTempReports(uint32 amount, ReportTypes type) { if (type >= MAX_REPORT_PLAYER_TYPES) return; _tempReports[type] = amount; }
    inline uint32 GetTempReports(ReportTypes type) const { return type < MAX_REPORT_PLAYER_TYPES ? _tempReports[type] : 0; }

    inline void SetTempReportsTimer(uint32 time, ReportTypes type) { if (type >= MAX_REPORT_PLAYER_TYPES) return; _tempReportsTimer[type] = time; }
    inline uint32 GetTempReportsTimer(ReportTypes type) const { return type < MAX_REPORT_PLAYER_TYPES ? _tempReportsTimer[type] : 0; }

    inline void SetLastReportTimer(uint32 time) { _lastReportTime = time; }
    inline uint32 GetLastReportTimer() const { return _lastReportTime; }

    inline void SetDailyReportState(bool report) { _hasDailyReport = report; }
    inline bool GetDailyReportState() const { return _hasDailyReport; }

private:
    uint16 _lastOpcode;
    uint32 _totalReports;
    uint32 _typeReports[MAX_REPORT_PLAYER_TYPES];
    float _average;
    uint32 _reportTime;
    uint32 _lastReportTime;
    uint32 _tempReports[MAX_REPORT_PLAYER_TYPES];
    uint32 _tempReportsTimer[MAX_REPORT_PLAYER_TYPES];
    bool _hasDailyReport;
};

#endif

