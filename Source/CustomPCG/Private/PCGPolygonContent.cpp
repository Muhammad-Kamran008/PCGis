#include "PCGPolygonContent.h"
#include "PCGGraph.h"
#include "TimerManager.h"
#include "Data/PCGPointData.h"
#include "CesiumGeoreference.h"
#undef OPAQUE
#include "Cesium3DTileset.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"    
#include <SQLiteDatabase.h>


void APCGPolygonContent::Tick(float DeltaTime) {

    Super::Tick(DeltaTime);

}

APCGPolygonContent::APCGPolygonContent()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void APCGPolygonContent::BeginPlay() {
    Super::BeginPlay();

}

void APCGPolygonContent::InitializeContent()
{
    CesiumGeoreference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
    Tileset = Cast<ACesium3DTileset>(UGameplayStatics::GetActorOfClass(GetWorld(), ACesium3DTileset::StaticClass()));

    GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    DBSubsystem = GI->GetSubsystem<UPcgSQLiteSubsystem>();
}

void APCGPolygonContent::InitializePCGPolygonData(const FShapeRawData& Raw)
{
    //InitializeContent();

    for (int32 i = 0; i < Raw.Attributes.Num(); ++i)
    {
        const auto& A = Raw.Attributes[i];
        FGrassPolygonData temp;


        auto GetSafeString = [&](const FString& key) ->FString {
            return A.Contains(key) ? A[key] : TEXT("");
            };


        auto GetSafeInt = [&](const FString& key) -> int32 {
            return A.Contains(key) ? FCString::Atoi(*A[key]) : 0;

            };


        auto GetSafeFloat = [&](const FString& key) -> float {
            return A.Contains(key) ? FCString::Atof(*A[key]) : 0.0f;

            };

        temp.Id = GetSafeInt("id");
        temp.Name = A["Name"];
        temp.Type = GetSafeString("Type");
        temp.Density = GetSafeFloat("Density");
        temp.Scale = GetSafeFloat("Scale");
        temp.Model = GetSafeString("Model");
        temp.FileName = Raw.CollectionName;
        temp.State = GetSafeString("State");
        temp.KindID = GetSafeInt("KindID");
        temp.KindDesc = GetSafeString("KindDesc");
        temp.DomainID = GetSafeInt("DomainID");
        temp.DomainDesc = GetSafeString("DomainDesc");
        temp.CountryID = GetSafeInt("CountryID");
        temp.CountryDes = GetSafeString("CountryDes");
        temp.CategoryID = GetSafeInt("Alpha");
        temp.CategoryDe = GetSafeString("CategoryDe");
        temp.SubCategID = GetSafeInt("SubCategID");
        temp.SubCategDe = GetSafeString("SubCategDe");
        temp.SpecificID = GetSafeInt("SpecificID");
        temp.SpecificDe = GetSafeString("SpecificDe");
        temp.Foliage = GetSafeString("Foliage");
        temp.EntityEnum = A.Contains("EntityEnum") ? A["EntityEnum"] : TEXT("");
        temp.PolygonPoints = Raw.Geometries[i];
        temp.Area = GetSafeFloat("Area");
        temp.Pnts = GetSafeFloat("pnts");

        PolygonDataList.Add(temp);
    }
}

bool APCGPolygonContent::InsertPolygonsFeaturesToDB(const FString& ShapefileID)
{
    if (PolygonDataList.Num() == 0)
        return false;

    if (!GI) return false;
    if (!DBSubsystem || !DBSubsystem->IsOpen())
    {
        UE_LOG(LogTemp, Error, TEXT("InsertPolygonFeaturesToDB: DB subsystem not available/open"));
        return false;
    }

    DBSubsystem->BeginTransaction();

    for (const FGrassPolygonData& Data : PolygonDataList)
    {
        FString SQL = FString::Format(
            TEXT("INSERT OR REPLACE INTO PolygonFeatures "
                "(ShapefileID, PolygonID, Name, Type, Scale, Model, State, Foliage, Density, Area, Pnts, "
                "KindID, KindDesc, DomainID, DomainDesc, CountryID, CountryDes, CategoryID, CategoryDe, "
                "SubCategID, SubCategDe, SpecificID, SpecificDe, EntityEnum, BoxExtentX, BoxExtentY, BoxExtentZ) "
                "VALUES ('{0}', {1}, '{2}', '{3}', {4}, '{5}', '{6}', '{7}', {8}, {9}, {10}, {11}, '{12}', "
                "{13}, '{14}', {15}, '{16}', {17}, '{18}', {19}, '{20}', {21}, '{22}', '{23}', {24}, {25}, {26});"),
            {
                *ShapefileID,
                Data.Id,
                *Data.Name,
                *Data.Type,
                Data.Scale,
                *Data.Model,
                *Data.State,
                *Data.Foliage,
                Data.Density,
                Data.Area,
                Data.Pnts,
                Data.KindID,
                *Data.KindDesc,
                Data.DomainID,
                *Data.DomainDesc,
                Data.CountryID,
                *Data.CountryDes,
                Data.CategoryID,
                *Data.CategoryDe,
                Data.SubCategID,
                *Data.SubCategDe,
                Data.SpecificID,
                *Data.SpecificDe,
                *Data.EntityEnum,
                Data.BoxExtents.X,
                Data.BoxExtents.Y,
                Data.BoxExtents.Z
            }
        );

        if (!DBSubsystem->Execute(SQL))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to insert polygon feature %d for shapefile %s"), Data.Id, *ShapefileID);
        }
    }

    DBSubsystem->CommitTransaction();

    UE_LOG(LogTemp, Log, TEXT("Inserted %d features for shapefile %s into PolygonFeatures table"), PolygonDataList.Num(), *ShapefileID);
    return true;
}

bool APCGPolygonContent::InsertPolygonFeaturesToDB(const FGrassPolygonData& Data)
{
 
    if (!GI) return false;
    if (!DBSubsystem || !DBSubsystem->IsOpen())
    {
        UE_LOG(LogTemp, Error, TEXT("InsertPolygonFeaturesToDB: DB subsystem not available/open"));
        return false;
    }

    DBSubsystem->BeginTransaction();

    FString SQL = FString::Format(
        TEXT("INSERT OR REPLACE INTO PolygonFeatures "
            "(ShapefileID, PolygonID, Name, Type, Scale, Model, State, Foliage, Density, Area, Pnts, "
            "KindID, KindDesc, DomainID, DomainDesc, CountryID, CountryDes, CategoryID, CategoryDe, "
            "SubCategID, SubCategDe, SpecificID, SpecificDe, EntityEnum, BoxExtentX, BoxExtentY, BoxExtentZ) "
            "VALUES ('{0}', {1}, '{2}', '{3}', {4}, '{5}', '{6}', '{7}', {8}, {9}, {10}, {11}, '{12}', "
            "{13}, '{14}', {15}, '{16}', {17}, '{18}', {19}, '{20}', {21}, '{22}', '{23}', {24}, {25}, {26});"),
        {
            *Data.FileName,
            Data.Id,
            *Data.Name,
            *Data.Type,
            Data.Scale,
            *Data.Model,
            *Data.State,
            *Data.Foliage,
            Data.Density,
            Data.Area,
            Data.Pnts,
            Data.KindID,
            *Data.KindDesc,
            Data.DomainID,
            *Data.DomainDesc,
            Data.CountryID,
            *Data.CountryDes,
            Data.CategoryID,
            *Data.CategoryDe,
            Data.SubCategID,
            *Data.SubCategDe,
            Data.SpecificID,
            *Data.SpecificDe,
            *Data.EntityEnum,
            Data.BoxExtents.X,
            Data.BoxExtents.Y,
            Data.BoxExtents.Z
        }
    );

    if (!DBSubsystem->Execute(SQL))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to insert polygon feature %d for shapefile %s"), Data.Id, *Data.FileName);
    }

    DBSubsystem->CommitTransaction();

    UE_LOG(LogTemp, Log, TEXT("Inserted %d features for shapefile %s and Polygon %d into PolygonFeatures table"), PolygonDataList.Num(), *Data.FileName, Data.Id);
    return true;
}

void APCGPolygonContent::SpawnPCGPolygonData()
{
    if (PolygonDataList.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No PCG polygon data available to spawn."));
        return;
    }

    FString BaseName = PolygonDataList[0].FileName;
    FName ActorName(BaseName);

    int Suffix = 0;

    while (FindObject<AActor>(nullptr, *ActorName.ToString())) {
        ++Suffix;
        ActorName = FName(*FString::Printf(TEXT("%s_%d"), *BaseName, Suffix));
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = ActorName;


    ParentActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    if (ParentActor)
    {

        USceneComponent* SceneRoot = NewObject<USceneComponent>(ParentActor);
        SceneRoot->SetMobility(EComponentMobility::Static);
        SceneRoot->RegisterComponent();
        ParentActor->SetRootComponent(SceneRoot);

#if WITH_EDITOR
        ParentActor->SetActorLabel(TEXT("PCG Polygon Parent"));
#endif
    }

    for (const FGrassPolygonData& Data : PolygonDataList)
    {
        SpawnIndividualPCGPolygonData(Data);
    }

}

static void ExtractFromHISMC(UHierarchicalInstancedStaticMeshComponent* ISMC, TArray<FBakedInstanceBatch>& Out)
{
    if (!ISMC || !ISMC->GetStaticMesh()) return;

    FBakedInstanceBatch Batch;
    Batch.MeshPath = FSoftObjectPath(ISMC->GetStaticMesh());

    const int32 MatCount = ISMC->GetNumMaterials();
    Batch.Materials.Reserve(MatCount);
    for (int32 i = 0; i < MatCount; ++i)
    {
        if (UMaterialInterface* M = ISMC->GetMaterial(i)) Batch.Materials.Add(FSoftObjectPath(M));
        else                                              Batch.Materials.Add(FSoftObjectPath());
    }

    const int32 N = ISMC->GetInstanceCount();
    Batch.Transforms.Reserve(N);
    for (int32 i = 0; i < N; ++i)
    {
        FTransform Xf;
        ISMC->GetInstanceTransform(i, Xf, /*bWorldSpace*/ true);
        Batch.Transforms.Add(Xf);
    }

    Out.Add(MoveTemp(Batch));
}

static void ExtractFromISMC(UInstancedStaticMeshComponent* ISMC, TArray<FBakedInstanceBatch>& Out)
{
    if (!ISMC || !ISMC->GetStaticMesh()) return;

    FBakedInstanceBatch Batch;
    Batch.MeshPath = FSoftObjectPath(ISMC->GetStaticMesh());

    const int32 MatCount = ISMC->GetNumMaterials();
    Batch.Materials.Reserve(MatCount);
    for (int32 i = 0; i < MatCount; ++i)
    {
        if (UMaterialInterface* M = ISMC->GetMaterial(i)) Batch.Materials.Add(FSoftObjectPath(M));
        else                                              Batch.Materials.Add(FSoftObjectPath());
    }

    const int32 N = ISMC->GetInstanceCount();
    Batch.Transforms.Reserve(N);
    for (int32 i = 0; i < N; ++i)
    {
        FTransform Xf;
        ISMC->GetInstanceTransform(i, Xf, /*bWorldSpace*/ true);
        Batch.Transforms.Add(Xf);
    }

    Out.Add(MoveTemp(Batch));
}

bool APCGPolygonContent::InsertPolygonPointsToDB(
    const FString& ShapefileID,
    FGrassPolygonData Data,
    const TArray<FTransform>& Instances,
    const FString& MeshID)
{
    if (!GI) return false;
    if (!DBSubsystem || !DBSubsystem->IsOpen())
    {
        UE_LOG(LogTemp, Error, TEXT("InsertPolygonPointsToDB: DB subsystem not available/open"));
        return false;
    }


    DBSubsystem->GetDatabase().Execute(TEXT("BEGIN TRANSACTION;"));
    for (int32 i = 0; i < Instances.Num(); ++i)
    {
        FVector Loc = Instances[i].GetLocation();

        FString SQL = FString::Printf(
            TEXT("INSERT INTO PolygonPoints (ShapefileID, PolygonID, PointIndex, X, Y, Z, MeshID) "
                "VALUES ('%s', %d, %d, %f, %f, %f, '%s');"),
            *ShapefileID, Data.Id, i, Loc.X, Loc.Y, Loc.Z, *MeshID);

        DBSubsystem->GetDatabase().Execute(*SQL);
    }
    DBSubsystem->GetDatabase().Execute(TEXT("COMMIT;"));

    return true;
}

void APCGPolygonContent::AssignPCGGraph(UPCGComponent* TargetPCG, const FString& GraphName)
{
    const FString GraphFolderPath = TEXT("/Game/PCGData/PolygonDataAssets");
    FString AssetName = FString::Printf(TEXT("PolygonData_%s"), *GraphName);
    FString AssetPath = FString::Printf(TEXT("%s/%s.%s"), *GraphFolderPath, *AssetName, *AssetName);

    UPCGGraph* Graph = Cast<UPCGGraph>(StaticLoadObject(UPCGGraph::StaticClass(), nullptr, *AssetPath));
    if (!Graph)
    {
        AssetPath = TEXT("/Game/PCGData/PolygonDataAssets/PolygonData_default.PolygonData_default");
        Graph = Cast<UPCGGraph>(StaticLoadObject(UPCGGraph::StaticClass(), nullptr, *AssetPath));
    }

    if (Graph)
    {
        TargetPCG->SetGraph(Graph);
        UE_LOG(LogTemp, Log, TEXT("Assigned PCG Graph: %s"), *GraphName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load PCG Graph: %s"), *GraphName);
    }
}

void APCGPolygonContent::SpawnIndividualPCGPolygonData(const FGrassPolygonData& Data)
{
    if (!Tileset || !CesiumGeoreference)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing Cesium references."));
        return;
    }

    const TArray<FVector>& Points = Data.PolygonPoints;
    if (Points.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No points found in polygon data."));
        return;
    }

    FCesiumSampleHeightMostDetailedCallback Callback;
    Callback.BindLambda([this, Points, Data](ACesium3DTileset* InTileset, const TArray<FCesiumSampleHeightResult>& SampledResults, const TArray<FString>& Warnings)
        {
            if (SampledResults.Num() != Points.Num())
            {
                UE_LOG(LogTemp, Error, TEXT("Sampled results do not match polygon point count."));
                return;
            }

            TArray<FVector> AdjustedPoints;
            for (int32 i = 0; i < SampledResults.Num(); ++i)
            {
                if (SampledResults[i].SampleSuccess) {
                    FVector UnrealPos = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(SampledResults[i].LongitudeLatitudeHeight);
                    AdjustedPoints.Add(UnrealPos);
                }
            }

            // Filter out duplicate points
            TArray<FVector> UniquePoints;
            for (const FVector& Pt : AdjustedPoints)
            {
                if (!UniquePoints.ContainsByPredicate([&](const FVector& Other)
                    { return FVector::DistSquared(Other, Pt) < KINDA_SMALL_NUMBER; }))
                {
                    UniquePoints.Add(Pt);
                }
            }

            if (UniquePoints.Num() < 3)
            {
                UE_LOG(LogTemp, Warning, TEXT("Polygon has fewer than 3 unique points, skipping."));
                return;
            }


            FVector MinPoint(FLT_MAX, FLT_MAX, FLT_MAX);
            FVector MaxPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);

            for (const FVector& Point : UniquePoints)
            {
                MinPoint = MinPoint.ComponentMin(Point);
                MaxPoint = MaxPoint.ComponentMax(Point);
            }

            FVector BoxExtents = (MaxPoint - MinPoint) * 0.5f;


            // Spawn APCGPolygonActor
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Name = FName(FString::Printf(TEXT("PCG Polygon Actor (%s) %d"), *Data.Name, Data.Id));
            APCGPolygonActor* PolygonActor = GetWorld()->SpawnActor<APCGPolygonActor>(
                APCGPolygonActor::StaticClass(),
                UniquePoints[0],
                FRotator::ZeroRotator,
                SpawnParams);

            if (!PolygonActor)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn APCGPolygonActor."));
                return;
            }

#if WITH_EDITOR
            PolygonActor->SetActorLabel(FString::Printf(TEXT("PCG Polygon Actor (%s) %d"), *Data.Name, Data.Id));
#endif

            PolygonActor->Data = Data;
            PolygonActor->Data.BoxExtents = BoxExtents;
            PolygonActor->InteriorSampleSpacing = Data.Density * 100.0f;
            PolygonActor->InteriorBorderSpacing = 100.0f;
            PolygonActor->FileName = Data.FileName;
            PolygonActor->ID = Data.Id;

            InsertPolygonFeaturesToDB(PolygonActor->Data);

            USplineComponent* Spline = PolygonActor->SplineComponent;
            Spline->ClearSplinePoints();

            FVector Origin = UniquePoints[0];
            for (int32 i = 0; i < UniquePoints.Num(); ++i)
            {
                FVector LocalPoint = UniquePoints[i] - Origin;
                Spline->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local);
                Spline->SetTangentAtSplinePoint(i, FVector::ZeroVector, ESplineCoordinateSpace::Local);
            }
            //  UE_LOG(LogTemp, Log, TEXT("Total Spline(%d) Points : %d"), Data.Id, Spline->GetNumberOfSplinePoints());

            Spline->SetClosedLoop(true);
            Spline->UpdateSpline();


            if (IsValid(ParentActor) && IsValid(ParentActor->GetRootComponent()))
            {
                PolygonActor->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            }

            UPCGComponent* PCG = PolygonActor->PCGComponent;
            PCG->SetComponentTickEnabled(false);
            PCG->bRuntimeGenerated = true;
            PCG->bActivated = true;

            FString GraphName = Data.Model.Replace(TEXT(" "), TEXT("")).ToLower();
            AssignPCGGraph(PCG, GraphName);


            PCG->CleanupLocal(true, true);
            PCG->GenerateLocal(true);
            PCG->OnPCGGraphGeneratedExternal.AddDynamic(this, &APCGPolygonContent::OnPcgGraphGenerated);

        });

    Tileset->SampleHeightMostDetailed(Points, Callback);
}

void APCGPolygonContent::OnPcgGraphGenerated(UPCGComponent* PCG)
{
    if (!PCG || !PCG->GetOwner()) return;

    APCGPolygonActor* PolygonActor = Cast<APCGPolygonActor>(PCG->GetOwner());
    if (!PolygonActor) return;

    TArray<FBakedInstanceBatch> Batches;
    for (UActorComponent* Comp : PolygonActor->GetComponents())
    {
        if (auto* HISMC = Cast<UHierarchicalInstancedStaticMeshComponent>(Comp))
            ExtractFromHISMC(HISMC, Batches);
        if (auto* ISMC = Cast<UInstancedStaticMeshComponent>(Comp))
            ExtractFromISMC(ISMC, Batches);
    }

    if (Batches.Num() == 0) return;

    for (auto& Batch : Batches)
    {
        InsertPolygonPointsToDB(PolygonActor->FileName, PolygonActor->Data, Batch.Transforms, Batch.MeshPath.ToString()); 
    }

    UE_LOG(LogTemp, Log, TEXT("Inserted polygon %d from shapefile %s into DB"), PolygonActor->ID, *PolygonActor->FileName);
}


void APCGPolygonContent::SpawnHiGenActorsFromDatabase(const FString& ShapefileID)
{
    if (!GI) return;
    if (!DBSubsystem || !DBSubsystem->IsOpen())
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnHiGenActorsFromDatabase: DB subsystem not available/open"));
        return;
    }

    FActorSpawnParameters SpawnParams;

    ParentActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

    if (ParentActor)
    {
        USceneComponent* SceneRoot = NewObject<USceneComponent>(ParentActor);
        SceneRoot->SetMobility(EComponentMobility::Static);
        SceneRoot->RegisterComponent();
        ParentActor->SetRootComponent(SceneRoot);

#if WITH_EDITOR
        ParentActor->SetActorLabel(TEXT("PCG Polygon Parent"));
#endif
    }

    TMap<int32, TArray<FVector>> PolygonPointsMap;
    TMap<int32, FString> PolygonMeshMap;
    TMap<int32, FString> PolygonGraphMap;
    TMap<int32, FVector> PolygonBoxExtentsMap;  

    // Query points
    FString PointsSQL = FString::Printf(TEXT("SELECT PolygonID, X, Y, Z, MeshID FROM PolygonPoints WHERE ShapefileID='%s';"), *ShapefileID);
    DBSubsystem->ExecuteWithCallback(PointsSQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
        {
            int32 PolygonID = 0;
            float X = 0, Y = 0, Z = 0;
            FString MeshID;

            Statement.GetColumnValueByName(TEXT("PolygonID"), PolygonID);
            Statement.GetColumnValueByName(TEXT("X"), X);
            Statement.GetColumnValueByName(TEXT("Y"), Y);
            Statement.GetColumnValueByName(TEXT("Z"), Z);
            Statement.GetColumnValueByName(TEXT("MeshID"), MeshID);

            PolygonPointsMap.FindOrAdd(PolygonID).Add(FVector(X, Y, Z));
            PolygonMeshMap.FindOrAdd(PolygonID) = MeshID;

            return ESQLitePreparedStatementExecuteRowResult::Continue;
        });

    // Query features 
    FString FeatureSQL = FString::Printf(TEXT("SELECT PolygonID, Model FROM PolygonFeatures WHERE ShapefileID='%s';"), *ShapefileID);
    DBSubsystem->ExecuteWithCallback(FeatureSQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
        {
            int32 PolygonID = 0;
            FString Model;
            Statement.GetColumnValueByName(TEXT("PolygonID"), PolygonID);
            Statement.GetColumnValueByName(TEXT("Model"), Model);
            PolygonGraphMap.FindOrAdd(PolygonID) = Model;

            return ESQLitePreparedStatementExecuteRowResult::Continue;
        });

    // Query bounding box extents
    FString ExtentsSQL = FString::Printf(TEXT("SELECT PolygonID, BoxExtentX, BoxExtentY, BoxExtentZ FROM PolygonFeatures WHERE ShapefileID='%s';"), *ShapefileID);
    DBSubsystem->ExecuteWithCallback(ExtentsSQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
        {
            int32 PolygonID = 0;
            float BoxExtentX = 0, BoxExtentY = 0, BoxExtentZ = 0;
            Statement.GetColumnValueByName(TEXT("PolygonID"), PolygonID);
            Statement.GetColumnValueByName(TEXT("BoxExtentX"), BoxExtentX);
            Statement.GetColumnValueByName(TEXT("BoxExtentY"), BoxExtentY);
            Statement.GetColumnValueByName(TEXT("BoxExtentZ"), BoxExtentZ);

            PolygonBoxExtentsMap.FindOrAdd(PolygonID) = FVector(BoxExtentX, BoxExtentY, BoxExtentZ);

            return ESQLitePreparedStatementExecuteRowResult::Continue;
        });


    int32 TotalPolygons = PolygonPointsMap.Num();
    int32 HalfPolygons = TotalPolygons ;  
    int32 Counter = 0;

    // Spawn a HiGen actor per polygon
    for (auto& Elem : PolygonPointsMap)
    {
        int32 PolygonID = Elem.Key;

        if (Counter >= HalfPolygons) break;

        const TArray<FVector>& Points = Elem.Value;
        if (Points.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Polygon %d has no points; skipping."), PolygonID);
            continue;
        }

        const FString& MeshID = PolygonMeshMap.FindRef(PolygonID);
        const FString& ModelName = PolygonGraphMap.FindRef(PolygonID);
        const FVector& BoxExtents = PolygonBoxExtentsMap.FindRef(PolygonID);  

        FString ActorName = FString::Printf(TEXT("HiGenActor_Polygon_%d"), PolygonID);
        FActorSpawnParameters Params;
        Params.Owner = this;

        FVector Location = Points[0];

        APolygonHiGenActor* HiGenActor = GetWorld()->SpawnActor<APolygonHiGenActor>(
            APolygonHiGenActor::StaticClass(),
            Location,
            FRotator::ZeroRotator,
            Params);

        if (!IsValid(HiGenActor))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn HiGenActor for polygon %d"), PolygonID);
            continue;
        }

#if WITH_EDITOR
        HiGenActor->SetActorLabel(ActorName);
#endif

        if (IsValid(ParentActor) && IsValid(ParentActor->GetRootComponent()))
        {
            HiGenActor->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        }

        HiGenActor->PolygonID = PolygonID;
        HiGenActor->Positions = Points;
        HiGenActor->BoxExtents = BoxExtents;

        FString GraphName = ModelName.Replace(TEXT(" "), TEXT("")).ToLower();
        HiGenActor->GraphName = GraphName;
        HiGenActor->AssignHiGenGraph();

        Counter++; 
    }
}




//void APCGPolygonContent::SpawnHiGenActorsFromDatabase(const FString& ShapefileID)
//{
//    if (!GI) return;
//    if (!DBSubsystem || !DBSubsystem->IsOpen())
//    {
//        UE_LOG(LogTemp, Error, TEXT("SpawnHiGenActorsFromDatabase: DB subsystem not available/open"));
//        return;
//    }
//
//    FActorSpawnParameters SpawnParams;
// 
//    ParentActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
//
//    if (ParentActor)
//    {
//
//        USceneComponent* SceneRoot = NewObject<USceneComponent>(ParentActor);
//        SceneRoot->SetMobility(EComponentMobility::Static);
//        SceneRoot->RegisterComponent();
//        ParentActor->SetRootComponent(SceneRoot);
//
//#if WITH_EDITOR
//        ParentActor->SetActorLabel(TEXT("PCG Polygon Parent"));
//#endif
//    }
//
//
//    TMap<int32, TArray<FVector>> PolygonPointsMap;
//    TMap<int32, FString> PolygonMeshMap;
//    TMap<int32, FString> PolygonGraphMap;
//
//    // Query points
//    FString PointsSQL = FString::Printf(TEXT("SELECT PolygonID, X, Y, Z, MeshID FROM PolygonPoints WHERE ShapefileID='%s';"), *ShapefileID);
//    DBSubsystem->ExecuteWithCallback(PointsSQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
//        {
//            int32 PolygonID = 0;
//            float X = 0, Y = 0, Z = 0;
//            FString MeshID;
//
//            Statement.GetColumnValueByName(TEXT("PolygonID"), PolygonID);
//            Statement.GetColumnValueByName(TEXT("X"), X);
//            Statement.GetColumnValueByName(TEXT("Y"), Y);
//            Statement.GetColumnValueByName(TEXT("Z"), Z);
//            Statement.GetColumnValueByName(TEXT("MeshID"), MeshID);
//
//            PolygonPointsMap.FindOrAdd(PolygonID).Add(FVector(X, Y, Z));
//            PolygonMeshMap.FindOrAdd(PolygonID) = MeshID;
//
//            return ESQLitePreparedStatementExecuteRowResult::Continue;
//        });
//
//    // Query features
//    FString FeatureSQL = FString::Printf(TEXT("SELECT PolygonID, Model FROM PolygonFeatures WHERE ShapefileID='%s';"), *ShapefileID);
//    DBSubsystem->ExecuteWithCallback(FeatureSQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
//        {
//            int32 PolygonID = 0;
//            FString Model;
//            Statement.GetColumnValueByName(TEXT("PolygonID"), PolygonID);
//            Statement.GetColumnValueByName(TEXT("Model"), Model);
//            PolygonGraphMap.FindOrAdd(PolygonID) = Model;
//            return ESQLitePreparedStatementExecuteRowResult::Continue;
//        });
//
//    // Spawn a HiGen actor per polygon
//    for (auto& Elem : PolygonPointsMap)
//    {
//        int32 PolygonID = Elem.Key;
//
//        const TArray<FVector>& Points = Elem.Value;
//        if (Points.Num() == 0)
//        {
//            UE_LOG(LogTemp, Warning, TEXT("Polygon %d has no points; skipping."), PolygonID);
//            continue;
//        }
//
//        const FString& MeshID = PolygonMeshMap.FindRef(PolygonID);
//        const FString& ModelName = PolygonGraphMap.FindRef(PolygonID);
//
//        FString ActorName = FString::Printf(TEXT("HiGenActor_Polygon_%d"), PolygonID);
//        FActorSpawnParameters Params;
//        Params.Owner = this;
//        //Params.Name = FName(*ActorName);
//
//        FVector Location = Points[0];
//
//        APolygonHiGenActor* HiGenActor = GetWorld()->SpawnActor<APolygonHiGenActor>(
//            APolygonHiGenActor::StaticClass(),
//            Location,
//            FRotator::ZeroRotator,
//            Params);
//
//        if (!IsValid(HiGenActor))
//        {
//            UE_LOG(LogTemp, Error, TEXT("Failed to spawn HiGenActor for polygon %d"), PolygonID);
//            continue;
//        }
//
//#if WITH_EDITOR
//        HiGenActor->SetActorLabel(ActorName);
//#endif
//
//        if (IsValid(ParentActor) && IsValid(ParentActor->GetRootComponent()))
//        {
//            HiGenActor->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
//        }
//
//        HiGenActor->PolygonID = PolygonID;
//        HiGenActor->Positions = Points;
//
//        FString GraphName = ModelName.Replace(TEXT(" "), TEXT("")).ToLower();
//        HiGenActor->GraphName = GraphName;
//        HiGenActor->AssignHiGenGraph();
//    }
//}

