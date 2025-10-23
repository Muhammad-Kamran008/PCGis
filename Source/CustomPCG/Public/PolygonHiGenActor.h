#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGComponent.h"
#include "Components/BoxComponent.h"
#include "PCGSubsystem.h"
#include "PolygonHiGenActor.generated.h"

UCLASS(Blueprintable)
class CUSTOMPCG_API APolygonHiGenActor : public AActor
{
    GENERATED_BODY()

public:
    APolygonHiGenActor();
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HiGen")
    UPCGComponent* PCGComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HiGen")
    UBoxComponent* BoxComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HiGen")
    TArray<FVector> Positions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HiGen")
    FVector BoxExtents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HiGen")
    int32 PolygonID;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiGen")
    FString GraphName;

    void AssignHiGenGraph();

};


