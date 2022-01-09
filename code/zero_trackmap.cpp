/*
Project: Arsene's Zero
File: zero_trackmap.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

struct TrackmapSettings
{
    ImVec2 norm;
    ImVec2 offset;
};

 function FiledTrack LoadTrackData(char *trackName, FILE *dataFile)
{
    FiledTrack result = {};
    
    while (result.nameLen == 0)
    {
        FiledTrack trackCheck = ReadTrackData(dataFile);
        if (strcmp(trackCheck.name, trackName) == 0)
        {
             result = trackCheck;
        }
        else
        {
        free(trackCheck.name);
        free(trackCheck.points);
        }
    }
    
    return result;
}

inline ImVec2 Normalise(ImVec2 val, ImVec2 min, ImVec2 max)
{
    ImVec2 result = val;
    
    result.x -= min.x;
    result.y -= min.y;
    max.x -= min.x;
    max.y -= min.y;
    result.x /= MAX(max.x, max.y);
    result.y /= MAX(max.x, max.y);
    
    return result;
}

inline s32 Lerp(s32 a, s32 b, f32 t)
{
    s32 result = (s32)((1.0f - t) * a + t * b);
    return result;
}

function void DrawLine(void *bitmapMemory, ImVec2 dims, ImVec2 a, ImVec2 b, u32 colour)
{
    s32 dx = Abs(RoundF32ToS32(b.x) - RoundF32ToS32(a.x));
    s32 dy = Abs(RoundF32ToS32(b.y) - RoundF32ToS32(a.y));
    
    s32 lineLength = SqRt(Sq(dx) + Sq(dy));
    for (s32 i = 0; i < lineLength; ++i)
    {
        *((u32 *)bitmapMemory + (Lerp(RoundF32ToS32(a.y), RoundF32ToS32(b.y), ((f32)i / (f32)lineLength)) * (s32)dims.x) + Lerp(RoundF32ToS32(a.x), RoundF32ToS32(b.x), ((f32)i / (f32)lineLength))) = colour;
    }
}

function void DrawTrackData(iRacingState *iRacing, FiledTrack *trackData, TrackmapSettings *trackmapSettings, GLuint trackTexture, ImVec2 *min, ImVec2 *max, ImVec2 *trackDims)
{
     *min = ImVec2(90.0f, 180.0f);
     *max = ImVec2(-90.0f, -180.0f);
    
    ImVec2 *pointArray = (ImVec2 *)malloc(trackData->pointCount * sizeof(ImVec2));
    
    for (s32 pointIndex = 0; pointIndex < trackData->pointCount; ++pointIndex)
    {
        TrackPoint currPoint = trackData->points[pointIndex];
        
        if (currPoint.lat < min->x)
        {
            min->x = (f32)currPoint.lat;
        }
        if (currPoint.lon < min->y)
        {
            min->y = (f32)currPoint.lon;
        }
        
        if (currPoint.lat > max->x)
        {
            max->x = (f32)currPoint.lat;
        }
        if (currPoint.lon > max->y)
        {
            max->y = (f32)currPoint.lon;
        }
    }
    
    min->x -= 0.0005f;
    min->y -= 0.0005f;
    max->x += 0.0005f;
    max->y += 0.0005f;
    
    for (s32 pointIndex = 0; pointIndex < trackData->pointCount; ++pointIndex)
    {
        ImVec2 point = {(f32)trackData->points[pointIndex].lat, (f32)trackData->points[pointIndex].lon};
        pointArray[pointIndex] = Normalise(point, *min, *max);
    }
    
    ImVec2 normMin = Normalise(*min, *min, *max);
    ImVec2 normMax = Normalise(*max, *min, *max);
    
    normMin.x *= trackmapSettings->norm.x;
    normMin.y *= trackmapSettings->norm.y;
    normMax.x *= trackmapSettings->norm.x;
    normMax.y *= trackmapSettings->norm.y;
    
    f32 trackAspect = (normMax.y - normMin.y) / (normMax.x - normMin.x);
    f32 windowAspect = ImGui::GetWindowSize().y / ImGui::GetWindowSize().x;
    
    if (trackAspect < windowAspect)
    {
        // NOTE(bSalmon): Limit by Width
        trackDims->x = normMax.x * (ImGui::GetWindowSize().x * 0.9f);
        trackDims->y = trackDims->x * trackAspect;
    }
    else
    {
        // NOTE(bSalmon): Limit by Height
        trackDims->y = normMax.y * (ImGui::GetWindowSize().y * 0.9f);
        trackDims->x = trackDims->y / trackAspect;
    }
    
    trackDims->x = (f32)RoundF32ToS32(trackDims->x);
    trackDims->y = (f32)RoundF32ToS32(trackDims->y);
    
    u32 *bitmapMemory = (u32 *)calloc(sizeof(u32), (s32)(trackDims->x + 2) * (s32)(trackDims->y + 2));
    ImVec2 pointA = pointArray[trackData->pointCount - 1];
    ImVec2 pointB = pointArray[0];
    for (s32 pointIndex = 0; pointIndex < trackData->pointCount; ++pointIndex)
    {
        pointA.x *= trackDims->x;
        pointA.y *= trackDims->y;
        pointB.x *= trackDims->x;
        pointB.y *= trackDims->y;
        
        u32 pointColour = 0xFFFFFFFF;
        for (s32 sectorIndex = 0; sectorIndex < iRacing->sectorCount; ++sectorIndex)
        {
            if (Abs(iRacing->sectors[sectorIndex].sectorStartPct - trackData->points[pointIndex].lapDistPct) < 0.005f)
        {
                pointColour = 0xFF0000FF;
                break;
            }
        }
        
        DrawLine(bitmapMemory, *trackDims, pointA, pointB, pointColour);
        
        if (pointIndex == trackData->pointCount - 1)
        {
            break;
        }
        
        pointA = pointArray[pointIndex];
        pointB = pointArray[pointIndex + 1];
    }

    if (trackTexture == 0)
    {
    glBindTexture(GL_TEXTURE_2D, trackTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (s32)trackDims->x, (s32)trackDims->y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmapMemory);
    }

    free(bitmapMemory);
    free(pointArray);

    ImVec2 imageCursorPos = ImVec2((ImGui::GetWindowSize().x - trackDims->x) * (0.5f * trackmapSettings->offset.x), (ImGui::GetWindowSize().y - trackDims->y) * (0.5f * trackmapSettings->offset.y));
    ImGui::SetCursorPos(imageCursorPos);
    ImGui::Image((void *)(intptr_t)trackTexture, *trackDims);
}

function void TrackMap(iRacingState *iRacing, TrackmapSettings *trackmapSettings, GLuint circleTexture, ImVec2 circleSize)
{
    if (iRacing->isRunning)
    {
            ImVec2 min = {};
            ImVec2 max = {};
        ImVec2 trackDims = {};
        local_persist GLuint trackTexture = 0;
        
        if (!iRacing->isTrackmapLoaded)
        {
            char str[64] = {};
            stbsp_sprintf(str, "tracks/%s.ztd", iRacing->trackName);
        FILE *dataFile = fopen(str, "rb+");
            if (dataFile && IsTrackFiled(iRacing->trackName, dataFile))
            {
                fseek(dataFile, 0, SEEK_SET);
                  iRacing->trackData = LoadTrackData(iRacing->trackName, dataFile);
                    iRacing->isTrackmapLoaded = true;
                
                fclose (dataFile);
        }
        else
        {
            ImGui::Text("Failed to find relevant track map data");
            }
        }
        else
        {
            ImGui::Text("N ->");
            DrawTrackData(iRacing, &iRacing->trackData, trackmapSettings, trackTexture, &min, &max, &trackDims);
            for (s32 driverIndex = 0; driverIndex <= iRacing->driverCount; ++driverIndex)
            {
                if (iRacing->drivers[driverIndex].carInWorld)
                {
                    TrackPoint currClosest = iRacing->trackData.points[0];
                    f32 currDelta = 100.0f;
                    for (s32 pointIndex = 0; pointIndex < iRacing->trackData.pointCount; ++pointIndex)
                    {
                        f32 delta = Abs(iRacing->trackData.points[pointIndex].lapDistPct - iRacing->drivers[driverIndex].lapDistPct);
                        if (delta < currDelta)
                            {
                            currDelta = delta;
                            currClosest = iRacing->trackData.points[pointIndex];
                        }
                        else if (delta > currDelta)
                            {
                            break;
                            }
                    }
                    
                    ImVec4 circleBack = ImVec4(1, 1, 1, 1);
                    ImVec4 circleFore = ImVec4(1, 1, 1, 1);
                    if (strcmp(iRacing->drivers[driverIndex].name, "Pace Car") == 0)
                    {
                        // NOTE(bSalmon): Pace Car = black
                        circleBack = ImVec4(0, 0, 0, 1);
                        circleFore = ImVec4(0, 0, 0, 1);
                    }
                    
                    circleBack = iRacing->colours[driverIndex].back;
                    circleFore = iRacing->colours[driverIndex].fore;
                    
                    ImVec2 carPoint = {currClosest.lat, currClosest.lon};
                    ImVec2 carPosNorm = Normalise(carPoint, min, max);
                    ImVec2 drawPoint = {(carPosNorm.x * trackDims.x) - (circleSize.x / 2), (carPosNorm.y * trackDims.y) - (circleSize.y / 2)};
                    ImVec2 imageCursorPos = ImVec2(((ImGui::GetWindowSize().x - trackDims.x) * (0.5f * trackmapSettings->offset.x)) + drawPoint.x, ((ImGui::GetWindowSize().y - trackDims.y) * (0.5f * trackmapSettings->offset.y)) + drawPoint.y);
                    ImGui::SetCursorPos(imageCursorPos);
                    ImGui::Image((void *)(intptr_t)circleTexture, circleSize, ImVec2(0, 0), ImVec2(1, 1), circleBack, ImVec4(0, 0, 0, 0));
                    
                    ImVec2 circleSizeFore = {circleSize.x / 2.0f, circleSize.y / 2.0f};
                    drawPoint = {(carPosNorm.x * trackDims.x) - (circleSizeFore.x / 2), (carPosNorm.y * trackDims.y) - (circleSizeFore.y / 2)};
                    imageCursorPos = ImVec2(((ImGui::GetWindowSize().x - trackDims.x) * (0.5f * trackmapSettings->offset.x)) + drawPoint.x,
                                            ((ImGui::GetWindowSize().y - trackDims.y) * (0.5f * trackmapSettings->offset.y)) + drawPoint.y);
                    ImGui::SetCursorPos(imageCursorPos);
                    ImGui::Image((void *)(intptr_t)circleTexture, circleSizeFore, ImVec2(0, 0), ImVec2(1, 1), circleFore, ImVec4(0, 0, 0, 0));
                }
            }
            
            if (iRacing->showPitLossOnTrackmap)
            {
                TrackPoint currClosest = iRacing->trackData.points[0];
                f32 currDelta = 100.0f;
                for (s32 pointIndex = 0; pointIndex < iRacing->trackData.pointCount; ++pointIndex)
                {
                    f32 delta = Abs(iRacing->trackData.points[pointIndex].lapDistPct - iRacing->pitLossLapDistPct);
                    if (delta < currDelta)
                    {
                        currDelta = delta;
                        currClosest = iRacing->trackData.points[pointIndex];
                    }
                    else if (delta > currDelta)
                    {
                        break;
                    }
                }
                
                ImVec4 circleColour = ImVec4(1, 1, 1, 1);
                ImVec2 carPoint = {currClosest.lat, currClosest.lon};
                ImVec2 carPosNorm = Normalise(carPoint, min, max);
                ImVec2 drawPoint = {(carPosNorm.x * trackDims.x) - (circleSize.x / 2), (carPosNorm.y * trackDims.y) - (circleSize.y / 2)};
                ImVec2 imageCursorPos = ImVec2(((ImGui::GetWindowSize().x - trackDims.x) * (0.5f * trackmapSettings->offset.x)) + drawPoint.x, ((ImGui::GetWindowSize().y - trackDims.y) * (0.5f * trackmapSettings->offset.y)) + drawPoint.y);
                ImGui::SetCursorPos(imageCursorPos);
                ImGui::Image((void *)(intptr_t)circleTexture, circleSize, ImVec2(0, 0), ImVec2(1, 1), circleColour, ImVec4(0, 0, 0, 0));
            }
        }
    }
    else
    {
        ImGui::Text("Waiting for the iRacing Service");
    }
}