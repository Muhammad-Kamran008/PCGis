#pragma once
#include "CoreMinimal.h"
#include "PCGContent.h"
#include "Components/SplineComponent.h"
#include "PCGComponent.h"
#include "PCGPolygonActor.h"
#include "PolygonHiGenActor.h"
#include "PcgSQLiteSubsystem.h"
#include "PCGPolygonContent.generated.h"


USTRUCT()
struct FBakedInstanceBatch
{
    GENERATED_BODY();

    // Mesh of the batch
    UPROPERTY(SaveGame) FSoftObjectPath MeshPath;
    // Optional material overrides per slot (leave empty if not used)
    UPROPERTY(SaveGame) TArray<FSoftObjectPath> Materials;
    // All instance transforms
    UPROPERTY(SaveGame) TArray<FTransform> Transforms;
};


UCLASS()
class CUSTOMPCG_API APCGPolygonContent : public APCGContent
{
    GENERATED_BODY()

private:
    UGameInstance* GI;
    UPcgSQLiteSubsystem* DBSubsystem;

public:

    APCGPolygonContent();


    virtual void BeginPlay() override;

    // Called when the game starts or when spawned
    virtual void InitializeContent() override;

    virtual void Tick(float DeltaTime) override;

    void InitializePCGPolygonData(const FShapeRawData& Raw);

    bool InsertPolygonsFeaturesToDB(const FString& ShapefileID);

    bool InsertPolygonFeaturesToDB(const FGrassPolygonData& Data);

    bool InsertPolygonFeaturesToDB(const FString& ShapefileID);

    void SpawnPCGPolygonData();

    void SpawnIndividualPCGPolygonData(const FGrassPolygonData& Data);

    void AssignPCGGraph(UPCGComponent* TargetPCG, const FString& GraphName);

    void SpawnHiGenActorsFromDatabase(const FString& ShapefileName);

    // If you want these visible in the editor, wrap with UPROPERTY + Category.
    UPROPERTY() ACesiumGeoreference* CesiumGeoreference = nullptr;
    UPROPERTY() ACesium3DTileset* Tileset = nullptr;

    TArray<FGrassPolygonData> PolygonDataList;

    UPROPERTY() AActor* ParentActor = nullptr;

    FTimerHandle handle;

    bool InsertPolygonPointsToDB(const FString& ShapefileID, FGrassPolygonData PolygonID, const TArray<FTransform>& Instances, const FString& MeshID);

    UFUNCTION()
    void OnPcgGraphGenerated(UPCGComponent* PCG);


    // Keep ParentActor public as you already had
   //UPROPERTY() AActor* ParentActor = nullptr;


};
