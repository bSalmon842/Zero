/*
Project: Arsene's Zero
File: zero.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved.
*/

#include "irsdk/irsdk_defines.h"
#include "irsdk/irsdk_utils.cpp"
#include "irsdk/yaml_parser.cpp"

#include "imgui/imgui.h"
#include "imgui/implot.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "glad/glad.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "zero_utility.h"
#include "zero_file.cpp"
#include "zero.h"

// TODO(bSalmon): Memory Leak (Maybe an issue of live graphs)
// TODO(bSalmon): Performance sucks

// TODO(bSalmon): Trackmap Data Viewer
// TODO(bSalmon): Fix Trackmapper rotating tracks 90 degrees
// TODO(bSalmon): Wind Direction Arrow on TM
// TODO(bSalmon): Save Trackmap settings (ideally remove the need for them
// TODO(bSalmon): Live lap time display
// TODO(bSalmon): Sector timing rework to fix inaccuracies
// TODO(bSalmon): Lock results at end of race
// TODO(bSalmon): Let pit stop trackmap vis work while MFD page not active
// TODO(bSalmon): Live lap delta graph fix
// TODO(bSalmon): Figure out speed calc with lapDistPct and how to get around iRacing being a pain

function void CreateDropdown(char *comboName, char **currItem, s32 *currItemIndex, char **itemList, s32 itemListSize)
{
    if (ImGui::BeginCombo(comboName, *currItem))
    {
        for (s32 i = 0; i < itemListSize; ++i)
        {
            b32 isSelected = (*currItem == itemList[i]);
            
            if (ImGui::Selectable(itemList[i], isSelected))
            {
                *currItem = itemList[i];
                *currItemIndex = i;
            }
            
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        
ImGui::EndCombo();
    }
}

function void CreateDropdown(char *comboName, f32 *currItem, s32 *currItemIndex, f32 *itemList, s32 itemListSize)
{
    char str[8];
    gcvt(*currItem, 4, str);
    
    if (ImGui::BeginCombo(comboName, str))
    {
        for (s32 i = 0; i < itemListSize; ++i)
        {
            char listStr[8];
            gcvt(itemList[i], 4, listStr);
            
            b32 isSelected = (*currItem == itemList[i]);
            
            if (ImGui::Selectable(listStr, isSelected))
            {
                *currItem = itemList[i];
                *currItemIndex = i;
            }
            
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        
        ImGui::EndCombo();
    }
}

struct LogItemInfo
{
    char str[128];
    u32 colour;
};

global ImVector<LogItemInfo> consoleBuffer;
function void LogToConsole(u32 colour, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
     LogItemInfo info = {};
    vsprintf(info.str, fmt, args);
    info.colour = colour;
    consoleBuffer.push_back(info);
    va_end(args);
}
function void LogToConsole(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogItemInfo info = {};
    vsprintf(info.str, fmt, args);
    info.colour = 0xFFFFFFFF;
    consoleBuffer.push_back(info);
    va_end(args);
}

#include "zero_trackmap.cpp"
#include "zero_calculator.cpp"
#include "zero_maintiming.cpp"
#include "zero_mfd_a.cpp"
#include "zero_mfd_b.cpp"

inline void *GetIRSDKVarLoc(s8 *data, char *var)
{
    void *result = data + irsdk_varNameToOffset(var);
    return result;
}
#define GetIRSDKVar(type, data, var) *(type *)GetIRSDKVarLoc(data, var)

inline b32 _GetS32FromYAML(s32 *result, char *sessionInfoString)
{
    const char *valueString;
    s32 valueLen;
    
    if (parseYaml(irsdk_getSessionInfoStr(), sessionInfoString, &valueString, &valueLen))
    {
        *result = atoi(valueString);
        return true;
    }
    else
    {
        return false;
    }
}

inline b32 _GetF32FromYAML(f32 *result, char *sessionInfoString)
{
    const char *valueString;
    s32 valueLen;
    
    if (parseYaml(irsdk_getSessionInfoStr(), sessionInfoString, &valueString, &valueLen))
    {
        *result = (f32)atof(valueString);
        return true;
    }
    else
    {
        return false;
    }
}

inline b32 _GetStringFromYAML(char *result, char *sessionInfoString)
{
    const char *valueString;
    s32 valueLen;
    
    if (parseYaml(irsdk_getSessionInfoStr(), sessionInfoString, &valueString, &valueLen))
    {
        for (s32 i = 0; i < valueLen; ++i)
        {
            result[i] = valueString[i];
        }
        
        result[valueLen] = '\0';
        
        return true;
    }
    else
    {
        return false;
    }
}

#define GetFromYAML(type, result, fmt, ...) \
{ \
    char buf[128]; \
    stbsp_sprintf(buf, fmt, ##__VA_ARGS__); \
             b32 boolRes = _Get##type##FromYAML(result, buf); \
ASSERT(boolRes); \
} \

#define GetFromYAMLUnchecked(type, result, fmt, ...) \
{ \
    char buf[128]; \
    stbsp_sprintf(buf, fmt, ##__VA_ARGS__); \
              _Get##type##FromYAML(result, buf); \
} \

#define GetFromYAMLReturn(type, result, boolResult, fmt, ...) \
{ \
    char buf[128]; \
    stbsp_sprintf(buf, fmt, ##__VA_ARGS__); \
             boolResult = _Get##type##FromYAML(result, buf); \
} \

s32 main(s32 argc, char **argv)
{
    LogToConsole("Initialising Zero...\n");
    b32 glfwInitResult = glfwInit();
    ASSERT(glfwInitResult);
    
    char *glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    GLFWwindow *window = glfwCreateWindow(1600, 900, "Zero", 0, 0);
    ASSERT(window);
    
    glfwMakeContextCurrent(window);
    
    b32 gladResult = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    ASSERT(gladResult);
    
    glfwSwapInterval(1);
    LogToConsole("GLFW successfully initialised\n");
    
    s32 screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);
    
    ImGui::StyleColorsDark();
    
    LogToConsole("ImGui successfully initialised\n");
    
    CalculatorState calcState = {};
    ClearCalcState(&calcState);
    
     s8 *irData = 0;
    s32 irDataLen = 0;
    
    iRacingState iRacing = {};
    InitialiseDriverColours(&iRacing);
    for (s32 i = 0; i < ARRAY_COUNT(iRacing.drivers); ++i)
    {
        iRacing.drivers[i].number = (char *)calloc(1, 8);
        iRacing.drivers[i].name = (char *)calloc(1, 64);
    }
    
    char *mfdAItems[] = {"Console Output", "Lap Time List", "Pit Loss Calc", "Trackmap Settings", "Sectors"};
    char *mfdBItems[] = {"Known Issues", "Lap Time Graph", "Running Gear Graph", "Best Lap Gear Graph", "Running Steer Graph", "Best Lap Steer Graph", "Live Delta Graph"};
    s32 mfdAItemsCount = ARRAY_COUNT(mfdAItems);
    s32 mfdBItemsCount = ARRAY_COUNT(mfdBItems);
    
    GLuint circleTexture;
    glGenTextures(1, &circleTexture);
    glBindTexture(GL_TEXTURE_2D, circleTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    s32 texWidth, texHeight;
    char *texPath = "data/circle.png";
    u8 *data = stbi_load(texPath, &texWidth, &texHeight, 0, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        printf("Failed to load texture: %s\n", texPath);
    }
    stbi_image_free(data);
    ImVec2 circleSize = {(f32)texWidth, (f32)texHeight};
    
    TrackmapSettings trackmapSettings = {};
    trackmapSettings.norm = ImVec2(1.0f, 1.0f);
    trackmapSettings.offset = ImVec2(1.0f, 1.0f);
    
    f64 lastTime = glfwGetTime();
    
    LogToConsole("Zero Initialised, proceeding with main loop\n\n");
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        f32 deltaTime = (f32)(glfwGetTime() - lastTime);
        lastTime = glfwGetTime();
        
        iRacing.isRunning = irsdk_isConnected();
        const irsdk_header *header = irsdk_getHeader();
        if (irsdk_waitForDataReady(TIMEOUT_MS, irData) && header)
        {
            if (!irData || irDataLen != header->bufLen)
            {
                if (irData)
                {
                    free(irData);
                }
                
                irDataLen = header->bufLen;
                irData = (s8 *)malloc(irDataLen);
#if 0
                {
                    FILE *outTestFile = fopen("testVars.csv", "w");
                    const irsdk_varHeader *varHeader = irsdk_getVarHeaderPtr();
                    fprintf(outTestFile, "Name,Unit,Type,Desc\n");
                    for(int index = 0; index < pHeader->numVars; index++)
                    {
                        const irsdk_varHeader *pVar = irsdk_getVarHeaderEntry(index);
                        fprintf(outTestFile, "%s,", pVar->name);
                        fprintf(outTestFile, "%s,", pVar->unit);
                        switch (pVar->type)
                        {
                            case irsdk_char:
                            {
                        fprintf(outTestFile, "s8,");
                            }break;
                            
                            case irsdk_bool:
                            {
                                fprintf(outTestFile, "b8,");
                            }break;
                            
                            case irsdk_int:
                            {
                                fprintf(outTestFile, "s32,");
                            }break;
                            
                            case irsdk_bitField:
                            {
                                fprintf(outTestFile, "u32,");
                            }break;
                            
                            case irsdk_float:
                            {
                                fprintf(outTestFile, "f32,");
                            }break;
                            
                            case irsdk_double:
                            {
                                fprintf(outTestFile, "f64,");
                            }break;
                            
                            default:
                            {
                                INVALID_CODE_PATH;
                            }break;
                        }
                        fprintf(outTestFile, "%s\n", pVar->desc);
                    }
                    return 0;
                }
#endif
            }
            else if (irData)
            {
                // TODO(bSalmon): Clean up the entirety of the program after sectors are done
                iRacingState oldiRacing = iRacing;
                
                iRacing.currSessionNum = GetIRSDKVar(s32, irData, "SessionNum");
                if (iRacing.currSessionNum != oldiRacing.currSessionNum || !iRacing.hasReceivedInitialData)
                {
                    s32 currSessionNum = iRacing.currSessionNum;
                    
                    iRacing = {};
                    InitialiseDriverColours(&iRacing);
                    iRacing.currSessionNum = currSessionNum;
                    
                    for (s32 i = 0; i < ARRAY_COUNT(iRacing.drivers); ++i)
                    {
                        free(iRacing.drivers[i].number);
                        free(iRacing.drivers[i].name);
                        free(iRacing.drivers[i].teamName);
                        
                        iRacing.drivers[i] = {};
                        
                        iRacing.drivers[i].number = (char *)calloc(1, 8);
                        iRacing.drivers[i].name = (char *)calloc(1, 64);
                        iRacing.drivers[i].teamName = (char *)calloc(1, 64);
                    }
                    
                    b32 countingSectors = true;
                    s32 index = 0;
                    do
                    {
                        s32 unused;
                        GetFromYAMLReturn(S32, &unused, countingSectors, "SplitTimeInfo:Sectors:SectorNum:{%d}", index++);
                    }
                    while (countingSectors);
                    iRacing.sectorCount = index - 1;
                    LogToConsole("SECTORS FOUND, SECTOR COUNT: %d\n", iRacing.sectorCount);
                    
                    if (index != 0 && !iRacing.sectors)
                    {
                        iRacing.sectors = (SectorInfo *)calloc(sizeof(SectorInfo), iRacing.sectorCount);
                        for (s32 sectorIndex = 0; sectorIndex < iRacing.sectorCount; ++sectorIndex)
                        {
                            iRacing.sectors[sectorIndex].sector = sectorIndex + 1;
                            iRacing.sectors[sectorIndex].sectorStartPct;
                            GetFromYAMLUnchecked(F32, &iRacing.sectors[sectorIndex].sectorStartPct, "SplitTimeInfo:Sectors:SectorNum:{%d}SectorStartPct:", sectorIndex);
                        }
                    }
                    
                    GetFromYAMLUnchecked(S32, &iRacing.isTeamSession, "WeekendInfo:TeamRacing:");
                    
                    GetFromYAML(String, iRacing.trackName, "WeekendInfo:TrackName:");
                    if (strcmp(iRacing.trackName, oldiRacing.trackName) != 0)
                    {
                        LogToConsole("Internal Track Name: %s\n\n", iRacing.trackName);
                    }
                    
                    LogToConsole("SESSION CHANGED, DATA PURGED\n");
                }
                
                DriverInfo oldDrivers[64] = {};
                for (s32 i = 0; i < ARRAY_COUNT(iRacing.drivers); ++i)
                {
                    oldDrivers[i] = iRacing.drivers[i];
                }
                
                b32 countingDrivers = true;
                s32 index = 0;
                do
                {
                    char unused[2];
                    GetFromYAMLReturn(String, unused, countingDrivers, "DriverInfo:Drivers:CarIdx:{%d}UserName:", index++);
                    iRacing.drivers[index].carIdx = index;
                }
                while (countingDrivers);
                iRacing.driverCount = index - 1;
                
                iRacing.ambientTemp = GetIRSDKVar(f32, irData, "AirTemp");
                iRacing.trackTemp = GetIRSDKVar(f32, irData, "TrackTemp");
                iRacing.humidity = GetIRSDKVar(f32, irData, "RelativeHumidity");
                iRacing.windSpeed = GetIRSDKVar(f32, irData, "WindVel");
                iRacing.windDir = GetIRSDKVar(f32, irData, "WindDir");
                iRacing.timeOfDay = GetIRSDKVar(f32, irData, "SessionTimeOfDay");
                
                iRacing.posArray = &GetIRSDKVar(s32, irData, "CarIdxPosition");
                iRacing.lapArray = &GetIRSDKVar(s32, irData, "CarIdxLap");
                iRacing.lapDistArray = &GetIRSDKVar(f32, irData, "CarIdxLapDistPct");
                iRacing.inPitsArray = &GetIRSDKVar(bool, irData, "CarIdxOnPitRoad");
                iRacing.trkSrfArray = &GetIRSDKVar(s32, irData, "CarIdxTrackSurface");
                iRacing.estTimeArray = &GetIRSDKVar(f32, irData, "CarIdxEstTime");
                iRacing.gearsArray = &GetIRSDKVar(s32, irData, "CarIdxGear");
                iRacing.steerArray = &GetIRSDKVar(f32, irData, "CarIdxSteer");
                
                iRacing.sessionLapsRemaining = GetIRSDKVar(s32, irData, "SessionLapsRemainEx");
                iRacing.sessionLaps = GetIRSDKVar(s32, irData, "SessionLapsTotal");
                iRacing.sessionTimeRemaining = GetIRSDKVar(f64, irData, "SessionTimeRemain");
                
                char sessionStr[32] = {};
                GetFromYAML(String, sessionStr, "SessionInfo:Sessions:SessionNum:{%d}SessionType:", iRacing.currSessionNum);
                iRacing.currSession = (strcmp(sessionStr, "Race") == 0) ? IR_SESSION_RACE : IR_SESSION_PRAC_QUAL;
                
                GetFromYAMLUnchecked(F32, &iRacing.gapData.leaderFastestLap, "SessionInfo:Sessions:SessionNum:{%d}ResultsFastestLap:FastestTime:", iRacing.currSessionNum);
                GetFromYAMLUnchecked(S32, &iRacing.fastestLapCarIdx, "SessionInfo:Sessions:SessionNum:{%d}ResultsFastestLap:CarIdx:", iRacing.currSessionNum);
                
                if (iRacing.currSession == IR_SESSION_RACE)
                {
                    iRacing.timedRace = (iRacing.sessionLapsRemaining == IRSDK_UNLIMITED_LAPS) ? true : false;
                    GetFromYAMLUnchecked(S32, &iRacing.leaderCarIdx, "SessionInfo:Sessions:SessionNum:{%d}ResultsPositions:Position:{%d}CarIdx:", iRacing.currSessionNum, 1);
                    iRacing.gapData.leaderCurrDist = iRacing.lapArray[iRacing.leaderCarIdx] + iRacing.lapDistArray[iRacing.leaderCarIdx];
                }
                
                for (s32 driverIndex = 0; driverIndex < iRacing.driverCount; ++driverIndex)
                {
                    DriverInfo *driver = &iRacing.drivers[driverIndex];
                    
                    if (iRacing.estTimeArray[driverIndex] > iRacing.maxEstTime)
                    {
                        iRacing.maxEstTime = iRacing.estTimeArray[driverIndex];
                    }
                    
                    GetFromYAMLUnchecked(String, driver->number, "DriverInfo:Drivers:CarIdx:{%d}CarNumber:", driverIndex);
                    GetFromYAMLUnchecked(String, driver->name, "DriverInfo:Drivers:CarIdx:{%d}UserName:", driverIndex);
                    GetFromYAMLUnchecked(String, driver->teamName, "DriverInfo:Drivers:CarIdx:{%d}TeamName:", driverIndex);
                    
                        // TODO(bSalmon): Maybe let it set from last lap time
                    GetFromYAMLUnchecked(F32, &driver->lastLapTime, "SessionInfo:Sessions:SessionNum:{%d}ResultsPositions:CarIdx:{%d}LastTime:", iRacing.currSessionNum, driverIndex);
                    GetFromYAMLUnchecked(F32, &driver->bestLapTime, "SessionInfo:Sessions:SessionNum:{%d}ResultsPositions:CarIdx:{%d}FastestTime:", iRacing.currSessionNum, driverIndex);
                    GetFromYAMLUnchecked(S32, &driver->bestLap, "SessionInfo:Sessions:SessionNum:{%d}ResultsPositions:CarIdx:{%d}FastestLap:", iRacing.currSessionNum, driverIndex);
                    
                    // NOTE(bSalmon): Done always in case of blinking
                    driver->currLapTime += deltaTime;
                    
                    driver->hasFastestLap = (driver->carIdx == iRacing.fastestLapCarIdx) ? true : false;
                    
                    if (iRacing.lapArray[driverIndex] != -1)
                    {
                        if (driver->bestLapTime != oldDrivers[driverIndex].bestLapTime)
                        {
                            driver->bestLapGearList.clear();
                            driver->bestLapGearList = driver->interGearList;

                            char str[16] = {};
                            FormatLapTime(driver->bestLapTime, str);
                            LogToConsole("%s has set a new PB: %s\n", driver->name, str);
                            
                            if (driver->hasFastestLap)
                            {
                                LogToConsole(0xFFBE46FF, "%s has set a new SB: %s\n", driver->name, str);
                            }
                        }
                    
                        driver->currLap = iRacing.lapArray[driverIndex];
                        driver->lapDistPct = iRacing.lapDistArray[driverIndex];
                        driver->carInWorld = (iRacing.trkSrfArray[driverIndex] != -1);
                        
                        LiveLapGraphInfo liveLapInfo = {};
                        liveLapInfo.lapDistPct = driver->lapDistPct;
                        liveLapInfo.lapTime = driver->currLapTime;
                        driver->runningLapList.push_back(liveLapInfo);
                        
                    if (iRacing.currSession != oldiRacing.currSession)
                    {
                        driver->lapTimeList.clear();
                    }
                    
                    if (driver->lastLapTime != oldDrivers[driverIndex].lastLapTime && (driver->currLap != -1 && driver->currLap != 0) && (oldDrivers[driverIndex].currLap != -1 && oldDrivers[driverIndex].currLap != 0))
                    {
                        LapGraphInfo info = {};
                        info.lap = driver->currLap - 1;
                        info.time = driver->lastLapTime;
                        
                        if (driver->lapTimeList.size() > 0 && driver->lapTimeList.back().lap != (info.lap - 1))
                        {
                            for (s32 i = driver->lapTimeList.back().lap + 1; i < info.lap; ++i)
                            {
                                LapGraphInfo errorFillInfo = {};
                                errorFillInfo.lap = i;
                                errorFillInfo.time = 0.0f;
                                driver->lapTimeList.push_back(errorFillInfo);
                            }
                        }
                        
                        b32 foundLap = false;
                        for (s32 i = 0; i < driver->lapTimeList.size(); ++i)
                        {
                            if (driver->lapTimeList[i].lap == info.lap)
                            {
                                foundLap = true;
                                break;
                            }
                        }
                        
                        if (!foundLap)
                        {
                            driver->lapTimeList.push_back(info);
                        }
                    }
                    
                    driver->inPits = iRacing.inPitsArray[driverIndex];
                    if (driver->inPits && !oldDrivers[driverIndex].inPits)
                    {
                        LogToConsole(0xFF00FFFF, "%s has entered Pit Lane\n", driver->name);
                    }
                    else if (!driver->inPits && oldDrivers[driverIndex].inPits)
                    {
                        LogToConsole(0xFF00FFFF, "%s has left Pit Lane\n", driver->name);
                    }
                    
                    if (driver->inPits)
                    {
                        driver->outlap = true;
                    }
                    else if ((driver->currLap != oldDrivers[driverIndex].currLap) && driver->outlap)
                    {
                        driver->outlap = false;
                    }
                    
                    if (iRacing.currSession == IR_SESSION_PRAC_QUAL)
                    {
                        driver->sortData.timedPos = iRacing.posArray[driverIndex];
                        driver->gap = driver->bestLapTime - iRacing.gapData.leaderFastestLap;
                        if (driver->sortData.timedPos == 0)
                        {
                            driver->bestLapTime = 0;
                            driver->bestLap = 0;
                            driver->lastLapTime = 0;
                        }
                    }
                    else
                    {
                        driver->sortData.currDist = iRacing.lapArray[driverIndex] + iRacing.lapDistArray[driverIndex];
                            
                            if (driver->inPits && (driver->sortData.currDist == oldDrivers[driverIndex].sortData.currDist))
                            {
                                driver->pitTimer += deltaTime;
                                driver->prePitBox = false;
                            }
                            else if (driver->inPits && driver->prePitBox)
                            {
                                driver->pitTimer = 0.0f;
                            }
                            else if (!driver->inPits)
                            {
                                driver->prePitBox = true;
                            }
                            
                        if (((iRacing.gapData.leaderCurrDist - driver->sortData.currDist) > 1.0f) && (iRacing.leaderCarIdx != driver->carIdx))
                        {
                            driver->gap = -(iRacing.gapData.leaderCurrDist - driver->sortData.currDist);
                        }
                        else
                        {
                            if (iRacing.lapArray[iRacing.leaderCarIdx] == iRacing.lapArray[driverIndex])
                            {
                                driver->gap = iRacing.estTimeArray[iRacing.leaderCarIdx] - iRacing.estTimeArray[driverIndex];
                            }
                            else
                            {
                                driver->gap = iRacing.estTimeArray[iRacing.leaderCarIdx] + (iRacing.maxEstTime - iRacing.estTimeArray[driverIndex]);
                            }
                        }
                    }
                    
                    if (driver->currLap == -1)
                    {
                        driver->tPosType = TPOS_OOW;
                    }
                    else if (driver->inPits)
                    {
                        driver->tPosType = TPOS_PIT;
                    }
                    else
                    {
                        driver->tPosType = TPOS_ONTRACK;
                        for (s32 sectorIndex = 0; sectorIndex < iRacing.sectorCount; ++sectorIndex)
                        {
                            if (driver->lapDistPct >= iRacing.sectors[sectorIndex].sectorStartPct)
                            {
                                driver->tPos = iRacing.sectors[sectorIndex].sector;
                                driver->currSectorInfo = iRacing.sectors[sectorIndex];
                            }
                        }
                    }
                        
                        if (driver->tPos == 2)
                        {
                            driver->pitTimer = 0.0f;
                        }
                        
                    if (driver->currSectorInfo.sector != oldDrivers[driverIndex].currSectorInfo.sector)
                    {
                        SectorTimeInfo sectorInfo = {};
                        sectorInfo.info = oldDrivers[driverIndex].currSectorInfo;
                        sectorInfo.lapTimeAtPoint = driver->currLapTime;
                        
                        if (driver->currSectorList.size() == 0)
                        {
                            sectorInfo.sectorTime = sectorInfo.lapTimeAtPoint;
                        }
                        else
                        {
                            sectorInfo.sectorTime = sectorInfo.lapTimeAtPoint - oldDrivers[driverIndex].currSectorList[driver->currSectorList.size() - 1].lapTimeAtPoint;
                        }
                        
                        if (driver->currSectorList.size() > iRacing.sectorCount || sectorInfo.info.sector == 1)
                        {
                            driver->currSectorList.clear();
                            }
                            
                            // NOTE(bSalmon): Check to make sure Sector 1 isn't being set immediately
                            if (sectorInfo.lapTimeAtPoint > 1.0f)
                            {
                                driver->currSectorList.push_back(sectorInfo);
                            }
                    }
                    
                        if (driver->bestLapTime != oldDrivers[driverIndex].bestLapTime)
                    {
                        if (driver->currSectorList.size() == iRacing.sectorCount)
                        {
                            driver->bestLapSectorList = driver->currSectorList;
                            driver->currSectorList.clear();
                                LogToConsole(0xFFFF0000, "Saved Sectors for %s\n", driver->name);
                            
                            if (driver->carIdx == iRacing.fastestLapCarIdx)
                            {
                                iRacing.bestLapSectorList = driver->bestLapSectorList;
                                LogToConsole(0xFFFF0000, "Sectors for %s saved as session fastest\n", driver->name);
                                }
                        }
                        else
                        {
                            LogToConsole(0xFF0000FF, "DEBUG: FAILED TO SAVE SECTORS FOR %s, DRIVER SECTOR COUNT WAS: %d, REQUIRED WAS: %d\n", driver->name, driver->currSectorList.size(), iRacing.sectorCount);
                        }
                    }
                    
                    if (driver->currLap != oldDrivers[driverIndex].currLap)
                    {
                            driver->currLapTime = 0.0f;
                            driver->runningLapList.clear();
                            driver->lastLapSectorList = driver->currSectorList;
                        }
                        
                    {
                    local_persist b32 interGearReady = false;
                    if (driver->lapDistPct < oldDrivers[driverIndex].lapDistPct)
                    {
                        driver->interGearList = driver->runningGearList;
                        driver->runningGearList.clear();
                        interGearReady = true;
                    }
                    
                    if (interGearReady)
                    {
                    if (driver->bestLapTime != oldDrivers[driverIndex].bestLapTime)
                    {
                            driver->bestLapGearList.clear();
                                driver->bestLapGearList = driver->interGearList;
                                interGearReady = false;
                        }
                        else if (driver->lapDistPct > 0.5f)
                        {
                            interGearReady = false;
                        }
                    }
                    
                    GearGraphInfo gearInfo = {};
                        gearInfo.lapDistPct = driver->lapDistPct;
                        gearInfo.gear = iRacing.gearsArray[driverIndex];
                    driver->runningGearList.push_back(gearInfo);
                    }
                        
                        {
                            local_persist b32 interSteerReady = false;
                            if (driver->lapDistPct < oldDrivers[driverIndex].lapDistPct)
                            {
                                driver->interSteerList = driver->runningSteerList;
                                driver->runningSteerList.clear();
                                interSteerReady = true;
                            }
                            
                            if (interSteerReady)
                            {
                                if (driver->bestLapTime != oldDrivers[driverIndex].bestLapTime)
                                {
                                    driver->bestLapSteerList.clear();
                                    driver->bestLapSteerList = driver->interSteerList;
                                    interSteerReady = false;
                                }
                                else if (driver->lapDistPct > 0.5f)
                                {
                                    interSteerReady = false;
                                }
                            }
                            
                            SteerGraphInfo gearInfo = {};
                            gearInfo.lapDistPct = driver->lapDistPct;
                            gearInfo.angle = iRacing.steerArray[driverIndex];
                            driver->runningSteerList.push_back(gearInfo);
                        }
                    }
                }
                
                iRacing.hasReceivedInitialData = true;
            }
        }
        
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        f32 margin = 0.005f;
        
        // NOTE(bSalmon): Main Timing
        ImVec2 mainTimingPos = ImVec2(screenWidth * margin, screenHeight * margin);
        ImVec2 mainTimingSize = ImVec2(screenWidth * (0.75f - margin), screenHeight * (0.66f - margin));
        
        //0ImGui::SetNextWindowPos(mainTimingPos);
        //ImGui::SetNextWindowSize(mainTimingSize);
        //ImGui::Begin("Main Timing", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::Begin("Main Timing", 0, ImGuiWindowFlags_NoCollapse);
        
        WeatherInfo("Ambient:", 0xFF3744FD, "%02.01fC", iRacing.ambientTemp);
        ImGui::SameLine();
        WeatherInfo("\t\tTrack:", 0xFF43C58B, "%02.01fC", iRacing.trackTemp);
        ImGui::SameLine();
        WeatherInfo("\t\tHumidity:", 0xFFFD4437, "%02.01f", iRacing.humidity * 100.0f);
        ImGui::SameLine();
        ImGui::Text("%%");
        ImGui::SameLine();
        WeatherInfo("\t\tWind:", 0xFF78EDE4, "%02.01fkm/h %c%c", iRacing.windSpeed, GetWindDirLetterFirst(iRacing.windDir), GetWindDirLetterSecond(iRacing.windDir));
        ImGui::SameLine();
        WeatherInfo("\t\tTime of Day:", 0xFFFFE697, "%02d:%02d %cM", GetTimeOfDayHour(iRacing.timeOfDay), GetTimeOfDayMinute(iRacing.timeOfDay), GetTimeOfDayMeridiem(iRacing.timeOfDay));
        ImGui::SameLine();
            // NOTE(bSalmon): Not Weather related but going in the same section
        if (iRacing.currSession == IR_SESSION_PRAC_QUAL || iRacing.timedRace)
        {
            WeatherInfo("\t\tTime Remaining:", 0xFF40A9FF, "%02d:%02d:%02d", (s32)(iRacing.sessionTimeRemaining / 60 / 60), GetTimeOfDayMinute((f32)iRacing.sessionTimeRemaining), RoundF32ToS32((f32)iRacing.sessionTimeRemaining) % 60);
        }
        else
        {
            WeatherInfo("\t\tLaps:", 0xFF40A9FF, "%d/%d", (iRacing.sessionLaps - iRacing.sessionLapsRemaining) + 1, iRacing.sessionLaps);
        }
        ImGui::Separator();
        
        s32 columnCount = TIMING_COLUMN_COUNT_BASE + iRacing.sectorCount;
        ImGui::BeginTable("Main Timing Table", columnCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp);
        ImGui::TableSetupColumn("Pos.");
        ImGui::TableSetupColumn("No.");
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Lap");
        ImGui::TableSetupColumn("TPos");
        ImGui::TableSetupColumn("Best Lap");
        ImGui::TableSetupColumn("Last Lap");
        ImGui::TableSetupColumn("Gap");
        
        for (s32 sectorIndex = 1; sectorIndex <= iRacing.sectorCount; ++sectorIndex)
        {
            char str[4] = {};
            sprintf(str, "S%d\0", sectorIndex);
                ImGui::TableSetupColumn(str);
        }
        
        ImGui::TableHeadersRow();
        
                MainTiming(&iRacing, columnCount);
        
        ImGui::EndTable();
        ImGui::End();
        
        // NOTE(bSalmon): Track Map
        ImVec2 trackMapPos = ImVec2((mainTimingPos.x + mainTimingSize.x) + (screenWidth * margin), mainTimingPos.y);
        ImVec2 trackMapSize = ImVec2((screenWidth * (1.0f - margin)) - (trackMapPos.x + (screenWidth * margin)), screenHeight * (0.4f - margin));
        
        //ImGui::SetNextWindowPos(trackMapPos);
        //ImGui::SetNextWindowSize(ImVec2(screenWidth * 0.245f, (screenHeight * 0.4f)));
        //ImGui::Begin("Track Map", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::Begin("Track Map", 0, ImGuiWindowFlags_NoCollapse);
        if (iRacing.hasReceivedInitialData)
        {
            TrackMap(&iRacing, &trackmapSettings, circleTexture, circleSize);
        }
        ImGui::End();
        
        // NOTE(bSalmon): Calculator
        ImVec2 calcPos = ImVec2(screenWidth * margin, (mainTimingPos.y + mainTimingSize.y) + (screenHeight * margin));
        ImVec2 calcSize = ImVec2(screenWidth * 0.2f, screenHeight * 0.225f);
        
        //ImGui::SetNextWindowPos(calcPos);
        //ImGui::SetNextWindowSize(calcSize);
        //ImGui::Begin("Calculator", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::Begin("Calculator", 0, ImGuiWindowFlags_NoCollapse);
        Calculator(&calcState);
        ImGui::End();
        
        // NOTE(bSalmon): MFD Setup
        ImVec2 setupPos = ImVec2(screenWidth * margin, (calcPos.y + calcSize.y) + (screenHeight * margin));
        ImVec2 setupSize = ImVec2(screenWidth * 0.2f, (screenHeight * (1.0f - margin)) - (setupPos.y + (screenHeight * margin)));
        
        //ImGui::SetNextWindowPos(setupPos);
        //ImGui::SetNextWindowSize(setupSize);
        //ImGui::Begin("MFD Setup", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::Begin("MFD Setup", 0, ImGuiWindowFlags_NoCollapse);
        local_persist s32 currMFDAIndex = 0;
        local_persist char *currMFDAItem = mfdAItems[currMFDAIndex];
        local_persist s32 currMFDBIndex = 0;
        local_persist char *currMFDBItem = mfdBItems[currMFDBIndex];
        CreateDropdown("  MFD A", &currMFDAItem, &currMFDAIndex, mfdAItems, mfdAItemsCount);
        CreateDropdown("  MFD B", &currMFDBItem, &currMFDBIndex, mfdBItems, mfdBItemsCount);
        ImGui::End();
        
        // NOTE(bSalmon): MFD A
        ImVec2 mfdAPos = ImVec2(trackMapPos.x, (trackMapPos.y + trackMapSize.y) + (screenHeight * (margin * 2.0f)));
        ImVec2 mfdASize = ImVec2(trackMapSize.x + (screenWidth * (margin * 2.0f)), (screenHeight * (1.0f - margin)) - (mfdAPos.y + (screenHeight * margin)));
        
        //ImGui::SetNextWindowPos(mfdAPos);
        //ImGui::SetNextWindowSize(mfdASize);
        //ImGui::Begin("MFD A", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::Begin("MFD A", 0, ImGuiWindowFlags_NoCollapse);
        switch (currMFDAIndex)
        {
            case 0:
            {
        MFD_A_ConsoleOutput();
            }break;
            
            case 1:
            {
                MFD_A_LapTimeList(&iRacing);
            }break;
            
            case 2:
            {
                MFD_A_PitLoss(&iRacing);
            }break;
            
            case 3:
            {
                MFD_A_TrackmapSettings(&trackmapSettings);
            }break;
            
            case 4:
            {
                MFD_A_Sectors(&iRacing);
            }break;
            
            default:
            {
                INVALID_CODE_PATH;
            }break;
        };
        ImGui::End();
        
        // NOTE(bSalmon): MFD B
        ImVec2 mfdBPos = ImVec2((calcPos.x + calcSize.x) + (screenWidth * margin), calcPos.y);
        ImVec2 mfdBSize = ImVec2((mfdAPos.x * (1.0f - margin)) - (mfdBPos.x), (screenHeight * (1.0f - margin)) - (mfdBPos.y + (screenHeight * margin)));
        
        //ImGui::SetNextWindowPos(mfdBPos);
        //ImGui::SetNextWindowSize(mfdBSize);
        //ImGui::Begin("MFD B", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::Begin("MFD B", 0, ImGuiWindowFlags_NoCollapse);
        switch (currMFDBIndex)
        {
            case 0:
            {
                ImGui::Text("Known Issues: ");
                ImGui::Text("\t- Results don't get locked in at the end of race sessions.");
                ImGui::Text("\t- Pit Stop Loss Trackmap Visualisation only work while active in MFD A.");
                ImGui::Text("\t- Live Gaps in races sometimes break after pit stops.");
                ImGui::Text("\t- Live Delta Graph in MFD B in completely useless, awaiting a full rework.");
                ImGui::Text("\t- Running/Live Graphs in MFD B seems to cause memory leaks due to tracking every car constantly.");
                ImGui::Text("\t- Starting the software while a car is in the middle of Sector 1 and then sets their best lap");
                ImGui::Text("\t  causes an issue where the sectors appear to be much quicker than the actual lap");
                
                ImGui::Text("\n");
                ImGui::Separator();
                ImGui::Text("\n");
                
                ImGui::Text("CHANGELOG");
                ImGui::Text("**********************************************");
                ImGui::Text("2021 - 09 - 02 -> 2021 - 09 - 03");
                ImGui::Text("- Added Last Lap Sectors to MFD A -> Sectors");
                ImGui::Text("- Drivers now all have individual colours akin to KD Timing");
                ImGui::Text("- Pit Stop Time is now shown and remains until the driver enters Sector 2, the time is calculated");
                ImGui::Text("  from the moment the car is stationary to the moment it sets off");
                ImGui::Text("- Sectors bounds are now shown on the trackmap");
                ImGui::Text("- Windows can now be moved and resized");
                ImGui::Text("**********************************************");
                
            }break;
            
            case 1:
            {
                MFD_B_LapTimeGraph(&iRacing);
            }break;
            
            case 2:
            {
                MFD_B_GearGraph(&iRacing, GRAPH_RUNNING);
            }break;
            
            case 3:
            {
                MFD_B_GearGraph(&iRacing, GRAPH_BEST);
            }break;
            
            case 4:
            {
                MFD_B_SteerGraph(&iRacing, GRAPH_RUNNING);
            }break;
            
            case 5:
            {
                MFD_B_SteerGraph(&iRacing, GRAPH_BEST);
            }break;
            
            case 6:
            {
                MFD_B_LiveDeltaGraph(&iRacing);
            }break;
            
            default:
            {
                INVALID_CODE_PATH;
            }break;
        };
        ImGui::End();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        glViewport(0, 0, screenWidth, screenHeight);
        glfwSwapBuffers(window);
    }
    
    return 0;
}