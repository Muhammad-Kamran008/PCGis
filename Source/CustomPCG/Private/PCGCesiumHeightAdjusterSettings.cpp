// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGCesiumHeightAdjusterSettings.h"
#include "PCGComponent.h"
#include "Data/PCGPointData.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "PCGPolygonContent.h"
#include "EngineUtils.h"
#include <optional>

#define LOCTEXT_NAMESPACE "PCGCesiumHeightAdjusterElement"


#if WITH_EDITOR
// The label the node is known as internally.
FName UPCGCesiumHeightAdjusterSettings::GetDefaultNodeName() const
{
    return FName(TEXT("PCGCesiumHeightAdjuster"));
}

// Default node name shown in the graph editor. Include spaces.
FText UPCGCesiumHeightAdjusterSettings::GetDefaultNodeTitle() const
{
    return LOCTEXT("NodeTitle", "PCGCesiumHeightAdjuster");
}

// Default tooltip for the node
FText UPCGCesiumHeightAdjusterSettings::GetNodeTooltipText() const
{
    return LOCTEXT("NodeTooltip", "Add tooltip here.");
}
#endif //WITH_EDITOR

TArray<FPCGPinProperties> UPCGCesiumHeightAdjusterSettings::InputPinProperties() const
{
    return { FPCGPinProperties(PCGPinConstants::DefaultInputLabel, EPCGDataType::Spatial) };
}

TArray<FPCGPinProperties> UPCGCesiumHeightAdjusterSettings::OutputPinProperties() const
{
    return { FPCGPinProperties(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Spatial) };
}

FPCGElementPtr UPCGCesiumHeightAdjusterSettings::CreateElement() const
{

    UE_LOG(LogTemp, Warning, TEXT("Creating PCG Element for Cesium Height Adjuster"));
    return MakeShared<FPCGCesiumHeightAdjusterElement>();

}


//bool FPCGCesiumHeightAdjusterElement::ExecuteInternal(FPCGContext* Context) const
//{
//	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCesiumHeightAdjusterElement::Execute);
//	check(Context);
//
//	if (!Context || !Context->SourceComponent.IsValid())
//	{
//		UE_LOG(LogTemp, Error, TEXT("Invalid PCG Context or Source Component."));
//		return true;
//	}
//
//	static TSet<uint64> RunningTasks;
//	static TSet<uint64> CompletedTasks;
//
//
//	const uint64 TaskId = Context->TaskId;
//
//	if (CompletedTasks.Contains(TaskId))
//	{
//		CompletedTasks.Empty();
//		return true;
//	}
//
//	if (RunningTasks.Contains(TaskId))
//	{
//		// Async work still in progress
//		return false;
//	}
//
//	// Start async sampling
//	RunningTasks.Add(TaskId);
//
//	UWorld* World = Context->SourceComponent->GetWorld();
//	ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(World);
//	ACesium3DTileset* Tileset = nullptr;
//
//	for (TActorIterator<ACesium3DTileset> It(World); It; ++It)
//	{
//		Tileset = *It;
//		break;
//	}
//
//	if (!Georef || !Tileset || !Tileset->GetTileset())
//	{
//		UE_LOG(LogTemp, Error, TEXT("Missing Cesium Georeference or Tileset"));
//		RunningTasks.Remove(TaskId);
//		CompletedTasks.Add(TaskId); // mark as complete to prevent retries
//		return true;
//	}
//
//	Cesium3DTilesSelection::Tileset* NativeTileset = Tileset->GetTileset();
//
//	// ✅ Wait until tileset finishes loading
//	if (!NativeTileset->getRootTile() ||
//		NativeTileset->getRootTile()->getState() != Cesium3DTilesSelection::TileLoadState::Done)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Waiting for Cesium root tile to finish loading..."));
//		RunningTasks.Remove(TaskId);
//		return false;
//	}
//
//
//
//
//	// Output point container
//
//	UPCGComponent* SourceComponent = Context->SourceComponent.Get();
//	UPCGPointData* OutputData = NewObject<UPCGPointData>(SourceComponent);
//
////	UPCGPointData* OutputData = NewObject<UPCGPointData>(GetTransientPackage());
//	TSharedRef<int32> Remaining = MakeShared<int32>(0);
//	int32 TotalQueued = 0;
//
//	for (const FPCGTaggedData& Input : Context->InputData.TaggedData)
//	{
//		const UPCGSpatialData* Spatial = Cast<UPCGSpatialData>(Input.Data);
//		const UPCGPointData* PointData = Spatial ? Spatial->ToPointData(Context) : nullptr;
//		if (!PointData) continue;
//
//		const TArray<FPCGPoint>& Points = PointData->GetPoints();
//		TArray<FPCGPoint> LocalCopy = Points;
//
//		std::vector<CesiumGeospatial::Cartographic> CesiumCoords;
//		for (const FPCGPoint& Pt : LocalCopy)
//		{
//			FVector LLH = Georef->TransformUnrealPositionToLongitudeLatitudeHeight(Pt.Transform.GetLocation());
//			CesiumCoords.emplace_back(
//				CesiumUtility::Math::degreesToRadians(LLH.X),
//				CesiumUtility::Math::degreesToRadians(LLH.Y),
//				0.0);
//		}
//
//		(*Remaining)++;
//		TotalQueued += Points.Num();
//
//		NativeTileset->sampleHeightMostDetailed(CesiumCoords).thenInMainThread(
//			[Georef, LocalCopy, OutputData, Remaining, Context, TaskId](const Cesium3DTilesSelection::SampleHeightResult& Result) mutable
//			{
//				if (!OutputData) return;
//
//				TArray<FPCGPoint>& PointsOut = OutputData->GetMutablePoints();
//
//				for (int32 i = 0; i < LocalCopy.Num(); ++i)
//				{
//					FPCGPoint P = LocalCopy[i];
//					if (Result.sampleSuccess[i])
//					{
//						const auto& Carto = Result.positions[i];
//						FVector LLH = FVector(
//							FMath::RadiansToDegrees(Carto.longitude),
//							FMath::RadiansToDegrees(Carto.latitude),
//							Carto.height);
//						P.Transform.SetLocation(Georef->TransformLongitudeLatitudeHeightPositionToUnreal(LLH));
//					}
//					PointsOut.Add(P);
//				}
//
//				if (--(*Remaining) == 0)
//				{
//					// All height sampling is complete
//					FPCGTaggedData& Output = Context->OutputData.TaggedData.AddDefaulted_GetRef();
//					Output.Data = OutputData;
//					Output.Tags = Context->InputData.TaggedData[0].Tags;
//
//					UE_LOG(LogTemp, Log, TEXT("Cesium async complete — %d points updated."), PointsOut.Num());
//
//					RunningTasks.Remove(TaskId);
//					CompletedTasks.Add(TaskId);
//				}
//			}
//		);
//	}
//
//	UE_LOG(LogTemp, Log, TEXT("Cesium PCG async sampling started for %d points."), TotalQueued);
//	return false; // Still processing
//}

//ray cast
//FPCGCesiumHeightAdjusterElement::ExecuteInternal(FPCGContext* Context) const
//{
//	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCesiumHeightAdjusterElement::Execute);
//	check(Context);
//
//	if (!Context || !Context->SourceComponent.Get())
//	{
//		UE_LOG(LogTemp, Error, TEXT("PCG Context or Source Component is NULL!"));
//		return false;
//	}
//
//	UPCGComponent* SourceComponent = Context->SourceComponent.Get();
//	if (!SourceComponent)
//	{
//		UE_LOG(LogTemp, Error, TEXT("PCG Source Component is NULL!"));
//		return false;
//	}
//
//
//	// Retrieve PCG Settings
//	const UPCGCesiumHeightAdjusterSettings* Settings = Context->GetInputSettings<UPCGCesiumHeightAdjusterSettings>();
//	if (!Settings)
//	{
//		UE_LOG(LogTemp, Error, TEXT("PCG Settings are NULL!"));
//		return false;
//	}
//
//	UWorld* World = Context->SourceComponent->GetWorld();
//	if (!World)
//	{
//		UE_LOG(LogTemp, Error, TEXT("World reference is NULL!"));
//		return false;
//	}
//
//
//	int32 TotalPointsModified = 0;
//	bool bAnyHit = false;
//
//	TArray<FVector> Locations;
//
//	// Iterate over PCG Point Data
//	for (FPCGTaggedData& InputData : Context->InputData.TaggedData)
//	{
//		const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(InputData.Data);
//		if (!SpatialData)
//			continue;
//
//		const UPCGPointData* PointData = SpatialData->ToPointData(Context);
//		if (!PointData)
//			continue;
//
//		UPCGPointData* MutablePointData = NewObject<UPCGPointData>(GetTransientPackage());
//
//		if (!MutablePointData)
//		{
//			UE_LOG(LogTemp, Error, TEXT("Failed to create MutablePointData!"));
//			continue;
//		}
//
//		MutablePointData->AddToRoot();
//
//		MutablePointData->InitializeFromData(PointData);
//
//
//		TArray<FPCGPoint> Points = PointData->GetPoints();
//
//
//		ACesiumGeoreference* Georeference = ACesiumGeoreference::GetDefaultGeoreference(World);
//
//		UE_LOG(LogTemp, Warning, TEXT("Total Input Points: %d"), Points.Num());
//
//
//		for (FPCGPoint& Point : Points)
//		{
//			FVector WorldLocation = Point.Transform.GetLocation();
//			FVector LongitudeLatitudeHeight = Georeference->TransformUnrealPositionToLongitudeLatitudeHeight(WorldLocation);
//			Locations.Add(LongitudeLatitudeHeight); // Add geodetic location to the array
//
//		}
//
//
//		UE_LOG(LogTemp, Warning, TEXT("LongitudeLatitudeHeightArray Size: %d"), Locations.Num());
//
//
//
//
//
//
//		//Ray cast approach
//
//		for (FPCGPoint& Point : Points)
//		{
//			FVector Start = Point.Transform.GetLocation() + FVector(0, 0, Settings->MaxHeightTrace);
//			FVector End = Point.Transform.GetLocation() + FVector(0, 0, Settings->MinHeightTrace);
//
//
//			UE_LOG(LogTemp, Warning, TEXT("Before Adjustment: %s"), *Point.Transform.GetLocation().ToString());
//
//			FHitResult HitResult;
//			FCollisionQueryParams Params;
//			Params.AddIgnoredActor(Context->SourceComponent->GetOwner());
//			if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, Params))
//			{
//				Point.Transform.SetLocation(HitResult.Location);
//
//				bAnyHit = true; // At least one raycast succeeded
//				UE_LOG(LogTemp, Warning, TEXT("After Adjustment: %s"), *Point.Transform.GetLocation().ToString());
//				TotalPointsModified++;
//
//			}
//
//			else {
//
//			}
//		}
//
//		MutablePointData->GetMutablePoints() = Points;
//		FPCGTaggedData& OutputData = Context->OutputData.TaggedData.Add_GetRef(InputData);
//		OutputData.Data = MutablePointData;
//
//
//	}
//
//
//	UE_LOG(LogTemp, Warning, TEXT("Total Points Modified: %d"), TotalPointsModified);
//
//
//	// If no raycasts succeeded, schedule a retry
//	if (!bAnyHit)
//	{
//		// UE_LOG(LogTemp, Warning, TEXT("No raycasts hit terrain, retrying in 3 seconds..."));
//
//
//		if (SourceComponent)
//		{
//			FTimerHandle RetryTimer;
//			World->GetTimerManager().SetTimer(
//				RetryTimer,
//				[SourceComponent]()
//				{
//
//					SourceComponent->Generate();
//
//				},
//				3.0f,
//				false // Run only once
//			);
//		}
//
//
//	}
//
//	return true;
//}

//kamran
//bool FPCGCesiumHeightAdjusterElement::ExecuteInternal(FPCGContext* Context) const
//{
//    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCesiumHeightAdjusterElement::Execute);
//    check(Context);
//
//    if (!Context || !Context->SourceComponent.IsValid())
//    {
//        UE_LOG(LogTemp, Error, TEXT("Invalid PCG Context or Source Component."));
//        return true;
//    }
//
//    static TSet<uint64> RunningTasks;
//    static TSet<uint64> CompletedTasks;
//
//    const uint64 TaskId = Context->TaskId;
//
//    if (CompletedTasks.Contains(TaskId))
//    {
//        CompletedTasks.Empty();
//        return true;
//    }
//
//    if (RunningTasks.Contains(TaskId))
//    {
//        return false; // Still processing
//    }
//
//    RunningTasks.Add(TaskId);
//
//    UWorld* World = Context->SourceComponent->GetWorld();
//    ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(World);
//    ACesium3DTileset* Tileset = nullptr;
//
//    for (TActorIterator<ACesium3DTileset> It(World); It; ++It)
//    {
//        Tileset = *It;
//        break;
//    }
//
//    if (!IsValid(Georef) || !IsValid(Tileset) || !Tileset->GetTileset())
//    {
//        UE_LOG(LogTemp, Error, TEXT("Missing Cesium Georeference or Tileset."));
//        RunningTasks.Remove(TaskId);
//        CompletedTasks.Add(TaskId);
//        return true;
//    }
//
//    Tileset->ResolveGeoreference();
//    Tileset->ResolveCameraManager();
//    Tileset->ResolveCreditSystem();
//
//
//    Cesium3DTilesSelection::Tileset* NativeTileset = Tileset->GetTileset();
//
//    if (!NativeTileset->getRootTile() ||
//        NativeTileset->getRootTile()->getState() != Cesium3DTilesSelection::TileLoadState::Done)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("Waiting for Cesium root tile to load..."));
//        RunningTasks.Remove(TaskId);
//        return false;
//    }
//
//    TSharedRef<int32> Remaining = MakeShared<int32>(0);
//    int32 TotalQueued = 0;
//
//    FPCGContext* RawContext = Context;
//    TWeakObjectPtr<ACesiumGeoreference> WeakGeoref(Georef);
//
//    for (const FPCGTaggedData& Input : Context->InputData.TaggedData)
//    {
//        const UPCGSpatialData* Spatial = Cast<UPCGSpatialData>(Input.Data);
//        const UPCGPointData* PointData = Spatial ? Spatial->ToPointData(Context) : nullptr;
//        if (!PointData) continue;
//
//        const TArray<FPCGPoint>& Points = PointData->GetPoints();
//        if (Points.IsEmpty()) continue;
//
//        TArray<FPCGPoint> LocalCopy = Points;
//
//        std::vector<CesiumGeospatial::Cartographic> CesiumCoords;
//        CesiumCoords.reserve(LocalCopy.Num());
//
//        for (const FPCGPoint& Pt : LocalCopy)
//        {
//            FVector LLH = Georef->TransformUnrealPositionToLongitudeLatitudeHeight(Pt.Transform.GetLocation());
//            CesiumCoords.emplace_back(
//                CesiumUtility::Math::degreesToRadians(LLH.X),
//                CesiumUtility::Math::degreesToRadians(LLH.Y),
//                0.0);
//        }
//
//        UPCGPointData* OutputData = NewObject<UPCGPointData>(Context->SourceComponent.Get());
//        TWeakObjectPtr<UPCGPointData> WeakOutputData(OutputData);
//
//        (*Remaining)++;
//        TotalQueued += Points.Num();
//
//        NativeTileset->sampleHeightMostDetailed(CesiumCoords).thenInMainThread(
//            [LocalCopy, WeakGeoref, WeakOutputData, RawContext, Remaining, TaskId](const Cesium3DTilesSelection::SampleHeightResult& Result) mutable
//            {
//                UPCGPointData* OutputData = WeakOutputData.Get();
//                ACesiumGeoreference* Georef = WeakGeoref.Get();
//
//                if (!RawContext || !RawContext->SourceComponent.IsValid() || !Georef || !OutputData)
//                {
//                    UE_LOG(LogTemp, Error, TEXT("Async callback: Invalid state — aborting."));
//                    return;
//                }
//
//                const int32 NumRequested = LocalCopy.Num();
//                const int32 NumReturned = static_cast<int32>(Result.sampleSuccess.size());
//
//                if (NumReturned != NumRequested || Result.positions.size() != NumRequested)
//                {
//                    UE_LOG(LogTemp, Error, TEXT("Mismatch in sample result count: Expected %d, Got %d."), NumRequested, NumReturned);
//                    return;
//                }
//
//                TArray<FPCGPoint> UpdatedPoints;
//                UpdatedPoints.Reserve(NumRequested);
//
//                for (int32 i = 0; i < NumRequested; ++i)
//                {
//                    FPCGPoint P = LocalCopy[i];
//
//                    if (Result.sampleSuccess[i])
//                    {
//                        const auto& Carto = Result.positions[i];
//                        FVector LLH(
//                            FMath::RadiansToDegrees(Carto.longitude),
//                            FMath::RadiansToDegrees(Carto.latitude),
//                            Carto.height);
//
//                        P.Transform.SetLocation(Georef->TransformLongitudeLatitudeHeightPositionToUnreal(LLH));
//                    }
//
//                    UpdatedPoints.Add(P);
//                }
//
//                OutputData->GetMutablePoints() = MoveTemp(UpdatedPoints);
//
//                if (--(*Remaining) == 0)
//                {
//                    FPCGTaggedData& Output = RawContext->OutputData.TaggedData.AddDefaulted_GetRef();
//                    Output.Data = OutputData;
//                    Output.Tags = RawContext->InputData.TaggedData[0].Tags;
//
//                    UE_LOG(LogTemp, Log, TEXT("Cesium async complete — %d points updated."), OutputData->GetPoints().Num());
//
//                    RunningTasks.Remove(TaskId);
//                    CompletedTasks.Add(TaskId);
//                }
//            }
//        );
//    }
//
//    UE_LOG(LogTemp, Log, TEXT("Cesium PCG async sampling started for %d points."), TotalQueued);
//    return false;
//}


//last working on 5.4
bool FPCGCesiumHeightAdjusterElement::ExecuteInternal(FPCGContext* Context) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCesiumHeightAdjusterElement::Execute);
    check(Context);

    if (!Context || !Context->SourceComponent.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid PCG Context or Source Component."));
        return true;
    }

    static TSet<uint64> RunningTasks;
    static TSet<uint64> CompletedTasks;
    static bool bfirstRun = true;

    static FCriticalSection TaskMutex;

    if (bfirstRun) {
        FScopeLock Lock(&TaskMutex);
        RunningTasks.Empty();
        CompletedTasks.Empty();
        bfirstRun = false;


    }

    const uint64 TaskId = Context->TaskId;

    {

        FScopeLock Lock(&TaskMutex);

        if (CompletedTasks.Contains(TaskId))
        {
            return true;
        }

        if (RunningTasks.Contains(TaskId))
        {
            return false; // Still processing
        }

        RunningTasks.Add(TaskId);

    }


    UObject* SourceComponentObj = Context->SourceComponent.Get();
    if (!SourceComponentObj)
    {
        UE_LOG(LogTemp, Error, TEXT("Null SourceComponent object."));
        RunningTasks.Remove(TaskId);
        CompletedTasks.Add(TaskId);
        return true;
    }

    UWorld* World = SourceComponentObj->GetWorld();
    ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(World);
    ACesium3DTileset* Tileset = nullptr;

    for (TActorIterator<ACesium3DTileset> It(World); It; ++It)
    {
        Tileset = *It;
        break;
    }

    if (!IsValid(Georef) || !IsValid(Tileset) || !Tileset->GetTileset())
    {
        UE_LOG(LogTemp, Error, TEXT("Missing Cesium Georeference or Tileset."));
        RunningTasks.Remove(TaskId);
        CompletedTasks.Add(TaskId);
        return true;
    }

    Tileset->ResolveGeoreference();
    Tileset->ResolveCameraManager();
    Tileset->ResolveCreditSystem();

    Cesium3DTilesSelection::Tileset* NativeTileset = Tileset->GetTileset();

    if (!NativeTileset->getRootTile() ||
        NativeTileset->getRootTile()->getState() != Cesium3DTilesSelection::TileLoadState::Done)
    {
        UE_LOG(LogTemp, Warning, TEXT("Waiting for Cesium root tile to load..."));
        RunningTasks.Remove(TaskId);
        return false;
    }

    TSharedRef<int32> Remaining = MakeShared<int32>(0);
    int32 TotalQueued = 0;

    for (const FPCGTaggedData& Input : Context->InputData.TaggedData)
    {
        const UPCGSpatialData* Spatial = Cast<UPCGSpatialData>(Input.Data);
        const UPCGPointData* PointData = Spatial ? Spatial->ToPointData(Context) : nullptr;
        if (!PointData) continue;

        const TArray<FPCGPoint>& Points = PointData->GetPoints();

        //if (Points.Num() > 10000) {
        //    UE_LOG(LogTemp, Warning, TEXT("Polygon  has points %d , skipping."), Points.Num());
        //    return true;

        //};


        if (Points.IsEmpty()) continue;

        TArray<FPCGPoint> LocalCopy = Points;
        if (LocalCopy.Num() > 10000)
        {
            UObject* OwnerObject = Context->SourceComponent->GetOwner();
            if (const APCGPolygonActor* PolygonActor = Cast<APCGPolygonActor>(OwnerObject)) {
                UE_LOG(LogTemp, Warning, TEXT("Skipping Actor: %s Points: %d "), *OwnerObject->GetName(), LocalCopy.Num());




            }
            continue;
        }


        std::vector<CesiumGeospatial::Cartographic> CesiumCoords;
        CesiumCoords.reserve(LocalCopy.Num());

        for (const FPCGPoint& Pt : LocalCopy)
        {
            FVector LLH = Georef->TransformUnrealPositionToLongitudeLatitudeHeight(Pt.Transform.GetLocation());
            CesiumCoords.emplace_back(
                CesiumUtility::Math::degreesToRadians(LLH.X),
                CesiumUtility::Math::degreesToRadians(LLH.Y),
                0.0);
        }

        UPCGPointData* OutputData = NewObject<UPCGPointData>(SourceComponentObj);
        check(OutputData);

        TWeakObjectPtr<UPCGPointData> WeakOutputData(OutputData);
        TWeakObjectPtr<UObject> WeakSourceComponent(SourceComponentObj);
        TWeakObjectPtr<ACesiumGeoreference> WeakGeoref(Georef);

        (*Remaining)++;
        TotalQueued += Points.Num();



        NativeTileset->sampleHeightMostDetailed(CesiumCoords).thenInMainThread(
            [LocalCopy, WeakOutputData, WeakGeoref, WeakSourceComponent, Remaining, TaskId, Tags = Input.Tags, Context](const Cesium3DTilesSelection::SampleHeightResult& Result) mutable
            {
                if (!WeakOutputData.IsValid() || !WeakGeoref.IsValid() || !WeakSourceComponent.IsValid())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Async callback skipped due to invalid references."));

                    if (--(*Remaining) == 0)
                    {
                        AsyncTask(ENamedThreads::GameThread, [TaskId]()
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Finalizing Cesium PCG task after skipped callbacks."));
                                RunningTasks.Remove(TaskId);
                                CompletedTasks.Add(TaskId);
                            });
                    }
                    return;
                }

                ACesiumGeoreference* Georef = WeakGeoref.Get();
                UPCGPointData* OutputData = WeakOutputData.Get();

                const int32 NumRequested = LocalCopy.Num();
                const int32 NumReturned = static_cast<int32>(Result.sampleSuccess.size());

                if (NumReturned != NumRequested || Result.positions.size() != NumRequested)
                {
                    UE_LOG(LogTemp, Error, TEXT("Mismatch in sample result count: Expected %d, Got %d."), NumRequested, NumReturned);

                    if (--(*Remaining) == 0)
                    {
                        AsyncTask(ENamedThreads::GameThread, [TaskId]()
                            {
                                RunningTasks.Remove(TaskId);
                                CompletedTasks.Add(TaskId);
                            });
                    }
                    return;
                }


                
                TArray<FPCGPoint> UpdatedPoints;
                UpdatedPoints.Reserve(NumRequested);

                for (int32 i = 0; i < NumRequested; ++i)
                {
                    FPCGPoint P = LocalCopy[i];

                    if (Result.sampleSuccess[i])
                    {
                        const auto& Carto = Result.positions[i];
                        FVector LLH(
                            FMath::RadiansToDegrees(Carto.longitude),
                            FMath::RadiansToDegrees(Carto.latitude),
                            Carto.height);

                        P.Transform.SetLocation(Georef->TransformLongitudeLatitudeHeightPositionToUnreal(LLH));
                    }

                    UpdatedPoints.Add(P);
                }

                OutputData->GetMutablePoints() = MoveTemp(UpdatedPoints);

                if (--(*Remaining) == 0)
                {
                    AsyncTask(ENamedThreads::GameThread, [Context, OutputData, Tags, TaskId]()
                        {
                            if (!Context || !Context->SourceComponent.IsValid())
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Context invalid or destroyed when writing output."));
                                RunningTasks.Remove(TaskId);
                                CompletedTasks.Add(TaskId);
                                return;
                            }

                            FPCGTaggedData& Output = Context->OutputData.TaggedData.AddDefaulted_GetRef();
                            Output.Data = OutputData;
                            Output.Tags = Tags;

                            int32 NumUpdatePoints = OutputData->GetPoints().Num();



                            UE_LOG(LogTemp, Log, TEXT("Cesium async complete — %d points updated."), NumUpdatePoints);

                            RunningTasks.Remove(TaskId);
                            CompletedTasks.Add(TaskId);
                        });
                }


            });




    }

    if (TotalQueued == 0) {

        RunningTasks.Remove(TaskId);
        CompletedTasks.Add(TaskId);

        RunningTasks.Empty();
        CompletedTasks.Empty();

        return true;

    }


    UE_LOG(LogTemp, Log, TEXT("Cesium PCG async sampling started for %d points."), TotalQueued);
    return false; // Async in progress
}



//5.6:

//bool FPCGCesiumHeightAdjusterElement::ExecuteInternal(FPCGContext* Context) const
//{
//    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGCesiumHeightAdjusterElement::Execute);
//    check(Context);
//
//    if (!Context || !Context->SourceComponent.IsValid())
//    {
//        UE_LOG(LogTemp, Error, TEXT("Invalid PCG Context or Source Component."));
//        return true;
//    }
//
//    // --- Shared bookkeeping & plain-result storage ---
//    struct FPlainResult
//    {
//        TArray<FPCGPoint> Points;
//        TSet<FString> Tags;
//    };
//
//    static TSet<uint64> RunningTasks;
//    static TSet<uint64> CompletedTasks;
//    static TMap<uint64, TArray<FPlainResult>> CompletedOutputsPlain;
//    static FCriticalSection TaskMutex;
//
//    const uint64 TaskId = Context->TaskId;
//
//    // 1) Consume completed POD results if available
//    {
//        FScopeLock Lock(&TaskMutex);
//        if (TArray<FPlainResult>* Found = CompletedOutputsPlain.Find(TaskId))
//        {
//            TArray<FPlainResult> Results = MoveTemp(*Found);
//            CompletedOutputsPlain.Remove(TaskId);
//            RunningTasks.Remove(TaskId);
//            CompletedTasks.Add(TaskId);
//
//            for (FPlainResult& R : Results)
//            {
//                UObject* SourceComponentObj = Context->SourceComponent.Get();
//                if (!SourceComponentObj) continue;
//
//                UPCGPointData* OutputData = NewObject<UPCGPointData>(SourceComponentObj);
//                check(OutputData);
//
//                OutputData->GetMutablePoints() = MoveTemp(R.Points);
//
//                FPCGTaggedData& OutTagged = Context->OutputData.TaggedData.AddDefaulted_GetRef();
//                OutTagged.Data = OutputData;
//
//                // Tags are TSet<FString> now, safe to assign
//                OutTagged.Tags = MoveTemp(R.Tags);
//            }
//
//            UE_LOG(LogTemp, Log, TEXT("Cesium PCG: consumed %d plain outputs for TaskId %llu"), Results.Num(), TaskId);
//            return true;
//        }
//
//        if (CompletedTasks.Contains(TaskId)) return true;
//        if (RunningTasks.Contains(TaskId)) return false;
//
//        RunningTasks.Add(TaskId);
//    }
//
//    // 2) Build POD copies of inputs
//    struct FInputPack
//    {
//        TArray<FPCGPoint> Points;
//        TSet<FString> Tags; // <-- 5.6
//    };
//
//    TArray<TSharedRef<FInputPack>> WorkPacks;
//    WorkPacks.Reserve(Context->InputData.TaggedData.Num());
//
//    for (const FPCGTaggedData& Input : Context->InputData.TaggedData)
//    {
//        const UPCGSpatialData* Spatial = Cast<UPCGSpatialData>(Input.Data);
//        const UPCGPointData* PointData = Spatial ? Spatial->ToPointData(Context) : nullptr;
//        if (!PointData) continue;
//
//        const TArray<FPCGPoint>& Points = PointData->GetPoints();
//        if (Points.IsEmpty()) continue;
//
//        TArray<FPCGPoint> LocalCopy = Points;
//        if (LocalCopy.Num() > 10000)
//        {
//            if (UObject* Owner = Context->SourceComponent->GetOwner())
//            {
//                UE_LOG(LogTemp, Warning, TEXT("Skipping Actor: %s Points: %d "), *Owner->GetName(), LocalCopy.Num());
//            }
//            continue;
//        }
//
//        TSharedRef<FInputPack> Pack = MakeShared<FInputPack>();
//        Pack->Points = MoveTemp(LocalCopy);
//        Pack->Tags.Empty();
//        Pack->Tags = Input.Tags;
//        WorkPacks.Add(Pack);
//    }
//
//    if (WorkPacks.Num() == 0)
//    {
//        FScopeLock Lock(&TaskMutex);
//        RunningTasks.Remove(TaskId);
//        CompletedTasks.Add(TaskId);
//        CompletedOutputsPlain.Remove(TaskId);
//        return true;
//    }
//
//    UE_LOG(LogTemp, Log, TEXT("Cesium PCG async sampling scheduled for %d packs (Task %llu)."), WorkPacks.Num(), TaskId);
//
//    TSharedRef<int32> Remaining = MakeShared<int32>(0);
//    *Remaining = 0;
//    TArray<TSharedRef<FInputPack>> LocalWorkPacks = WorkPacks;
//
//    // 3) Schedule Game Thread work
//    AsyncTask(ENamedThreads::GameThread, [this, TaskId, WeakSourceComponent = Context->SourceComponent, LocalWorkPacks = MoveTemp(LocalWorkPacks), Remaining]()
//        {
//            UObject* SourceComponentObj = WeakSourceComponent.IsValid() ? WeakSourceComponent.Get() : nullptr;
//            if (!SourceComponentObj)
//            {
//                UE_LOG(LogTemp, Warning, TEXT("Cesium PCG: SourceComponent invalid on GameThread for Task %llu"), TaskId);
//                FScopeLock Lock(&TaskMutex);
//                RunningTasks.Remove(TaskId);
//                CompletedTasks.Add(TaskId);
//                return;
//            }
//
//            UWorld* World = SourceComponentObj->GetWorld();
//            if (!World)
//            {
//                UE_LOG(LogTemp, Warning, TEXT("Cesium PCG: World invalid on GameThread for Task %llu"), TaskId);
//                FScopeLock Lock(&TaskMutex);
//                RunningTasks.Remove(TaskId);
//                CompletedTasks.Add(TaskId);
//                return;
//            }
//
//            ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(World);
//            ACesium3DTileset* Tileset = nullptr;
//            for (TActorIterator<ACesium3DTileset> It(World); It; ++It) { Tileset = *It; break; }
//
//            if (!IsValid(Georef) || !IsValid(Tileset) || !Tileset->GetTileset())
//            {
//                UE_LOG(LogTemp, Warning, TEXT("Cesium PCG: Missing georef/tileset for Task %llu"), TaskId);
//                FScopeLock Lock(&TaskMutex);
//                RunningTasks.Remove(TaskId);
//                CompletedTasks.Add(TaskId);
//                return;
//            }
//
//            Tileset->ResolveGeoreference();
//            Tileset->ResolveCameraManager();
//            Tileset->ResolveCreditSystem();
//
//            Cesium3DTilesSelection::Tileset* NativeTileset = Tileset->GetTileset();
//            if (!NativeTileset->getRootTile() || NativeTileset->getRootTile()->getState() != Cesium3DTilesSelection::TileLoadState::Done)
//            {
//                UE_LOG(LogTemp, Warning, TEXT("Cesium PCG: Tileset root not ready for Task %llu"), TaskId);
//                FScopeLock Lock(&TaskMutex);
//                RunningTasks.Remove(TaskId);
//                return;
//            }
//
//            *Remaining = LocalWorkPacks.Num();
//
//            for (const TSharedRef<FInputPack>& PackRef : LocalWorkPacks)
//            {
//                const TArray<FPCGPoint>& LocalPoints = PackRef->Points;
//                const int32 NumPoints = LocalPoints.Num();
//                std::vector<CesiumGeospatial::Cartographic> CesiumCoords;
//                CesiumCoords.reserve(NumPoints);
//
//                for (const FPCGPoint& Pt : LocalPoints)
//                {
//                    FVector LLH = Georef->TransformUnrealPositionToLongitudeLatitudeHeight(Pt.Transform.GetLocation());
//                    CesiumCoords.emplace_back(
//                        CesiumUtility::Math::degreesToRadians(LLH.X),
//                        CesiumUtility::Math::degreesToRadians(LLH.Y),
//                        0.0
//                    );
//                }
//
//                NativeTileset->sampleHeightMostDetailed(CesiumCoords).thenInMainThread(
//                    [PackRef, WeakSourceComponent, Georef, Remaining, TaskId](const Cesium3DTilesSelection::SampleHeightResult& Result) mutable
//                    {
//                        UObject* SourceCompObj = WeakSourceComponent.IsValid() ? WeakSourceComponent.Get() : nullptr;
//                        if (!SourceCompObj || !Georef)
//                        {
//                            if (--(*Remaining) == 0)
//                            {
//                                FScopeLock Lock(&TaskMutex);
//                                CompletedOutputsPlain.FindOrAdd(TaskId);
//                            }
//                            return;
//                        }
//
//                        const int32 NumRequested = PackRef->Points.Num();
//                        const int32 NumReturned = static_cast<int32>(Result.sampleSuccess.size());
//                        if (NumReturned != NumRequested || static_cast<int32>(Result.positions.size()) != NumRequested)
//                        {
//                            if (--(*Remaining) == 0)
//                            {
//                                FScopeLock Lock(&TaskMutex);
//                                CompletedOutputsPlain.FindOrAdd(TaskId);
//                            }
//                            return;
//                        }
//
//                        FPlainResult Plain;
//                        Plain.Points.Reserve(NumRequested);
//
//                        for (int32 i = 0; i < NumRequested; ++i)
//                        {
//                            FPCGPoint NewP = PackRef->Points[i];
//                            if (Result.sampleSuccess[i])
//                            {
//                                const auto& Carto = Result.positions[i];
//                                FVector LLH(
//                                    FMath::RadiansToDegrees(Carto.longitude),
//                                    FMath::RadiansToDegrees(Carto.latitude),
//                                    Carto.height
//                                );
//                                NewP.Transform.SetLocation(Georef->TransformLongitudeLatitudeHeightPositionToUnreal(LLH));
//                            }
//                            Plain.Points.Add(MoveTemp(NewP));
//                        }
//
//                        Plain.Tags = PackRef->Tags; // Already TSet<FString>
//
//                        FScopeLock Lock(&TaskMutex);
//                        TArray<FPlainResult>& Arr = CompletedOutputsPlain.FindOrAdd(TaskId);
//                        Arr.Add(MoveTemp(Plain));
//
//                        if (--(*Remaining) == 0)
//                        {
//                            UE_LOG(LogTemp, Log, TEXT("Cesium PCG: All sampling callbacks complete for Task %llu (POD outputs stored)."), TaskId);
//                        }
//                    });
//            }
//        });
//
//    return false; // Async in progress
//}
