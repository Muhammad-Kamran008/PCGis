#include "PolygonHiGenActor.h"
#include "PCGGraph.h"
#include "Engine/World.h"
#include "RuntimeGen/SchedulingPolicies/PCGSchedulingPolicyDistanceAndDirection.h"


APolygonHiGenActor::APolygonHiGenActor()
{
    PrimaryActorTick.bCanEverTick = false;
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    RootComponent = BoxComponent;
   // BoxComponent->SetBoxExtent(FVector(20000.f, 20000.f, 200.f));
    BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BoxComponent->SetHiddenInGame(true);
    BoxComponent->SetCanEverAffectNavigation(false);
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    PCGComponent->bActivated = true;
    PCGComponent->SetIsPartitioned(true);
    PCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateAtRuntime;
    TSubclassOf<UPCGSchedulingPolicyBase> PolicyClass = UPCGSchedulingPolicyDistanceAndDirection::StaticClass();
    PCGComponent->SetSchedulingPolicyClass(PolicyClass);
    PCGComponent->bOverrideGenerationRadii = true;
    FPCGRuntimeGenerationRadii bGenerationRadius;
    bGenerationRadius.GenerationRadius = 512.0f;
    bGenerationRadius.GenerationRadius25600 = 512.0f;
    bGenerationRadius.CleanupRadiusMultiplier = 1.1f;
    PCGComponent->GenerationRadii = bGenerationRadius;
}

void APolygonHiGenActor::AssignHiGenGraph()
{
    if (GraphName.IsEmpty() || !PCGComponent) return;


    const FString GraphFolderPath = TEXT("/Game/PCGData/PolygonDataAssets/HiGen");
    FString AssetName = FString::Printf(TEXT("PolygonData_%s"), *GraphName);
    FString AssetPath = FString::Printf(TEXT("%s/%s.%s"), *GraphFolderPath, *AssetName, *AssetName);
    UPCGGraph* Graph = Cast<UPCGGraph>(StaticLoadObject(UPCGGraph::StaticClass(), nullptr, *AssetPath));
    if (!Graph)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load HiGen Graph: %s"), *GraphName);
        return;
    }
    BoxComponent->SetBoxExtent(BoxExtents);
    PCGComponent->SetGraph(Graph);
}

void APolygonHiGenActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    PCGComponent->Cleanup();
    PCGComponent->UnregisterComponent();
    Super::EndPlay(EndPlayReason);
}