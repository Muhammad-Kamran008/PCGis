// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGContent.h"
#include "PolygonCacheSave.h"
#include "SQLiteDatabase.h"
#include "PCGManager.generated.h"


UCLASS()
class CUSTOMPCG_API APCGManager : public AActor
{
	GENERATED_BODY()


public:
    APCGManager();

    TQueue<UPCGComponent*> pendingPCGActors;
    TArray<UPCGComponent*> activePCGActors;
    int32 MaxActivePCG = 50;

    virtual void BeginPlay() override;

    void LoadDataforPCGPoint();

    void InitializeDatabase();

    void LoadDataforPCGPolygon();

private:
    FSQLiteDatabase DB;
};

