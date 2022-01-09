/*
Project: Arsene's Zero
File: zero_mfd_a.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

function void MFD_A_ConsoleOutput()
{
    ImGui::Text("Console Output:");
    ImGui::Separator();
    
    ImGui::BeginChild("Scroll Output", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    for (s32 i = 0; i < consoleBuffer.size(); ++i)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, consoleBuffer[i].colour);
        ImGui::Text(consoleBuffer[i].str);
        ImGui::PopStyleColor();
    }
    ImGui::PopStyleVar();
    
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

function void MFD_A_LapTimeList(iRacingState *iRacing)
{
    char *driverNames[64] = {};
    for (s32 i = 0; i < 64; ++i)
    {
        driverNames[i] = iRacing->drivers[i].name;
    }
    
    local_persist s32 currDriverIndex = 0;
    local_persist char *currDriverItem = iRacing->drivers[currDriverIndex].name;
    CreateDropdown("  Driver", &currDriverItem, &currDriverIndex, driverNames, iRacing->driverCount);
    ImGui::Separator();
    
    for (s32 i = 0; i < iRacing->drivers[currDriverIndex].lapTimeList.size(); ++i)
    {
        char str[16] = {};
        FormatLapTime(iRacing->drivers[currDriverIndex].lapTimeList[i].time, str);
        ImGui::Text("%d: %s", iRacing->drivers[currDriverIndex].lapTimeList[i].lap, str);
    }
}

function void MFD_A_PitLoss(iRacingState *iRacing)
{
    local_persist f32 tyreTime = 0.0f;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Total 4 Tyre Time", &tyreTime, 0.0f, 60.0f, "%.2f");
    
    local_persist f32 tyreMultiplier = 1.0f;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Tyre Time Multiplier", &tyreMultiplier, 0.25f, 1.0f, "%.2f");
    
    ImGui::Separator();
    
    local_persist f32 secondsPerLitre = 0.0f;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Seconds Per Litre", &secondsPerLitre, 0.0f, 30.0f, "%.2f");
    
    local_persist f32 litresFill = 0.0f;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Litres to Fill", &litresFill, 0.0f, 200.0f, "%.2f");
    
    local_persist bool simultaneousFuelTyres = false;
    ImGui::Checkbox("Fuel and Tyres are Simultaneous", &simultaneousFuelTyres);
    
    ImGui::Separator();
    
    local_persist f32 stopGoLoss = 0.0f;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Stop-Go Time Loss", &stopGoLoss, 0.0f, 120.0f, "%.2f");
    
    ImGui::Separator();
    
    local_persist f32 totalLoss = 0.0f;
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Override Total Loss", &totalLoss, 0.0f, 240.0f, "%.2f");
    
    ImGui::NewLine();
    f32 totalTyreTime = tyreTime * tyreMultiplier;
    f32 totalFuelTime = secondsPerLitre * litresFill;
    f32 stoppedTime = (simultaneousFuelTyres) ? (MAX(totalTyreTime, totalFuelTime)) : (totalTyreTime + totalFuelTime);
    f32 approxTimeLoss = stoppedTime + stopGoLoss;
    if (totalLoss != 0.0f)
    {
        approxTimeLoss = totalLoss;
    }
    else
    {
        ImGui::Text("Time Stopped: %.2f", stoppedTime);
    }
    ImGui::Text("Approx. Time Loss: %.2f", approxTimeLoss);
    ImGui::NewLine();
    
    ImGui::Separator();
    
    // TODO(bSalmon): Trackmap should probably show a range
    
    char *driverNames[64] = {};
    for (s32 i = 0; i < 64; ++i)
    {
        driverNames[i] = iRacing->drivers[i].name;
    }
    
    local_persist s32 currDriverIndex = 0;
    local_persist char *currDriverItem = iRacing->drivers[currDriverIndex].name;
    CreateDropdown("  Driver", &currDriverItem, &currDriverIndex, driverNames, iRacing->driverCount);
    
    ImGui::Checkbox("Show Projected Exit Location On Trackmap", &iRacing->showPitLossOnTrackmap);
    
    f32 driverEstTime = iRacing->estTimeArray[currDriverIndex];
    if (approxTimeLoss < driverEstTime)
    {
        f32 estTimeCalc = driverEstTime - approxTimeLoss;
        f32 approxLapDistPct = estTimeCalc / iRacing->maxEstTime;
        iRacing->pitLossLapDistPct = approxLapDistPct;
    }
    else if (approxTimeLoss == driverEstTime)
    {
        iRacing->pitLossLapDistPct = 0.0f;
    }
    else
    {
        // TODO(bSalmon): Need to add support for going multiple laps down in a pitstop
        f32 estTimeCalc = iRacing->maxEstTime - (approxTimeLoss - driverEstTime);
        f32 approxLapDistPct = estTimeCalc / iRacing->maxEstTime;
        iRacing->pitLossLapDistPct = approxLapDistPct;
    }
}

function void MFD_A_TrackmapSettings(TrackmapSettings *trackmapSettings)
{
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Norm X", &trackmapSettings->norm.x, 0.0f, 10.0f, "%.1f");
    
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Norm Y", &trackmapSettings->norm.y, 0.0f, 10.0f, "%.1f");
    
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Offset X", &trackmapSettings->offset.x, -5.0f, 5.0f, "%.1f");
    
    ImGui::SetNextItemWidth(ImGui::GetWindowSize().x * 0.25f);
    ImGui::InputFloat("Offset Y", &trackmapSettings->offset.y, -5.0f, 5.0f, "%.1f");
}

function void MFD_A_Sectors(iRacingState *iRacing)
{
    char *driverNames[64] = {};
    for (s32 i = 0; i < 64; ++i)
    {
        driverNames[i] = iRacing->drivers[i].name;
    }
    
    local_persist s32 currDriverIndex = 0;
    local_persist char *currDriverItem = iRacing->drivers[currDriverIndex].name;
    CreateDropdown("  Driver", &currDriverItem, &currDriverIndex, driverNames, iRacing->driverCount);
    ImGui::Separator();
    
    ImGui::Text("Best Lap Sectors");
    
    s32 colCount = 4;
    ImGui::BeginTable("Sectors Table", colCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("S");
    ImGui::TableSetupColumn("S Time");
    ImGui::TableSetupColumn("L Time");
    ImGui::TableSetupColumn("P1 Del");
    ImGui::TableHeadersRow();
    
        for (s32 rowIndex = 0; rowIndex < iRacing->drivers[currDriverIndex].bestLapSectorList.size(); ++rowIndex)
    {
        ImGui::TableNextRow();
        for (s32 colIndex = 0; colIndex < colCount; ++colIndex)
        {
            ImGui::TableSetColumnIndex(colIndex);
            
            switch (colIndex)
            {
                case 0:
                {
                    ImGui::Text("%d", rowIndex + 1);
                }break;
                
                case 1:
                {
                    char time[32];
                    FormatLapTime(iRacing->drivers[currDriverIndex].bestLapSectorList[rowIndex].sectorTime, time);
                    ImGui::Text(time);
                }break;
                
                case 2:
                {
                    char time[32];
                    FormatLapTime(iRacing->drivers[currDriverIndex].bestLapSectorList[rowIndex].lapTimeAtPoint, time);
                    ImGui::Text(time);
                }break;
                
                case 3:
                {
                    if (iRacing->bestLapSectorList.size() > 0)
                    {
                    f32 sectorDelta = iRacing->drivers[currDriverIndex].bestLapSectorList[rowIndex].sectorTime - iRacing->bestLapSectorList[rowIndex].sectorTime;
                    char time[32];
                    
                    if (sectorDelta < 0.0f)
                    {
                        time[0] = '-';
                    }
                    else if (sectorDelta > 0.0f)
                    {
                        time[0] = '+';
                    }
                    
                    FormatLapTime(Abs(sectorDelta), &time[1]);
                        ImGui::Text(time);
                    }
                }break;
                
                default:
                {
                    INVALID_CODE_PATH;
                }break;
            }
        }
    }
    ImGui::EndTable();
    
    ImGui::Separator();
    
    ImGui::Text("Last Lap Sectors");
    
    ImGui::BeginTable("Sectors Table", colCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("S");
    ImGui::TableSetupColumn("S Time");
    ImGui::TableSetupColumn("L Time");
    ImGui::TableSetupColumn("Del");
    ImGui::TableHeadersRow();
    
    for (s32 rowIndex = 0; rowIndex < iRacing->drivers[currDriverIndex].lastLapSectorList.size(); ++rowIndex)
    {
        ImGui::TableNextRow();
        for (s32 colIndex = 0; colIndex < colCount; ++colIndex)
        {
            ImGui::TableSetColumnIndex(colIndex);
            
            switch (colIndex)
            {
                case 0:
                {
                    ImGui::Text("%d", rowIndex + 1);
                }break;
                
                case 1:
                {
                    char time[32];
                    FormatLapTime(iRacing->drivers[currDriverIndex].lastLapSectorList[rowIndex].sectorTime, time);
                    ImGui::Text(time);
                }break;
                
                case 2:
                {
                    char time[32];
                    FormatLapTime(iRacing->drivers[currDriverIndex].lastLapSectorList[rowIndex].lapTimeAtPoint, time);
                    ImGui::Text(time);
                }break;
                
                case 3:
                {
                    if (iRacing->drivers[currDriverIndex].bestLapSectorList.size() > 0)
                    {
                        f32 sectorDelta = iRacing->drivers[currDriverIndex].lastLapSectorList[rowIndex].sectorTime - iRacing->drivers[currDriverIndex].bestLapSectorList[rowIndex].sectorTime;
                        char time[32];
                        
                        if (sectorDelta < 0.0f)
                        {
                            time[0] = '-';
                        }
                        else if (sectorDelta > 0.0f)
                        {
                            time[0] = '+';
                        }
                        
                        FormatLapTime(Abs(sectorDelta), &time[1]);
                        ImGui::Text(time);
                    }
                }break;
                
                default:
                {
                    INVALID_CODE_PATH;
                }break;
            }
        }
    }
    ImGui::EndTable();
    
    ImGui::Separator();
    
    ImGui::Text("Curr Lap Sectors");
    
    ImGui::BeginTable("Sectors Table", colCount, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("S");
    ImGui::TableSetupColumn("S Time");
    ImGui::TableSetupColumn("L Time");
    ImGui::TableSetupColumn("Del");
    ImGui::TableHeadersRow();
    
    for (s32 rowIndex = 0; rowIndex < iRacing->drivers[currDriverIndex].currSectorList.size(); ++rowIndex)
    {
        ImGui::TableNextRow();
        for (s32 colIndex = 0; colIndex < colCount; ++colIndex)
        {
            ImGui::TableSetColumnIndex(colIndex);
            
            switch (colIndex)
            {
                case 0:
                {
                    ImGui::Text("%d", rowIndex + 1);
                }break;
                
                case 1:
                {
                    char time[32];
                    FormatLapTime(iRacing->drivers[currDriverIndex].currSectorList[rowIndex].sectorTime, time);
                    ImGui::Text(time);
                }break;
                
                case 2:
                {
                    char time[32];
                    FormatLapTime(iRacing->drivers[currDriverIndex].currSectorList[rowIndex].lapTimeAtPoint, time);
                    ImGui::Text(time);
                }break;
                
                case 3:
                {
                    if (iRacing->drivers[currDriverIndex].bestLapSectorList.size() > 0)
                    {
                        f32 sectorDelta = iRacing->drivers[currDriverIndex].currSectorList[rowIndex].sectorTime - iRacing->drivers[currDriverIndex].bestLapSectorList[rowIndex].sectorTime;
                        char time[32];
                        
                        if (sectorDelta < 0.0f)
                        {
                            time[0] = '-';
                        }
                        else if (sectorDelta > 0.0f)
                        {
                            time[0] = '+';
                        }
                        
                        FormatLapTime(Abs(sectorDelta), &time[1]);
                        ImGui::Text(time);
                    }
                }break;
                
                default:
                {
                    INVALID_CODE_PATH;
                }break;
            }
        }
    }
    ImGui::EndTable();
}