/*
Project: Arsene's Zero
File: zero_mfd_b.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

// TODO(bSalmon): Running Lap Time Delta

function void MFD_B_LapTimeGraph(iRacingState *iRacing)
{
    char *driverNames[64] = {};
    for (s32 i = 0; i < 64; ++i)
    {
        driverNames[i] = iRacing->drivers[i].name;
    }
    
    local_persist s32 currDriverAIndex = 0;
    local_persist char *currDriverAItem = iRacing->drivers[currDriverAIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.4f);
    CreateDropdown(" A\t", &currDriverAItem, &currDriverAIndex, driverNames, iRacing->driverCount);
    
    ImGui::SameLine();
    
    local_persist s32 currDriverBIndex = 0;
    local_persist char *currDriverBItem = iRacing->drivers[currDriverBIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.4f);
    CreateDropdown(" B", &currDriverBItem, &currDriverBIndex, driverNames, iRacing->driverCount);
    
    ImGui::Separator();
    
    s32 sizeA = iRacing->drivers[currDriverAIndex].lapTimeList.size();
    f32 *xDataA = (f32 *)calloc(sizeA, sizeof(f32));
    f32 *yDataA = (f32 *)calloc(sizeA, sizeof(f32));
    
    s32 sizeB = iRacing->drivers[currDriverBIndex].lapTimeList.size();
    f32 *xDataB = (f32 *)calloc(sizeB, sizeof(f32));
    f32 *yDataB = (f32 *)calloc(sizeB, sizeof(f32));
    
    for (s32 i = 0; i < sizeA; ++i)
    {
        if (!(iRacing->drivers[currDriverAIndex].lapTimeList[i].time <= 0.0f || iRacing->drivers[currDriverAIndex].lapTimeList[i].lap == 0))
        {
            xDataA[i] = (f32)iRacing->drivers[currDriverAIndex].lapTimeList[i].lap;
            yDataA[i] = iRacing->drivers[currDriverAIndex].lapTimeList[i].time;
        }
    }
    
    for (s32 i = 0; i < sizeB; ++i)
    {
        if (!(iRacing->drivers[currDriverBIndex].lapTimeList[i].time <= 0.0f || iRacing->drivers[currDriverBIndex].lapTimeList[i].lap == 0))
        {
            xDataB[i] = (f32)iRacing->drivers[currDriverBIndex].lapTimeList[i].lap;
            yDataB[i] = iRacing->drivers[currDriverBIndex].lapTimeList[i].time;
        }
    }
    
    ImPlot::BeginPlot("Lap Time Graph", "Lap", "Lap Time", ImVec2(ImGui::GetWindowSize().x * 0.98f, ImGui::GetWindowSize().y * 0.775f), ImPlotFlags_NoTitle | ImPlotFlags_NoLegend);
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorTickLen, ImVec2(10.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorTickLen, ImVec2(1.0f, 0.1f));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(1.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, ImVec2(1.0f, 1.0f));
    ImPlot::PushStyleColor(ImPlotCol_Line, 0xFFFFFFFF);
    
    if (strcmp(currDriverAItem, "Pace Car") != 0)
    {
        ImPlot::PushStyleColor(ImPlotCol_MarkerFill, 0xFF0000FF);
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond);
        ImPlot::PlotLine("Driver A Lap Time", xDataA, yDataA, sizeA);
        ImPlot::PopStyleColor();
    }
    
    
    if (strcmp(currDriverBItem, "Pace Car") != 0)
    {
        ImPlot::PushStyleColor(ImPlotCol_MarkerFill, 0xFF00FF00);
        ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond);
        ImPlot::PlotLine("Driver B Lap Time", xDataB, yDataB, sizeB);
        ImPlot::PopStyleColor();
    }
    
    ImPlot::PopStyleColor();
    ImPlot::PopStyleVar(4);
    ImPlot::EndPlot();
    
    free(xDataA);
    free(yDataA);
    free(xDataB);
    free(yDataB);
}

function void MFD_B_GearGraph(iRacingState *iRacing, GraphType graph)
{
    char *driverNames[64] = {};
    for (s32 i = 0; i < 64; ++i)
    {
        driverNames[i] = iRacing->drivers[i].name;
    }
    
    local_persist s32 currDriverAIndex = 0;
    local_persist char *currDriverAItem = iRacing->drivers[currDriverAIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.4f);
    CreateDropdown(" A\t", &currDriverAItem, &currDriverAIndex, driverNames, iRacing->driverCount);
    
    ImGui::SameLine();
    
    local_persist s32 currDriverBIndex = 0;
    local_persist char *currDriverBItem = iRacing->drivers[currDriverBIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.4f);
    CreateDropdown(" B", &currDriverBItem, &currDriverBIndex, driverNames, iRacing->driverCount);
    
    ImGui::Separator();
    
    ImVector<GearGraphInfo> gearListA;
    ImVector<GearGraphInfo> gearListB;
    
    if (graph == GRAPH_RUNNING)
    {
        gearListA = iRacing->drivers[currDriverAIndex].runningGearList;
        gearListB = iRacing->drivers[currDriverBIndex].runningGearList;
    }
    else
    {
        gearListA = iRacing->drivers[currDriverAIndex].bestLapGearList;
        gearListB = iRacing->drivers[currDriverBIndex].bestLapGearList;
    }
    
    s32 sizeA = gearListA.size();
    f32 *xDataA = (f32 *)calloc(sizeA, sizeof(f32));
    f32 *yDataA = (f32 *)calloc(sizeA, sizeof(f32));
    
    s32 sizeB = gearListB.size();
    f32 *xDataB = (f32 *)calloc(sizeB, sizeof(f32));
    f32 *yDataB = (f32 *)calloc(sizeB, sizeof(f32));
    
    for (s32 i = 0; i < sizeA; ++i)
    {
        xDataA[i] = gearListA[i].lapDistPct * 10.0f;
        yDataA[i] = (f32)gearListA[i].gear;
    }
    
    for (s32 i = 0; i < sizeB; ++i)
    {
        xDataB[i] = gearListB[i].lapDistPct * 10.0f;
        yDataB[i] = (f32)gearListB[i].gear;
    }
    
    ImPlot::BeginPlot("Gear Graph", "LapPct (* 10.0f)", "Gear", ImVec2(ImGui::GetWindowSize().x * 0.98f, ImGui::GetWindowSize().y * 0.775f), ImPlotFlags_NoTitle | ImPlotFlags_NoLegend);
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorTickLen, ImVec2(10.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorTickLen, ImVec2(5.0f, 0.5f));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(1.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, ImVec2(1.0f, 1.0f));
    
    if (strcmp(currDriverAItem, "Pace Car") != 0)
    {
    ImPlot::PushStyleColor(ImPlotCol_Line, 0xFF0000FF);
        ImPlot::PlotLine("Driver A Gear", xDataA, yDataA, sizeA);
        ImPlot::PopStyleColor();
    }
    
    if (strcmp(currDriverBItem, "Pace Car") != 0)
    {
        ImPlot::PushStyleColor(ImPlotCol_Line, 0xFF00FF00);
        ImPlot::PlotLine("Driver B Gear", xDataB, yDataB, sizeB);
        ImPlot::PopStyleColor();
    }
    
    ImPlot::PopStyleVar(4);
    ImPlot::EndPlot();
    
    free(xDataA);
    free(yDataA);
    free(xDataB);
    free(yDataB);
}

#define RAD_TO_DEG (180.0f / PI)
function void MFD_B_SteerGraph(iRacingState *iRacing, GraphType graph)
{
    char *driverNames[64] = {};
    for (s32 i = 0; i < 64; ++i)
    {
        driverNames[i] = iRacing->drivers[i].name;
    }
    
    local_persist s32 currDriverAIndex = 0;
    local_persist char *currDriverAItem = iRacing->drivers[currDriverAIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.4f);
    CreateDropdown(" A\t", &currDriverAItem, &currDriverAIndex, driverNames, iRacing->driverCount);
    
    ImGui::SameLine();
    
    local_persist s32 currDriverBIndex = 0;
    local_persist char *currDriverBItem = iRacing->drivers[currDriverBIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.4f);
    CreateDropdown(" B", &currDriverBItem, &currDriverBIndex, driverNames, iRacing->driverCount);
    
    ImGui::Separator();
    
    ImVector<SteerGraphInfo> steerListA;
    ImVector<SteerGraphInfo> steerListB;
    
    if (graph == GRAPH_RUNNING)
    {
        steerListA = iRacing->drivers[currDriverAIndex].runningSteerList;
        steerListB = iRacing->drivers[currDriverBIndex].runningSteerList;
    }
    else
    {
        steerListA = iRacing->drivers[currDriverAIndex].bestLapSteerList;
        steerListB = iRacing->drivers[currDriverBIndex].bestLapSteerList;
    }
    
    s32 sizeA = steerListA.size();
    f32 *xDataA = (f32 *)calloc(sizeA, sizeof(f32));
    f32 *yDataA = (f32 *)calloc(sizeA, sizeof(f32));
    
    s32 sizeB = steerListB.size();
    f32 *xDataB = (f32 *)calloc(sizeB, sizeof(f32));
    f32 *yDataB = (f32 *)calloc(sizeB, sizeof(f32));
    
    for (s32 i = 0; i < sizeA; ++i)
    {
        xDataA[i] = steerListA[i].lapDistPct * 200.0f;
        yDataA[i] = (f32)steerListA[i].angle * RAD_TO_DEG;
    }
    
    for (s32 i = 0; i < sizeB; ++i)
    {
        xDataB[i] = steerListB[i].lapDistPct * 200.0f;
        yDataB[i] = (f32)steerListB[i].angle * RAD_TO_DEG;
    }
    
    ImPlot::BeginPlot("Steer Graph", "LapPct", "Steer", ImVec2(ImGui::GetWindowSize().x * 0.98f, ImGui::GetWindowSize().y * 0.775f), ImPlotFlags_NoTitle | ImPlotFlags_NoLegend);
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorTickLen, ImVec2(10.0f, 50.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorTickLen, ImVec2(5.0f, 25.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(1.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, ImVec2(1.0f, 1.0f));
    
    if (strcmp(currDriverAItem, "Pace Car") != 0)
    {
        ImPlot::PushStyleColor(ImPlotCol_Line, 0xFF0000FF);
        ImPlot::PlotLine("Driver A Steer", xDataA, yDataA, sizeA);
        ImPlot::PopStyleColor();
    }
    
    if (strcmp(currDriverBItem, "Pace Car") != 0)
    {
        ImPlot::PushStyleColor(ImPlotCol_Line, 0xFF00FF00);
        ImPlot::PlotLine("Driver B Steer", xDataB, yDataB, sizeB);
        ImPlot::PopStyleColor();
    }
    
    ImPlot::PopStyleVar(4);
    ImPlot::EndPlot();
    
    free(xDataA);
    free(yDataA);
    free(xDataB);
    free(yDataB);
}

// TODO(bSalmon): Match this according to lapDistPct rather than samples
// TODO(bSalmon): Base it off a list of the best lap
function void MFD_B_LiveDeltaGraph(iRacingState *iRacing)
{
#if 0
    char *driverNames[64] = {};
    for (s32 i = 0; i < 64; ++i)
    {
        driverNames[i] = iRacing->drivers[i].name;
    }
    
    local_persist s32 currDriverAIndex = 0;
    local_persist char *currDriverAItem = iRacing->drivers[currDriverAIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    CreateDropdown(" A\t", &currDriverAItem, &currDriverAIndex, driverNames, iRacing->driverCount);
    
    ImGui::SameLine();
    
    local_persist s32 currDriverBIndex = 0;
    local_persist char *currDriverBItem = iRacing->drivers[currDriverBIndex].name;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    CreateDropdown(" B", &currDriverBItem, &currDriverBIndex, driverNames, iRacing->driverCount);
    
    s32 sizeA = iRacing->drivers[currDriverAIndex].runningLapList.size();
    f32 *xDataA = (f32 *)calloc(sizeA, sizeof(f32));
    f32 *yDataA = (f32 *)calloc(sizeA, sizeof(f32));
    
    s32 sizeB = iRacing->drivers[currDriverBIndex].runningLapList.size();
    f32 *xDataB = (f32 *)calloc(sizeB, sizeof(f32));
    f32 *yDataB = (f32 *)calloc(sizeB, sizeof(f32));
    
    char baseName[64] = {};
    if (sizeA >= sizeB)
    {
    for (s32 i = 0; i < sizeB; ++i)
    {
        xDataA[i] = iRacing->drivers[currDriverAIndex].runningLapList[i].lapDistPct * 10.0f;
        xDataB[i] = iRacing->drivers[currDriverBIndex].runningLapList[i].lapDistPct * 10.0f;
            
            yDataA[i] = (f32)iRacing->drivers[currDriverAIndex].runningLapList[i].lapTime - iRacing->drivers[currDriverBIndex].runningLapList[i].lapTime;
        yDataB[i] = 0.0f;
        }
        
        stbsp_sprintf(baseName, "%s", iRacing->drivers[currDriverBIndex].name);
    }
    else
    {
        for (s32 i = 0; i < sizeA; ++i)
        {
            xDataA[i] = iRacing->drivers[currDriverAIndex].runningLapList[i].lapDistPct * 10.0f;
            xDataB[i] = iRacing->drivers[currDriverBIndex].runningLapList[i].lapDistPct * 10.0f;
            
            yDataA[i] = 0.0f;
            yDataB[i] = (f32)iRacing->drivers[currDriverBIndex].runningLapList[i].lapTime - iRacing->drivers[currDriverAIndex].runningLapList[i].lapTime;
        }
        
        stbsp_sprintf(baseName, "%s", iRacing->drivers[currDriverAIndex].name);
    }
    
    ImGui::SameLine();
    ImGui::Text("Base: %s", baseName);
    
    ImGui::Separator();
    
    ImPlot::BeginPlot("Live Delta Graph", "LapPct", "Delta", ImVec2(ImGui::GetWindowSize().x * 0.98f, ImGui::GetWindowSize().y * 0.775f), ImPlotFlags_NoTitle | ImPlotFlags_NoLegend);
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorTickLen, ImVec2(10.0f, 10.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorTickLen, ImVec2(5.0f, 0.5f));
    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(1.0f, 1.0f));
    ImPlot::PushStyleVar(ImPlotStyleVar_LabelPadding, ImVec2(1.0f, 1.0f));
    
    if (strcmp(currDriverAItem, "Pace Car") != 0)
    {
        ImPlot::PushStyleColor(ImPlotCol_Line, 0xFF0000FF);
        ImPlot::PlotLine("Driver A Delta", xDataA, yDataA, sizeA);
        ImPlot::PopStyleColor();
    }
    
    if (strcmp(currDriverBItem, "Pace Car") != 0)
    {
        ImPlot::PushStyleColor(ImPlotCol_Line, 0xFF00FF00);
        ImPlot::PlotLine("Driver B Delta", xDataB, yDataB, sizeB);
        ImPlot::PopStyleColor();
    }
    
    ImPlot::PopStyleVar(4);
    ImPlot::EndPlot();
    
    free(xDataA);
    free(yDataA);
    free(xDataB);
    free(yDataB);
#endif
}