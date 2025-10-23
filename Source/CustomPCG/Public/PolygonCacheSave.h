// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PolygonCacheSave.generated.h"





//USTRUCT()
//struct FPolygonCacheRecord
//{
//    GENERATED_BODY();
//
//    // For change detection
//    UPROPERTY(SaveGame) FString ShapefilePath;
//    UPROPERTY(SaveGame) int32   FeatureId = -1;
//    UPROPERTY(SaveGame) uint32  SourceHash = 0; // hash of raw polygon data + file timestamp
//
//    // Height-adjusted boundary (so you never sample again)
//    UPROPERTY(SaveGame) TArray<FVector> BoundaryPoints;
//
//    // Baked instances produced by PCG (what we will respawn)
//    UPROPERTY(SaveGame) TArray<FBakedInstanceBatch> InstanceBatches;
//};


UCLASS()
class CUSTOMPCG_API UPolygonCacheSave : public USaveGame
{
    GENERATED_BODY()
public:
    //UPROPERTY(SaveGame) int32 Version = 1;
    //UPROPERTY(SaveGame) FString MapId;
    //UPROPERTY(SaveGame) TArray<FPolygonCacheRecord> Records;
    //// Optional: one hash per shapefile if you prefer coarse invalidation
    //UPROPERTY(SaveGame) TMap<FString, uint32> FileHash;
};




