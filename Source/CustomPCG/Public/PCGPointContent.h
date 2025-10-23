// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGContent.h"
#include "BlueprintRegistery.h"
#include "Engine/Blueprint.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "HAL/ThreadSafeCounter.h"
#include "PCGPointContent.generated.h"

// --- STRUCT 1: For vegetation_elev.shp ---
USTRUCT(BlueprintType)
struct FVegetationPointData
{
    GENERATED_BODY();

    // Category groups pins on Make/Break struct nodes and satisfies the Engine-module rule.
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") int32 Id;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString Name;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") float Lat;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") float Long;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") float Elev;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") float Height;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") float Scale;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") float Rotation;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString Code;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString Model;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString State;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") float Altitude;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") int32 KindID;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString KindDesc;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") int32 DomainID;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString DomainDesc;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") int32 CountryID;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString CountryDes;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") int32 CategoryID;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString CategoryDe;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") int32 SubCategID;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString SubCategDe;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") int32 SpecificID;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString SpecificDe;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString EntityEnum;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FVector Location;
    UPROPERTY(BlueprintReadWrite, Category = "CustomPCG|PointData") FString FileName;
};

UCLASS()
class CUSTOMPCG_API APCGPointContent : public APCGContent
{
    GENERATED_BODY()

public:
    // Constructor
    APCGPointContent();

    // Not exposed -> no Category required
    TArray<FVegetationPointData> PointDataList;

    // Override the InitializeContent function
    virtual void InitializeContent() override;

    void InitializePCGPointData(const FShapeRawData& Raw);
    void SpawnPCGPointData();
    void SpawnPointDataOnTerrainUsingCesiumSampler(const FVegetationPointData& Data);
    void SpawnPointDataOnTerrainUsingElevData(const FVegetationPointData& Data);
    void SpawnPointDataOnTerrainUsingCesiumSampler_HISM(const FVegetationPointData& Data);

    void SpawnPointDataOnTerrainUsingElevData_HISM(const FVegetationPointData& Data);

    UStaticMesh* ExtractMeshFromBlueprint(TSubclassOf<AActor> BPClass);

    // If you want to see these in Details, add VisibleAnywhere + Category.
    // Otherwise, keep them as plain members or minimally as UPROPERTY without exposure.
    UPROPERTY() ACesiumGeoreference* CesiumGeoreference = nullptr;
    UPROPERTY() ACesium3DTileset* Tileset = nullptr;

    UPROPERTY() UBlueprintRegistery* BlueprintRegistry = nullptr;

    UPROPERTY() AActor* ParentActor = nullptr;

    // Caches extracted static meshes for each object type
    UPROPERTY() TMap<FString, UStaticMesh*> StaticMeshesPerType;

    // Maps type name to its corresponding HISM component
    UPROPERTY() TMap<FString, UHierarchicalInstancedStaticMeshComponent*> InstancedMeshMap;

    static int32     TotalPointDataPoints;
    static FDateTime StartTime;
    static int32     ExpectedTotalPoints;
    static bool      bTimerStarted;
};
