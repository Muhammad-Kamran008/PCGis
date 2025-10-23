// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SQLiteDatabase.h"
#include "GridStreamingManager.generated.h"

UCLASS()
class CUSTOMPCG_API AGridStreamingManager : public AActor
{
	GENERATED_BODY()
	
public:
    AGridStreamingManager();
    virtual void Tick(float DeltaTime) override;
    void LoadDataFromDatabase(const FString& ShapefileID);
    void UpdateStreaming(FVector PawnLocation);

    FSQLiteDatabase DB;
private:
    TSet<FIntPoint> LoadedCells;
    UPROPERTY() AActor* ParentActor;
    int32 Radius = 500; // radius around pawn
    int32 GridSize = 100; 
    APlayerController* PC;
    void LoadCell(FIntPoint Cell);
    void LoadCellAsync(FIntPoint Cell);
    void UnloadCell(FIntPoint Cell);

    int32 DrawEveryNFrames = 60; 
    int32 FramesSinceUpdate = 0;
    TSet<FIntPoint> RequestedCells; 


};
