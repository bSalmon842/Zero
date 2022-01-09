/*
Project: Arsene's Zero
File: zero_file.cpp
Author: Brock Salmon
Notice: (C) Copyright 2021 by Brock Salmon. All Rights Reserved
*/

struct TrackPoint
{
    f32 lapDistPct;
    f32 lat;
    f32 lon;
};

struct FiledTrack
{
    s32 nameLen;
    char *name;
    s32 pointCount;
    TrackPoint *points;
};

inline s32 GetFileSize(FILE *file)
{
    s32 result = 0;
    
    fseek(file, 0, SEEK_END);
     result = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    return result;
}

function FiledTrack ReadTrackData(FILE *dataFile)
{
    FiledTrack track = {};

#if 1
    s32 fileSize = GetFileSize(dataFile);
    void *fileData = calloc(1, fileSize);
    fread(fileData, fileSize, 1, dataFile);
    
    track.nameLen = *(s32 *)fileData;
    track.name = (char *)malloc(track.nameLen);
    CopyMem(track.name, ((s8 *)fileData + sizeof(s32)), track.nameLen);
    
    track.pointCount = *(s32 *)((s8 *)fileData + (track.nameLen) + sizeof(s32));
    track.points = (TrackPoint *)calloc(track.pointCount, sizeof(TrackPoint));
    for (s32 pointIndex = 0; pointIndex < track.pointCount; ++pointIndex)
    {
        f32 lapDistPct = *(f32 *)((s8 *)fileData + (track.nameLen) + (2 * sizeof(s32)) + ((3 * sizeof(f32)) * pointIndex));
        f32 lat = *(f32 *)((s8 *)fileData + (track.nameLen) + (2 * sizeof(s32)) + ((3 * sizeof(f32)) * pointIndex) + sizeof(f32));
        f32 lon = *(f32 *)((s8 *)fileData + (track.nameLen) + (2 * sizeof(s32)) + ((3 * sizeof(f32)) * pointIndex) + (2 * sizeof(f32)));
        
        track.points[pointIndex].lapDistPct = lapDistPct;
        track.points[pointIndex].lat = lat;
        track.points[pointIndex].lon = lon;
    }
    
    free(fileData);
    #else
    fread(&track.nameLen, sizeof(s32), 1, dataFile);
    track.name = (char *)malloc(track.nameLen);
    fread(track.name, sizeof(char), track.nameLen, dataFile);
    fread(&track.pointCount, sizeof(s32), 1, dataFile);
    track.point = (TrackPoint *)malloc(track.pointCount * sizeof(TrackPoint));
    for (s32 pointIndex = 0; pointIndex < track.pointCount; ++pointIndex)
    {
        f32 lapDistPct = 0.0f;
        f32 lat = 0.0f;
        f32 lon = 0.0f;
        
        fread(&lapDistPct, sizeof(f32), 1, dataFile);
        fread(&lat, sizeof(f32), 1, dataFile);
        fread(&lon, sizeof(f32), 1, dataFile);
        
        track.point[pointIndex].lapDistPct = lapDistPct;
        track.point[pointIndex].lat = lat;
        track.point[pointIndex].lon = lon;
    }
    #endif

    return track;
}

function b32 IsTrackFiled(char *trackName, FILE *dataFile)
{
    b32 result = false;
    
    s32 runningSize = 0;
    s32 fileSize = GetFileSize(dataFile);
    while (!result && (runningSize < fileSize))
    {
    FiledTrack trackCheck = ReadTrackData(dataFile);
    
    if (strcmp(trackCheck.name, trackName) == 0)
    {
         result = true;
    }
    
    free(trackCheck.name);
        free(trackCheck.points);
        
        runningSize += ftell(dataFile);
    }
    
    return result;
}
