#include "PCGPointContent.h" 


int32 APCGPointContent::TotalPointDataPoints = 0;
int32 APCGPointContent::ExpectedTotalPoints = 0;
FDateTime APCGPointContent::StartTime = FDateTime::UtcNow();
bool APCGPointContent::bTimerStarted = false;

APCGPointContent::APCGPointContent()
{

}


void APCGPointContent::InitializeContent()
{
    CesiumGeoreference = ACesiumGeoreference::GetDefaultGeoreference(GetWorld());
    Tileset = Cast<ACesium3DTileset>(UGameplayStatics::GetActorOfClass(GetWorld(), ACesium3DTileset::StaticClass()));

    const FString RegistryPath = TEXT("/Game/PCGData/PointDataAssets/BP_Registry.BP_Registry");


    UBlueprintRegistery* LoadedRegistry = Cast<UBlueprintRegistery>(
        StaticLoadObject(UBlueprintRegistery::StaticClass(), nullptr, *RegistryPath)
    );


    if (LoadedRegistry)
    {
        BlueprintRegistry = LoadedRegistry;
        UE_LOG(LogTemp, Warning, TEXT("Successfully loaded BP_Registry asset."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load BP_Registry from path: %s"), *RegistryPath);
    }


}

void APCGPointContent::InitializePCGPointData(const FShapeRawData& Raw)
{


    InitializeContent();
    for (int32 i = 0; i < Raw.Attributes.Num(); ++i)
    {


        const auto& A = Raw.Attributes[i];

        auto GetSafeString = [&](const FString& key) ->FString {
            return A.Contains(key) ? A[key] : TEXT("");
            };


        auto GetSafeInt = [&](const FString& key) -> int32 {
            return A.Contains(key) ? FCString::Atoi(*A[key]) : 0;

            };


        auto GetSafeFloat = [&](const FString& key) -> float {
            return A.Contains(key) ? FCString::Atof(*A[key]) : 0.0f;

            };


        FVegetationPointData temp;
        //temp.Id = FCString::Atoi(*A["id"]);
        //temp.Name = A["Name"];
        //temp.Lat = FCString::Atof(*A["lat"]);
        //temp.Long = FCString::Atof(*A["long"]);
        //temp.Elev = FCString::Atof(*A["Elev"]);
        //temp.Height = FCString::Atof(*A["Height"]);
        //temp.Scale = FCString::Atof(*A["Scale"]);
        //temp.Rotation = FCString::Atof(*A["Rotation"]);
        //temp.Code = A["code"];
        //temp.Model = A["Model"];
        //temp.FileName = Raw.CollectionName;
        // temp.State = A["State"];
        // temp.Altitude = FCString::Atof(*A["Altitude"]);
        // temp.KindID = FCString::Atoi(*A["KindID"]);
        // temp.KindDesc = A["KindDesc"];
        // temp.DomainID = FCString::Atoi(*A["DomainID"]);
        // temp.DomainDesc = A["DomainDesc"];
        // temp.CountryID = FCString::Atoi(*A["CountryID"]);
        // temp.CountryDes = A["CountryDes"];
        // temp.CategoryID = FCString::Atoi(*A["CategoryID"]);
        // temp.CategoryDe = A["CategoryDe"];
        // temp.SubCategID = FCString::Atoi(*A["SubCategID"]);
        // temp.SubCategDe = A["SubCategDe"];
        // temp.SpecificID = FCString::Atoi(*A["SpecificID"]);
        // temp.SpecificDe = A["SpecificDe"];
        //  temp.EntityEnum = A.Contains("EntityEnum") ? A["EntityEnum"] : TEXT("");
        //  temp.Location = Raw.Geometries[i].IsValidIndex(0) ? Raw.Geometries[i][0] : FVector::ZeroVector;



          temp.Id = GetSafeInt("id");
          temp.Name = A["Name"];
          temp.Lat = GetSafeFloat("lat");
          temp.Long = GetSafeFloat("long");
          temp.Elev = GetSafeFloat("Elev");
          temp.Height = GetSafeFloat("Height");
          temp.Scale = GetSafeFloat("Scale");
          temp.Rotation = GetSafeFloat("Rotation");
          temp.Code = GetSafeString("code");
          temp.Model = GetSafeString("Model");
          temp.FileName = Raw.CollectionName;
          temp.State = GetSafeString("State");
          temp.Altitude = GetSafeFloat("Altitude");
          temp.KindID = GetSafeInt("KindID");
          temp.KindDesc = GetSafeString("KindDesc");
          temp.DomainID = GetSafeInt("DomainID");
          temp.DomainDesc = GetSafeString("DomainDesc");
          temp.CountryID = GetSafeInt("CountryID");
          temp.CountryDes = GetSafeString("CountryDes");
          temp.CategoryID = GetSafeInt("CategoryID");
          temp.CategoryDe = GetSafeString("CategoryDe");
          temp.SubCategID = GetSafeInt("SubCategID");
          temp.SubCategDe = GetSafeString("SubCategDe");
          temp.SpecificID = GetSafeInt("SpecificID");
          temp.SpecificDe = GetSafeString("SpecificDe");
          temp.EntityEnum = A.Contains("EntityEnum") ? A["EntityEnum"] : TEXT("");
          temp.Location = Raw.Geometries[i].IsValidIndex(0) ? Raw.Geometries[i][0] : FVector::ZeroVector;
          PointDataList.Add(temp);
    }


}

void APCGPointContent::SpawnPCGPointData()
{


    if (!bTimerStarted) {
        StartTime = FDateTime::UtcNow();
        bTimerStarted = true;

    }

    if (PointDataList.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No PCG point data available to spawn."));
        return;
    }

    ExpectedTotalPoints += PointDataList.Num();

    //TotalPointDataPoints += PointDataList.Num();

    FString BaseName = PointDataList[0].FileName;
    FName DesiredName(*BaseName);

    // Generate a unique name in the current world (Outer)
    FName UniqueName = MakeUniqueObjectName(GetWorld(), AActor::StaticClass(), DesiredName);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = UniqueName;

    ParentActor = GetWorld()->SpawnActor<AActor>(
        AActor::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);
    if (ParentActor)
    {
        USceneComponent* SceneRoot = NewObject<USceneComponent>(ParentActor);
        SceneRoot->SetupAttachment(nullptr);
        SceneRoot->RegisterComponent();
        ParentActor->SetRootComponent(SceneRoot);

#if WITH_EDITOR
        ParentActor->SetActorLabel(TEXT("PCG Point Parent"));
#endif
    }

    for (const FVegetationPointData& Data : PointDataList)
    {
        // SpawnPointDataOnTerrainUsingElevData(Data);
        //SpawnPointDataOnTerrainUsingCesiumSampler_HISM(Data);
        SpawnPointDataOnTerrainUsingElevData_HISM(Data);
        APCGPointContent::TotalPointDataPoints++;

        //SpawnPointDataOnTerrainUsingCesiumSampler(Data);
    }


}

void APCGPointContent::SpawnPointDataOnTerrainUsingCesiumSampler(const FVegetationPointData& Data)
{
    if (!CesiumGeoreference || !BlueprintRegistry || !Tileset)
    {
        UE_LOG(LogTemp, Error, TEXT("CesiumGeoreference, Tileset, or BlueprintRegistry is null."));
        return;
    }

    TArray<FVector> Positions;
    Positions.Add(FVector(Data.Long, Data.Lat, 0)); // Z is ignored by Cesium sampler

    // Pre-bind data
    FString CleanedName = Data.Model.Replace(TEXT(" "), TEXT("")).ToLower();
    FVector FinalScale = FVector(Data.Scale / 100.f);
    FRotator FinalRotation = FRotator(0.0f, Data.Rotation + 90.0f, 0.0f);

    FCesiumSampleHeightMostDetailedCallback CesiumCallback;
    CesiumCallback.BindLambda(
        [this, CleanedName, FinalScale, FinalRotation, Data](ACesium3DTileset* /*Tileset*/, const TArray<FCesiumSampleHeightResult>& Results, const TArray<FString>& Warnings)
        {
            if (Results.Num() == 0 || !Results[0].SampleSuccess)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to sample terrain height for model: %s"), *CleanedName);
                return;
            }

            if (!CesiumGeoreference) return;
            const FCesiumSampleHeightResult& Result = Results[0];
            FVector WorldLocation = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(Result.LongitudeLatitudeHeight);

            // Load blueprint from registry
            TSubclassOf<AActor> MatchedBPClass = nullptr;
            for (const TSoftClassPtr<AActor>& SoftBP : BlueprintRegistry->BlueprintClasses)
            {
                FString AssetName = SoftBP.GetAssetName().ToLower();
                AssetName.RemoveFromEnd(TEXT("_c"));

                if (AssetName.Equals("pd_" + CleanedName))
                {
                    MatchedBPClass = SoftBP.LoadSynchronous();
                    break;
                }
            }

            if (!MatchedBPClass)
            {
                UE_LOG(LogTemp, Error, TEXT("No blueprint found in registry for model: %s"), *CleanedName);
                return;
            }

            // Spawn actor
            FActorSpawnParameters SpawnParams;
            AActor* Spawned = GetWorld()->SpawnActor<AActor>(MatchedBPClass, WorldLocation, FinalRotation, SpawnParams);
            if (Spawned && ParentActor && ParentActor->GetRootComponent())
            {
                Spawned->SetActorScale3D(FinalScale);
                Spawned->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
                UE_LOG(LogTemp, Log, TEXT("Spawned %s at %s with Cesium height: %.2f"), *CleanedName, *WorldLocation.ToString(), Result.LongitudeLatitudeHeight[2]);
            }

            for (const FString& Warn : Warnings)
            {
                UE_LOG(LogTemp, Warning, TEXT("Cesium Warning: %s"), *Warn);
            }
        });

    Tileset->SampleHeightMostDetailed(Positions, CesiumCallback);
}

void APCGPointContent::SpawnPointDataOnTerrainUsingElevData(const FVegetationPointData& Data)
{
    if (!CesiumGeoreference || !BlueprintRegistry)
    {
        UE_LOG(LogTemp, Error, TEXT("CesiumGeoreference or BlueprintRegistry is null."));
        return;
    }

    // Step 1: Convert LLH to Unreal World Location
    FVector LLH(static_cast<double>(Data.Long), static_cast<double>(Data.Lat), static_cast<double>(Data.Elev));
    FVector WorldLocation = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(LLH);

    // Step 2: Match model name
    FString CleanedName = Data.Model.Replace(TEXT(" "), TEXT("")).ToLower();
    TSubclassOf<AActor> MatchedBPClass = nullptr;

    for (const TSoftClassPtr<AActor>& SoftBP : BlueprintRegistry->BlueprintClasses)
    {
        FString AssetName = SoftBP.GetAssetName().ToLower();
        AssetName.RemoveFromEnd(TEXT("_c")); // remove _C suffix if present

        if (AssetName.Equals("pd_" + CleanedName))
        {
            MatchedBPClass = SoftBP.LoadSynchronous();
            break;
        }
    }

    if (!MatchedBPClass)
    {
        UE_LOG(LogTemp, Error, TEXT("No blueprint found in registry for model: %s"), *CleanedName);
        return;
    }

    // Step 3: Spawn actor with position, scale, and rotation
    FVector FinalScale = FVector(Data.Scale / 100.f);
    FRotator FinalRotation = FRotator(0.0f, Data.Rotation, 0.0f);
    FActorSpawnParameters SpawnParams;

    AActor* Spawned = GetWorld()->SpawnActor<AActor>(MatchedBPClass, WorldLocation, FinalRotation, SpawnParams);
    if (Spawned && ParentActor && ParentActor->GetRootComponent())
    {
        Spawned->SetActorScale3D(FinalScale);
        Spawned->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        UE_LOG(LogTemp, Log, TEXT("Spawned %s at %s"), *CleanedName, *WorldLocation.ToString());
    }
}

void APCGPointContent::SpawnPointDataOnTerrainUsingCesiumSampler_HISM(const FVegetationPointData& Data)
{
    if (!CesiumGeoreference || !BlueprintRegistry || !Tileset || !ParentActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing required references."));
        return;
    }

    TArray<FVector> Positions;
    Positions.Add(FVector(Data.Long, Data.Lat, 0)); // Z ignored for sampling

    FString CleanedName = Data.Model.Replace(TEXT(" "), TEXT("")).ToLower();
    FVector FinalScale = FVector(Data.Scale / 100.f);
    FRotator FinalRotation = FRotator(0.0f, Data.Rotation, 0.0f);



    FCesiumSampleHeightMostDetailedCallback CesiumCallback;
    CesiumCallback.BindLambda(
        [this, CleanedName, FinalScale, FinalRotation, Data](ACesium3DTileset*, const TArray<FCesiumSampleHeightResult>& Results, const TArray<FString>& Warnings)
        {
            if (Results.Num() == 0 || !Results[0].SampleSuccess)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to sample height for %s"), *CleanedName);
                return;
            }
            if (!CesiumGeoreference) return;

            // Step 1: Cesium world transform
            const FVector WorldLocation = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(
                Results[0].LongitudeLatitudeHeight);


            // Step 2: Retrieve or extract mesh
            UStaticMesh* MeshToUse = StaticMeshesPerType.FindRef(CleanedName);
            if (!MeshToUse)
            {
                TSubclassOf<AActor> MatchedBPClass = nullptr;
                for (const TSoftClassPtr<AActor>& SoftBP : BlueprintRegistry->BlueprintClasses)
                {
                    FString AssetName = SoftBP.GetAssetName().ToLower();
                    AssetName.RemoveFromEnd(TEXT("_c"));

                    if (AssetName.Equals("pd_" + CleanedName))
                    {
                        MatchedBPClass = SoftBP.LoadSynchronous();
                        break;
                    }
                }

                if (!MatchedBPClass)
                {
                    UE_LOG(LogTemp, Error, TEXT("Blueprint not found for type: %s"), *CleanedName);
                    return;
                }

                MeshToUse = ExtractMeshFromBlueprint(MatchedBPClass);
                if (!MeshToUse)
                {
                    UE_LOG(LogTemp, Error, TEXT("No mesh extracted from blueprint: %s"), *CleanedName);
                    return;
                }

                StaticMeshesPerType.Add(CleanedName, MeshToUse);
            }

            UHierarchicalInstancedStaticMeshComponent* HISM = nullptr;

            if (!InstancedMeshMap.Contains(CleanedName))
            {
                HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(ParentActor);
                HISM->SetStaticMesh(MeshToUse);
                HISM->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
                HISM->RegisterComponent();
                AddInstanceComponent(HISM);
                InstancedMeshMap.Add(CleanedName, HISM);
            }
            else
            {
                HISM = InstancedMeshMap[CleanedName];
            }

            // Step 4: Add the instance
            FTransform InstanceTransform(FinalRotation, WorldLocation, FinalScale);
            HISM->AddInstance(InstanceTransform);

            //APCGPointContent::TotalPointDataPoints++;

            if (APCGPointContent::TotalPointDataPoints!=0 && ExpectedTotalPoints == APCGPointContent::TotalPointDataPoints) {
                FTimespan Elapsed = FDateTime::UtcNow() - StartTime;

                GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Green, FString::Printf(TEXT("Total Point Data Points: %d"), TotalPointDataPoints));
                GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, FString::Printf(TEXT("Total Time(s) Point Data: %f"), Elapsed.GetTotalSeconds()));


                UE_LOG(LogTemp, Log, TEXT("Total Point Data Points are: %d"), TotalPointDataPoints);
                UE_LOG(LogTemp, Log, TEXT("Total Execution Time(Point Data): %.2f"), Elapsed.GetTotalSeconds());

                TotalPointDataPoints = 0;
                ExpectedTotalPoints = 0;
                StartTime = FDateTime::MinValue();
                bTimerStarted = false;
            }


 
            UE_LOG(LogTemp, Log, TEXT("Instanced %s at %s"), *CleanedName, *WorldLocation.ToString());

            for (const FString& Warn : Warnings)
            {
                UE_LOG(LogTemp, Warning, TEXT("Cesium warning: %s"), *Warn);
            }
        });

    Tileset->SampleHeightMostDetailed(Positions, CesiumCallback);
}

void APCGPointContent::SpawnPointDataOnTerrainUsingElevData_HISM(const FVegetationPointData& Data) {
    if (!CesiumGeoreference || !BlueprintRegistry)
    {
        UE_LOG(LogTemp, Error, TEXT("CesiumGeoreference or BlueprintRegistry is null."));
        return;
    }
    FVector LLH(static_cast<double>(Data.Long), static_cast<double>(Data.Lat), static_cast<double>(Data.Elev));
    FVector WorldLocation = CesiumGeoreference->TransformLongitudeLatitudeHeightPositionToUnreal(LLH);
    FString CleanedName = Data.Model.Replace(TEXT(" "), TEXT("")).ToLower();
    FVector FinalScale = FVector(Data.Scale / 100.f);
    FRotator FinalRotation = FRotator(0.0f, Data.Rotation, 0.0f);
    TSubclassOf<AActor> MatchedBPClass = nullptr;
    UStaticMesh* MeshToUse = StaticMeshesPerType.FindRef(CleanedName);

    for (const TSoftClassPtr<AActor>& SoftBP : BlueprintRegistry->BlueprintClasses)
    {
        FString AssetName = SoftBP.GetAssetName().ToLower();
        AssetName.RemoveFromEnd(TEXT("_c")); // remove _C suffix if present

        if (AssetName.Equals("pd_" + CleanedName))
        {
            MatchedBPClass = SoftBP.LoadSynchronous();
            break;
        }
    }
    if (!MatchedBPClass)
    {
        UE_LOG(LogTemp, Error, TEXT("No blueprint found in registry for model: %s"), *CleanedName);
        return;
    }
    MeshToUse = ExtractMeshFromBlueprint(MatchedBPClass);
    if (!MeshToUse)
    {
        UE_LOG(LogTemp, Error, TEXT("No mesh extracted from blueprint: %s"), *CleanedName);
        return;
    }
    StaticMeshesPerType.Add(CleanedName, MeshToUse);

    UHierarchicalInstancedStaticMeshComponent* HISM = nullptr;

    if (!InstancedMeshMap.Contains(CleanedName))
    {
        HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(ParentActor);
        HISM->SetStaticMesh(MeshToUse);
        HISM->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        HISM->RegisterComponent();
        AddInstanceComponent(HISM);
        InstancedMeshMap.Add(CleanedName, HISM);
    }
    else
    {
        HISM = InstancedMeshMap[CleanedName];
    }
    FTransform InstanceTransform(FinalRotation, WorldLocation, FinalScale);
    HISM->AddInstance(InstanceTransform);
}

UStaticMesh* APCGPointContent::ExtractMeshFromBlueprint(TSubclassOf<AActor> BPClass)
{
    if (!BPClass) return nullptr;

    AActor* TempActor = GetWorld()->SpawnActor<AActor>(BPClass, FVector::ZeroVector, FRotator::ZeroRotator);
    if (!TempActor) return nullptr;

    UStaticMesh* FoundMesh = nullptr;
    TArray<UStaticMeshComponent*> MeshComponents;
    TempActor->GetComponents<UStaticMeshComponent>(MeshComponents);

    for (UStaticMeshComponent* Comp : MeshComponents)
    {
        if (Comp->GetStaticMesh())
        {
            FoundMesh = Comp->GetStaticMesh();
            break;
        }
    }

    TempActor->Destroy();
    return FoundMesh;
}

