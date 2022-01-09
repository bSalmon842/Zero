/*
Project: Arsene's Zero
File: zero_maintiming.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

#define PI 3.1415926535f

inline f32 ConvertDegToRad(f32 deg)
{
    f32 result;
    
    result = deg * (PI / 180.0f);
    
    return result;
}

inline char GetWindDirLetterFirst(f32 windDir)
{
    char result = ' ';
    
    if (windDir >= ConvertDegToRad(315.0f) || windDir <= ConvertDegToRad(45.0f))
    {
        result = 'N';
    }
    else if (windDir > ConvertDegToRad(45.0f) && windDir < ConvertDegToRad(135.0f))
    {
        result = 'E';
    }
    else if (windDir >= ConvertDegToRad(135.0f) && windDir <= ConvertDegToRad(225.0f))
    {
        result = 'S';
    }
    else if (windDir > ConvertDegToRad(225.0f) && windDir < ConvertDegToRad(315.0f))
    {
        result = 'W';
    }
    
    return result;
}

inline char GetWindDirLetterSecond(f32 windDir)
{
    char result = ' ';
    
    if ((GetWindDirLetterFirst(windDir) != 'E') && (windDir > ConvertDegToRad(22.5f) && windDir < ConvertDegToRad(157.5f)))
    {
        result = 'E';
    }
    else if ((GetWindDirLetterFirst(windDir) != 'W') && (windDir > ConvertDegToRad(202.5f) && windDir < ConvertDegToRad(337.5f)))
    {
        result = 'W';
    }
    
    return result;
}

inline s32 GetTimeOfDayHour(f32 timeOfDay)
{
    s32 result = (s32)(timeOfDay / 60 / 60);
    if (result > 12)
    {
        result -= 12;
    }
    return result;
}

inline s32 GetTimeOfDayMinute(f32 timeOfDay)
{
    s32 result = 0;
    s32 sTime = (s32)(timeOfDay / 60 / 60);
    result = (s32)(((timeOfDay / 60 / 60) - sTime) * 60);
    return result;
}

inline char GetTimeOfDayMeridiem(f32 timeOfDay)
{
    char result = 'A';
    if (timeOfDay >= 43200.0f)
    {
        result = 'P';
    }
    return result;
}

inline void WeatherInfo(char *headString, u32 colour, char *infoFmt, ...)
{
    ImGui::PushStyleColor(ImGuiCol_Text, colour);
    ImGui::Text(headString);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    
    char buf[128];
    va_list args;
    va_start(args, infoFmt);
    vsprintf(buf, infoFmt, args);
    ImGui::Text(buf);
    va_end(args);
}

function void SortDrivers(iRacingState *iRacing, DriverInfo *sortedDrivers)
{
    if (iRacing->currSession == IR_SESSION_PRAC_QUAL)
    {
        b32 sorting = true;
        while (sorting)
        {
            sorting = false;
            for (s32 driverIndex = 0; driverIndex < (iRacing->driverCount - 1); ++driverIndex)
            {
                DriverInfo *driverA = &sortedDrivers[driverIndex];
                DriverInfo *driverB = &sortedDrivers[driverIndex + 1];
                if (driverB->sortData.timedPos != 0 && (driverA->sortData.timedPos > driverB->sortData.timedPos || driverA->sortData.timedPos == 0))
                {
                    DriverInfo temp = *driverA;
                    *driverA = *driverB;
                    *driverB = temp;
                    sorting = true;
                }
                
                driverA->pos = driverA->sortData.timedPos;
                driverB->pos = driverB->sortData.timedPos;
            }
        }
    }
    else
    {
        f32 currFastestLastLap = 10000.0f;
        s32 currFastestLastLapIndex = -1;
        b32 sorting = true;
        while (sorting)
        {
            sorting = false;
            for (s32 driverIndex = 0; driverIndex < (iRacing->driverCount - 1); ++driverIndex)
            {
                DriverInfo *driverA = &sortedDrivers[driverIndex];
                DriverInfo *driverB = &sortedDrivers[driverIndex + 1];
                if (driverA->sortData.currDist < driverB->sortData.currDist)
                {
                    DriverInfo temp = *driverA;
                    *driverA = *driverB;
                    *driverB = temp;
                    sorting = true;
                }
            }
        }
        
            for (s32 driverIndex = 0; driverIndex < iRacing->driverCount; ++driverIndex)
            {
                sortedDrivers[driverIndex].wasFastestLastLap = false;
                sortedDrivers[driverIndex].pos = driverIndex + 1;
                if (sortedDrivers[driverIndex].lastLapTime < currFastestLastLap)
                {
                    currFastestLastLap = sortedDrivers[driverIndex].lastLapTime;
                    currFastestLastLapIndex = driverIndex;
                }
            }
            
            sortedDrivers[currFastestLastLapIndex].wasFastestLastLap = true;
    }
}

function void MainTiming(iRacingState *iRacing, s32 columnCount)
{
    DriverInfo sortedDrivers[64] = {};
    for (s32 i = 0; i < ARRAY_COUNT(iRacing->drivers); ++i)
    {
        sortedDrivers[i] = iRacing->drivers[i];
    }
    SortDrivers(iRacing, sortedDrivers);
    
    for (s32 rowIndex = 0; rowIndex < iRacing->driverCount; ++rowIndex)
    {
        if (strcmp(sortedDrivers[rowIndex].name, "Pace Car") != 0)
        {
            ImGui::TableNextRow();
            for (s32 columnIndex = 0; columnIndex < columnCount; ++columnIndex)
            {
                ImGui::TableSetColumnIndex(columnIndex);
                
                DriverInfo info = sortedDrivers[rowIndex];
                if (columnIndex < 8)
                {
                switch (columnIndex)
                {
                    case 0:
                    {
                            ImGui::Text("%d", info.pos);
                    }break;
                    
                    case 1:
                    {
                        info.number = &info.number[1];
                        for (s32 charIndex = 0; charIndex < StringLength(info.number); ++charIndex)
                        {
                            if (info.number[charIndex] == '"')
                            {
                                info.number[charIndex] = ' ';
                            }
                        }
                        
                        ImGui::Text(info.number);
                    }break;
                    
                    case 2:
                        {
                            ImU32 backColour = ImGui::ColorConvertFloat4ToU32(iRacing->colours[info.carIdx].back);
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backColour);
                            
                            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(iRacing->colours[info.carIdx].fore));
                            char str[128] = {};
                            if (iRacing->isTeamSession)
                            {
                                stbsp_sprintf(str, "%s (%s)", info.teamName, info.name);
                            }
                            else
                            {
                                stbsp_sprintf(str, "%s", info.name);
                            }
                            ImGui::Text(str);
                            ImGui::PopStyleColor();
                    }break;
                    
                    case 3:
                    {
                        char str[4] = {};
                        stbsp_sprintf(str, "%d", info.currLap);
                        
                        if (info.inPits)
                        {
                            ImGui::Text(str);
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF00AAAA);
                        }
                        else if (info.outlap && iRacing->currSession == IR_SESSION_PRAC_QUAL)
                        {
                            ImGui::Text(str);
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF825700);
                        }
                        else
                        {
                            ImGui::Text(str);
                        }
                    }break;
                    
                    case 4:
                    {
                        switch (info.tPosType)
                        {
                            case TPOS_PIT:
                                {
                                    if (iRacing->currSession != IR_SESSION_PRAC_QUAL && info.pitTimer < 300.0f)
                                    {
                                        ImGui::Text("PIT");
                                        ImGui::SameLine();
                                        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF00AAAA);
                                        ImGui::Text("(%.02f)", info.pitTimer);
                                        ImGui::PopStyleColor();
                                    }
                                    else
                                    {
                                        ImGui::Text("PIT");
                                    }
                            }break;
                            
                            case TPOS_OOW:
                            {
                                ImGui::Text("OUT");
                            }break;
                            
                            case TPOS_ONTRACK:
                                {
                                    if (info.pitTimer > 0.0f)
                                    {
                                        ImGui::Text("S%d", info.tPos);
                                        ImGui::SameLine();
                                        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF00AAAA);
                                        ImGui::Text("(%.02f)", info.pitTimer);
                                        ImGui::PopStyleColor();
                                    }
                                    else
                                    {
                                ImGui::Text("S%d", info.tPos);
                                    }
                            }break;
                            
                            default:
                            {
                                INVALID_CODE_PATH;
                            }break;
                        }
                    }break;
                        
                    case 5:
                    {
                        char time[32];
                        FormatLapTime(info.bestLapTime, time);
                        stbsp_sprintf(&time[9], " (%d)\0", info.bestLap);
                        
                        if (info.hasFastestLap)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, 0xFFBE46FF);
                            ImGui::Text(time);
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            ImGui::Text(time);
                        }
                    }break;
                    
                    case 6:
                    {
                        char time[16];
                        FormatLapTime(info.lastLapTime, time);
                        
                        if (info.lastLapTime == iRacing->gapData.leaderFastestLap)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, 0xFFBE46FF);
                            ImGui::Text(time);
                            ImGui::PopStyleColor();
                        }
                        else
                        {
                            ImGui::Text(time);
                        }
                    }break;
                    
                    case 7:
                    {
                        char time[16] = {};
                        
                        if (iRacing->currSession == IR_SESSION_PRAC_QUAL)
                        {
                            if (!info.hasFastestLap)
                            {
                                stbsp_sprintf(time, "+");
                                FormatLapTime(info.gap, &time[1]);
                            }
                        }
                        else
                        {
                            if (!info.pos != 1)
                            {
                                if (info.gap < 0.0f)
                                {
                                    s32 lapsDown = Abs(RoundF32ToS32(info.gap));
                                    if (lapsDown == 1)
                                    {
                                        stbsp_sprintf(time, "+1 Lap");
                                    }
                                        else if (lapsDown > 1)
                                    {
                                        stbsp_sprintf(time, "+%d Laps", lapsDown);
                                    }
                                }
                                else
                                {
                                    stbsp_sprintf(time, "+");
                                    FormatLapTime(info.gap, &time[1]);
                                }
                            }
                        }
                        
                        ImGui::Text(time);
                    }break;
                    
                    default:
                    {
                        INVALID_CODE_PATH;
                    }break;
                    }
                }
                else
                {
                    for (s32 caseIndex = TIMING_COLUMN_COUNT_BASE; caseIndex < (TIMING_COLUMN_COUNT_BASE + iRacing->sectorCount); ++caseIndex)
                    {
                        if (caseIndex == columnIndex)
                        {
                            if (info.currSectorList.size() > 0 && info.bestLapSectorList.size() > 0 && iRacing->bestLapSectorList.size() > 0)
                            {
                                s32 sector = caseIndex - TIMING_COLUMN_COUNT_BASE;
                                if (sector < info.currSectorList.size())
                                {
                                    if (info.currSectorList[sector].lapTimeAtPoint < iRacing->bestLapSectorList[sector].lapTimeAtPoint)
                                    {
                                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFFBE46FF);
                                    }
                                    else if ((info.currSectorList[sector].lapTimeAtPoint >= iRacing->bestLapSectorList[sector].lapTimeAtPoint) &&
                                             (info.currSectorList[sector].lapTimeAtPoint < info.bestLapSectorList[sector].lapTimeAtPoint))
                                    {
                                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF00FF00);
                                    }
                                    else
                                    {
                                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, 0xFF0000FF);
                                    }
                                    
                                    char sectorDiff[16] = {};
                                    f32 delta = info.currSectorList[sector].lapTimeAtPoint - iRacing->bestLapSectorList[sector].lapTimeAtPoint;
                                    if (delta < 0.0f)
                                    {
                                        sectorDiff[0] = '-';
                                    }
                                    else
                                    {
                                        sectorDiff[0] = '+';
                                    }
                                    stbsp_sprintf(&sectorDiff[1], "%.03f", Abs(delta));
                                    
                                    ImGui::PushStyleColor(ImGuiCol_Text, 0xFF000000);
                                    ImGui::Text(sectorDiff);
                                    ImGui::PopStyleColor();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

