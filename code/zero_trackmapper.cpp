/*
Project: Arsene's Zero
File: zero_trackmapper.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

#define WITHOUT_IMGUI
#include "zero_utility.h"

#include "irsdk/irsdk_defines.h"
#include "irsdk/irsdk_utils.cpp"

#include "re.h"

#include "zero_file.cpp"

s32 main(s32 argc, char **argv)
{
    ASSERT(argc > 1);
    
    irsdk_startup();
    
    FILE *ibtFile = fopen(argv[1], "rb");
    FILE *outFile = 0;
    
    for (s32 charIndex = 0; charIndex < StringLength(argv[1]); ++charIndex)
    {
        if (argv[1][charIndex] == '_')
        {
            argv[1][charIndex] = ' ';
        }
        
        s32 length = 0;
        s32 match = re_match("^20[0-9][0-9]-[0-1][0-9]-[0-3][0-9]", &argv[1][charIndex], &length);
        if (match == 0)
        {
            s32 nameLen = charIndex;
            char *name = (char *)malloc(nameLen);
            CopyMem(name, argv[1], nameLen);
            name[charIndex - 1] = '\0';
            
            char str[64] = {};
            stbsp_sprintf(str, "%s.ztd", name);
            outFile = fopen(str, "wb+");
            
            if(IsTrackFiled(name, outFile))
            {
                printf("Track Data already exsits\n");
                return 1;
            }
            else
            {
                fseek(outFile, 0, SEEK_END);
                fwrite(&nameLen, sizeof(s32), 1, outFile);
                fwrite(name, sizeof(char), nameLen, outFile);
            break;
            }
        }
    }
    
    s32 pointCountLoc = ftell(outFile);
    s32 runningPointCount = 0;
    
    s32 lapDistPctIndex = 0;
    s32 latIndex = 0;
    s32 lonIndex = 0;
    s32 onPitRoadIndex = 0;
    
    irsdk_header header;
    fread(&header, 1, sizeof(header), ibtFile);
    
    irsdk_varHeader *varHeaders = new irsdk_varHeader[header.numVars];
    if (varHeaders)
    {
        fseek(ibtFile, header.varHeaderOffset, SEEK_SET);
        fread(varHeaders, 1, header.numVars * sizeof(irsdk_varHeader), ibtFile);
        
        for (s32 i = 0; i < header.numVars; ++i)
        {
            irsdk_varHeader *currHeader = &varHeaders[i];
            
            if (currHeader)
            {
                if (strcmp(currHeader->name, "LapDistPct") == 0)
                {
                    lapDistPctIndex = i;
                }
                
                if (strcmp(currHeader->name, "Lat") == 0)
                {
                    latIndex = i;
                }
                
                if (strcmp(currHeader->name, "Lon") == 0)
                {
                    lonIndex = i;
                }
                
                if (strcmp(currHeader->name, "OnPitRoad") == 0)
                {
                     onPitRoadIndex = i;
                }
            }
        }
    }
    
    char *varBuffer = new char[header.bufLen];
    if (varBuffer)
    {
        fseek(ibtFile, header.varBuf[0].bufOffset, SEEK_SET);
        
        b32 recording = false;
                  b32 firstPoint = false;
        while (fread(varBuffer, sizeof(char), header.bufLen, ibtFile))
        {
            f32 lapDistPct = 0.0f;
            f32 lat = 0.0;
            f32 lon = 0.0;
            bool inPitLane = true;
            for (s32 i = 0; i < header.numVars; ++i)
            {
                irsdk_varHeader *currHeader = &varHeaders[i];
                
                if (currHeader)
                {
                    if (i == lapDistPctIndex)
                    {
                        lapDistPct = *(f32 *)(varBuffer + currHeader->offset);
                    }
                    else if (i == latIndex)
                    {
                        lat = (f32)(*(f64 *)(varBuffer + currHeader->offset));
                    }
                    else if (i == lonIndex)
                    {
                        lon = (f32)(*(f64 *)(varBuffer + currHeader->offset));
                    }
                    else if (i == onPitRoadIndex)
                    {
                         inPitLane = *(bool *)(varBuffer + currHeader->offset);
                    }
                }
            }
            
            // NOTE(bSalmon): The lat lon check is fine unless they build a track at the intersect of the equator and the prime meridian
            if (lapDistPct < 0.001f && !inPitLane && lat != 0.0f && lon != 0.0f)
            {
                recording = true;
            }
            
            // NOTE(bSalmon): To filter out the first foundd point crossing the line which seems to have a garbage LapDistPct value
            if (recording)
            {
                if (firstPoint)
                {
                runningPointCount++;
                
                if (lapDistPct > 0.999f)
                {
                    recording = false;
                    break;
                    }
                }
                else
                {
                    firstPoint = true;
                }
            }
        }
    }
    
    fwrite(&runningPointCount, sizeof(s32), 1, outFile);
    
    f32 *pointData = (f32 *)calloc(sizeof(f32), runningPointCount * 3);
    s32 pointIndex = 0;
    
    if (varBuffer)
    {
        fseek(ibtFile, header.varBuf[0].bufOffset, SEEK_SET);
        
        b32 recording = false;
        b32 firstPoint = false;
        while (fread(varBuffer, sizeof(char), header.bufLen, ibtFile))
        {
            f32 lapDistPct = 0.0f;
            f32 lat = 0.0;
            f32 lon = 0.0;
            bool inPitLane = true;
            for (s32 i = 0; i < header.numVars; ++i)
            {
                irsdk_varHeader *currHeader = &varHeaders[i];
                
                if (currHeader)
                {
                    if (i == lapDistPctIndex)
                    {
                        lapDistPct = *(f32 *)(varBuffer + currHeader->offset);
                    }
                    else if (i == latIndex)
                    {
                        lat = (f32)(*(f64 *)(varBuffer + currHeader->offset));
                    }
                    else if (i == lonIndex)
                    {
                        lon = (f32)(*(f64 *)(varBuffer + currHeader->offset));
                    }
                    else if (i == onPitRoadIndex)
                    {
                        inPitLane = *(bool *)(varBuffer + currHeader->offset);
                    }
                }
            }
            
            // NOTE(bSalmon): The lat lon check is fine unless they build a track at the intersect of the equator and the prime meridian
            if (lapDistPct < 0.001f && !inPitLane && lat != 0.0f && lon != 0.0f)
            {
                recording = true;
            }
            
            // NOTE(bSalmon): To filter out the first foundd point crossing the line which seems to have a garbage LapDistPct value
            if (recording)
            {
                if (firstPoint)
                {
                    pointData[pointIndex * 3] = lapDistPct;
                    pointData[(pointIndex * 3) + 1] = lat;
                    pointData[(pointIndex * 3) + 2] = lon;
                    pointIndex++;
                    
                    if (lapDistPct > 0.999f)
                    {
                        recording = false;
                        break;
                    }
                }
                else
                {
                    firstPoint = true;
                }
            }
        }
    }
    
    fwrite(pointData, sizeof(f32) * (runningPointCount * 3), 1, outFile);
    
    delete[] varHeaders;
    varHeaders = 0;
    
    delete[] varBuffer;
    varBuffer = 0;
    
        fclose(outFile);
    fclose(ibtFile);
    return 0;
}
