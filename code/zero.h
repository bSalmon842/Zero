/*
Project: Arsene's Zero
File: zero.h
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

#ifndef ZERO_H

enum iRacingSession
{
    IR_SESSION_PRAC_QUAL,
    IR_SESSION_RACE,
};

enum TPosType
{
    TPOS_PIT,
    TPOS_OOW,
    TPOS_ONTRACK,
};

enum GraphType
{
    GRAPH_RUNNING,
    GRAPH_BEST,
};

struct LapGraphInfo
{
    s32 lap;
    f32 time;
};

struct GearGraphInfo
{
    s32 gear;
    f32 lapDistPct;
};

struct SteerGraphInfo
{
    f32 angle;
    f32 lapDistPct;
};

struct SectorInfo
{
    s32 sector;
    f32 sectorStartPct;
};

struct SectorTimeInfo
{
    SectorInfo info;
    f32 sectorTime;
    f32 lapTimeAtPoint;
};

struct LiveLapGraphInfo
{
    f32 lapTime;
    f32 lapDistPct;
};

struct VisualColours
{
     ImVec4 back;
     ImVec4 fore;
};

struct DriverInfo
{
    b32 isSlotFilled;
    
    s32 carIdx;
    
    union SortData
    {
        s32 timedPos;
        f32 currDist;
    } sortData;
    
    s32 pos;
    char *number;
    char *name;
    char *teamName;
    
    f32 speed;
    
    s32 currLap;
    f32 bestLapTime;
    s32 bestLap;
    b32 hasFastestLap;
    b32 wasFastestLastLap;
    b32 outlap;
    
    f32 lastLapTime;
    f32 gap;
    
    b32 carInWorld;
    b32 inPits;
    f32 lapDistPct;
    
    s32 tPos;
    TPosType tPosType;
    
    ImVector<LapGraphInfo> lapTimeList;
    ImVector<LiveLapGraphInfo> runningLapList;
    
    ImVector<GearGraphInfo> bestLapGearList;
    ImVector<GearGraphInfo> interGearList;
    ImVector<GearGraphInfo> runningGearList;
    
    ImVector<SteerGraphInfo> bestLapSteerList;
    ImVector<SteerGraphInfo> interSteerList;
    ImVector<SteerGraphInfo> runningSteerList;
    
    ImVector<SectorTimeInfo> currSectorList;
    ImVector<SectorTimeInfo> lastLapSectorList;
    ImVector<SectorTimeInfo> bestLapSectorList;
    
    SectorInfo currSectorInfo;
    f32 currLapTime;
    
    f32 pitTimer;
    b32 prePitBox;
};

struct iRacingState
{
    b32 isRunning;
    b32 hasReceivedInitialData;
    
    char trackName[64];
    b32 isTrackmapLoaded;
    FiledTrack trackData;
    
    s32 currSessionNum;
    iRacingSession currSession;
    
    s32 sessionLaps;
    s32 sessionLapsRemaining;
    f64 sessionTimeRemaining;
    b32 timedRace;
    
    s32 *posArray;
    s32 *lapArray;
    f32 *lapDistArray;
    bool *inPitsArray;
    s32 *trkSrfArray;
    f32 *estTimeArray;
    s32 *gearsArray;
    f32 *steerArray;
    
    f32 maxEstTime;
    
    union GapData
    {
        f32 leaderFastestLap;
        f32 leaderCurrDist;
    } gapData;
    s32 leaderCarIdx;
    s32 fastestLapCarIdx;
    
    s32 driverCount;
    DriverInfo drivers[64];
    VisualColours colours[64];
    
    //b32 raceEnding;
    
    f32 ambientTemp;
    f32 trackTemp;
    f32 humidity;
    f32 windSpeed;
    f32 windDir;
    f32 timeOfDay;
    
    s32 sectorCount;
    SectorInfo *sectors;
    
    ImVector<SectorTimeInfo> bestLapSectorList;
    
    bool showPitLossOnTrackmap;
    f32 pitLossLapDistPct;
    
    b32 isTeamSession;
};

function void InitialiseDriverColours(iRacingState *iRacing)
{
    // Red (f: white, black)
    iRacing->colours[0].back = ImVec4(1, 0, 0, 1);
    iRacing->colours[0].fore = ImVec4(1, 1, 1, 1);
    iRacing->colours[1].back = ImVec4(1, 0, 0, 1);
    iRacing->colours[1].fore = ImVec4(0, 0, 0, 1);
    
    // Green (f: white, black)
    iRacing->colours[2].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[2].fore = ImVec4(1, 1, 1, 1);
    iRacing->colours[3].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[3].fore = ImVec4(0, 0, 0, 1);
    
    // Blue (f: white, black)
    iRacing->colours[4].back = ImVec4(0, 0, 1, 1);
    iRacing->colours[4].fore = ImVec4(1, 1, 1, 1);
    
    // Yellow (f: white, black)
    iRacing->colours[7].back = ImVec4(1, 1, 0, 1);
    iRacing->colours[7].fore = ImVec4(0, 0, 0, 1);
    
    // Magenta (f: white, black)
    iRacing->colours[8].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[8].fore = ImVec4(1, 1, 1, 1);
    iRacing->colours[9].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[9].fore = ImVec4(0, 0, 0, 1);
    
    // Aqua (f: white, black)
    iRacing->colours[10].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[10].fore = ImVec4(1, 1, 1, 1);
    iRacing->colours[11].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[11].fore = ImVec4(0, 0, 0, 1);
    
    // Monochrome (f: opposite)
    iRacing->colours[12].back = ImVec4(1, 1, 1, 1);
    iRacing->colours[12].fore = ImVec4(0, 0, 0, 1);
    iRacing->colours[13].back = ImVec4(0, 0, 0, 1);
    iRacing->colours[13].fore = ImVec4(1, 1, 1, 1);
    
    // Orange (f: white, black)
    iRacing->colours[14].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[14].fore = ImVec4(1, 1, 1, 1);
    iRacing->colours[15].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[15].fore = ImVec4(0, 0, 0, 1);
    
    // Red (f: green, blue, yellow, magenta, aqua, orange)
    iRacing->colours[16].back = ImVec4(1, 0, 0, 1);
    iRacing->colours[16].fore = ImVec4(0, 1, 0, 1);
    iRacing->colours[17].back = ImVec4(1, 0, 0, 1);
    iRacing->colours[17].fore = ImVec4(0, 0, 1, 1);
    iRacing->colours[18].back = ImVec4(1, 0, 0, 1);
    iRacing->colours[18].fore = ImVec4(1, 1, 0, 1);
    iRacing->colours[20].back = ImVec4(1, 0, 0, 1);
    iRacing->colours[20].fore = ImVec4(0, 1, 1, 1);
    iRacing->colours[21].back = ImVec4(1, 0, 0, 1);
    iRacing->colours[21].fore = ImVec4(1, 0.5f, 0, 1);
    
    // Green (f: red, blue, yellow, magenta, aqua, orange)
    iRacing->colours[22].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[22].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[23].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[23].fore = ImVec4(0, 0, 1, 1);
    iRacing->colours[24].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[24].fore = ImVec4(1, 1, 0, 1);
    iRacing->colours[25].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[25].fore = ImVec4(1, 0, 1, 1);
    iRacing->colours[26].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[26].fore = ImVec4(0, 1, 1, 1);
    iRacing->colours[27].back = ImVec4(0, 1, 0, 1);
    iRacing->colours[27].fore = ImVec4(1, 0.5f, 0, 1);
    
    // Blue (f: red, green, yellow, magenta, aqua, orange)
    iRacing->colours[28].back = ImVec4(0, 0, 1, 1);
    iRacing->colours[28].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[29].back = ImVec4(0, 0, 1, 1);
    iRacing->colours[29].fore = ImVec4(0, 1, 0, 1);
    iRacing->colours[30].back = ImVec4(0, 0, 1, 1);
    iRacing->colours[30].fore = ImVec4(1, 1, 0, 1);
    iRacing->colours[31].back = ImVec4(0, 0, 1, 1);
    iRacing->colours[31].fore = ImVec4(1, 0, 1, 1);
    iRacing->colours[32].back = ImVec4(0, 0, 1, 1);
    iRacing->colours[32].fore = ImVec4(0, 1, 1, 1);
    iRacing->colours[33].back = ImVec4(0, 0, 1, 1);
    iRacing->colours[33].fore = ImVec4(1, 0.5f, 0, 1);
    
    // Yellow (f: red, green, blue, magenta, aqua, orange)
    iRacing->colours[34].back = ImVec4(1, 1, 0, 1);
    iRacing->colours[34].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[35].back = ImVec4(1, 1, 0, 1);
    iRacing->colours[35].fore = ImVec4(0, 1, 0, 1);
    iRacing->colours[36].back = ImVec4(1, 1, 0, 1);
    iRacing->colours[36].fore = ImVec4(0, 0, 1, 1);
    iRacing->colours[37].back = ImVec4(1, 1, 0, 1);
    iRacing->colours[37].fore = ImVec4(1, 0, 1, 1);
    iRacing->colours[38].back = ImVec4(1, 1, 0, 1);
    iRacing->colours[38].fore = ImVec4(0, 1, 1, 1);
    iRacing->colours[39].back = ImVec4(1, 1, 0, 1);
    iRacing->colours[39].fore = ImVec4(1, 0.5f, 0, 1);
    
    // Magenta (f: red, green, blue, yellow, aqua, orange)
    iRacing->colours[40].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[40].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[41].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[41].fore = ImVec4(0, 1, 0, 1);
    iRacing->colours[42].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[42].fore = ImVec4(0, 0, 1, 1);
    iRacing->colours[43].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[43].fore = ImVec4(1, 1, 0, 1);
    iRacing->colours[44].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[44].fore = ImVec4(0, 1, 1, 1);
    iRacing->colours[45].back = ImVec4(1, 0, 1, 1);
    iRacing->colours[45].fore = ImVec4(1, 0.5f, 0, 1);
    
    // Aqua (f: red, green, blue, yellow, magenta, orange)
    iRacing->colours[46].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[46].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[47].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[47].fore = ImVec4(0, 1, 0, 1);
    iRacing->colours[48].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[48].fore = ImVec4(0, 0, 1, 1);
    iRacing->colours[49].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[49].fore = ImVec4(1, 1, 0, 1);
    iRacing->colours[50].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[50].fore = ImVec4(1, 0, 1, 1);
    iRacing->colours[51].back = ImVec4(0, 1, 1, 1);
    iRacing->colours[51].fore = ImVec4(1, 0.5f, 0, 1);
    
    // Orange (f: red, green, blue, yellow, magenta, aqua)
    iRacing->colours[52].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[52].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[53].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[53].fore = ImVec4(0, 1, 0, 1);
    iRacing->colours[54].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[54].fore = ImVec4(0, 0, 1, 1);
    iRacing->colours[55].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[55].fore = ImVec4(1, 1, 0, 1);
    iRacing->colours[56].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[56].fore = ImVec4(1, 0, 1, 1);
    iRacing->colours[57].back = ImVec4(1, 0.5f, 0, 1);
    iRacing->colours[57].fore = ImVec4(0, 1, 1, 1);
    
    // Black (f: red, green, blue, yellow, aqua, orange)
    iRacing->colours[58].back = ImVec4(0, 0, 0, 1);
    iRacing->colours[58].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[59].back = ImVec4(0, 0, 0, 1);
    iRacing->colours[59].fore = ImVec4(0, 1, 0, 1);
    iRacing->colours[60].back = ImVec4(0, 0, 0, 1);
    iRacing->colours[60].fore = ImVec4(0, 0, 1, 1);
    iRacing->colours[61].back = ImVec4(0, 0, 0, 1);
    iRacing->colours[61].fore = ImVec4(1, 1, 0, 1);
    iRacing->colours[62].back = ImVec4(0, 0, 0, 1);
    iRacing->colours[62].fore = ImVec4(0, 1, 1, 1);
    iRacing->colours[63].back = ImVec4(0, 0, 0, 1);
    iRacing->colours[63].fore = ImVec4(1, 0.5f, 0, 1);
    
    // White (f: red, dark green, blue) (used to replace bad colour combos)
    iRacing->colours[5].back = ImVec4(1, 1, 1, 1);
    iRacing->colours[5].fore = ImVec4(1, 0, 0, 1);
    iRacing->colours[6].back = ImVec4(1, 1, 1, 1);
    iRacing->colours[6].fore = ImVec4(0, 0.5f, 0, 1);
    iRacing->colours[19].back = ImVec4(1, 1, 1, 1);
    iRacing->colours[19].fore = ImVec4(0, 0, 1, 1);
}

#define ZERO_H
#endif //ZERO_H
