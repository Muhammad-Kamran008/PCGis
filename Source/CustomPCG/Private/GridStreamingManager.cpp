#include "GridStreamingManager.h"
#include "SQLiteDatabase.h"
#include <SQLiteResultSet.h>
#include <Components/HierarchicalInstancedStaticMeshComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Engine/StreamableManager.h>
#include "Engine/AssetManager.h"



AGridStreamingManager::AGridStreamingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    ParentActor = this;
}

void AGridStreamingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
  
    if (++FramesSinceUpdate >= DrawEveryNFrames)
    {
        if (UWorld* World = GetWorld())
        {
            PC = UGameplayStatics::GetPlayerController(this, 0);
            UpdateStreaming(PC->PlayerCameraManager->GetCameraLocation());
        }
        FramesSinceUpdate = 0;
    }
}

void AGridStreamingManager::UpdateStreaming(FVector PawnLocation)
{
    int32 MinX = FMath::FloorToInt((PawnLocation.X - Radius) / GridSize);
    int32 MaxX = FMath::FloorToInt((PawnLocation.X + Radius) / GridSize);
    int32 MinY = FMath::FloorToInt((PawnLocation.Y - Radius) / GridSize);
    int32 MaxY = FMath::FloorToInt((PawnLocation.Y + Radius) / GridSize);

    TSet<FIntPoint> NewCells;
    for (int32 X = MinX; X <= MaxX; ++X)
        for (int32 Y = MinY; Y <= MaxY; ++Y)
            NewCells.Add(FIntPoint(X, Y));

    // Cells to unload
    for (auto& Cell : LoadedCells.Difference(NewCells))
        UnloadCell(Cell);

    // Cells to load
    for (auto& Cell : NewCells.Difference(LoadedCells)) {
        LoadCell(Cell);
        //RequestedCells.Add(Cell); // Mark as requested immediately
    }
    LoadedCells = NewCells;
}

void AGridStreamingManager::LoadCell(FIntPoint Cell)
{
    FString DBPath = FPaths::ProjectSavedDir() / TEXT("PolygonData.db");
    if (!DB.Open(*DBPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open DB"));
        return;
    }

    FString SQL = FString::Printf(TEXT(
        "SELECT PolygonID, X, Y, Z, MeshID FROM PolygonPoints WHERE GridX=%d AND GridY=%d;"),
        Cell.X, Cell.Y);

    // Execute the statement with a callback for each row
    DB.Execute(*SQL, [this](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
        {
            int32 PolygonID = 0;
            float X = 0, Y = 0, Z = 0;
            FString MeshPath;

            // Use GetColumnValueByName to extract values from current row
            Statement.GetColumnValueByName(TEXT("PolygonID"), PolygonID);
            Statement.GetColumnValueByName(TEXT("X"), X);
            Statement.GetColumnValueByName(TEXT("Y"), Y);
            Statement.GetColumnValueByName(TEXT("Z"), Z);
            Statement.GetColumnValueByName(TEXT("MeshID"), MeshPath);

            FVector Loc(X, Y, Z);

            UStaticMesh* Mesh = Cast<UStaticMesh>(FSoftObjectPath(MeshPath).TryLoad());
            if (!Mesh) return ESQLitePreparedStatementExecuteRowResult::Continue;

            // Spawn or reuse HISM component
            UHierarchicalInstancedStaticMeshComponent* HISMC = nullptr;
            for (auto* Comp : ParentActor->GetComponents())
            {
                if (auto* H = Cast<UHierarchicalInstancedStaticMeshComponent>(Comp))
                    if (H->GetStaticMesh() == Mesh) { HISMC = H; break; }
            }
            if (!HISMC)
            {
                HISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(ParentActor);
                HISMC->SetupAttachment(ParentActor->GetRootComponent());
                HISMC->SetStaticMesh(Mesh);
                HISMC->RegisterComponent();
            }

            HISMC->AddInstanceWorldSpace(FTransform(Loc));

            return ESQLitePreparedStatementExecuteRowResult::Continue;
        });

    DB.Close();
}

void AGridStreamingManager::LoadDataFromDatabase(const FString& ShapefileID)
{
    PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;
    if (!ParentActor) ParentActor = this; // default root actor

    // Compute which grid cells to load based on pawn location
    FVector PawnLoc = PC->PlayerCameraManager->GetCameraLocation();
    int32 MinX = FMath::FloorToInt((PawnLoc.X - Radius) / GridSize);
    int32 MaxX = FMath::FloorToInt((PawnLoc.X + Radius) / GridSize);
    int32 MinY = FMath::FloorToInt((PawnLoc.Y - Radius) / GridSize);
    int32 MaxY = FMath::FloorToInt((PawnLoc.Y + Radius) / GridSize);

    TSet<FIntPoint> NewCells;
    for (int32 X = MinX; X <= MaxX; ++X)
        for (int32 Y = MinY; Y <= MaxY; ++Y)
            NewCells.Add(FIntPoint(X, Y));

    // Determine which cells to load/unload
    TSet<FIntPoint> CellsToLoad = NewCells.Difference(LoadedCells);
    TSet<FIntPoint> CellsToUnload = LoadedCells.Difference(NewCells);

    // Unload HISM for cells outside radius
    for (const FIntPoint& Cell : CellsToUnload)
        UnloadCell(Cell); // implement removal of HISM instances per cell

    // Load new cells
    FString DBPath = FPaths::ProjectSavedDir() / TEXT("PolygonData.db");
    if (!DB.Open(*DBPath)) return;

    for (const FIntPoint& Cell : CellsToLoad)
    {
        FString SQL = FString::Printf(TEXT(
            "SELECT PolygonID, X, Y, Z, MeshID FROM PolygonPoints "
            "WHERE ShapefileID='%s' AND GridX=%d AND GridY=%d;"),
            *ShapefileID, Cell.X, Cell.Y);

        DB.Execute(*SQL, [this](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
            {
                int32 PolygonID;
                float X, Y, Z;
                FString MeshPath;

                Statement.GetColumnValueByName(TEXT("PolygonID"), PolygonID);
                Statement.GetColumnValueByName(TEXT("X"), X);
                Statement.GetColumnValueByName(TEXT("Y"), Y);
                Statement.GetColumnValueByName(TEXT("Z"), Z);
                Statement.GetColumnValueByName(TEXT("MeshID"), MeshPath);

                FVector Loc(X, Y, Z);

                UStaticMesh* Mesh = Cast<UStaticMesh>(FSoftObjectPath(MeshPath).TryLoad());
                if (!Mesh) return ESQLitePreparedStatementExecuteRowResult::Continue;

                // Find existing HISM or create new
                UHierarchicalInstancedStaticMeshComponent* HISMC = nullptr;
                for (auto* Comp : ParentActor->GetComponents())
                {
                    if (auto* H = Cast<UHierarchicalInstancedStaticMeshComponent>(Comp))
                        if (H->GetStaticMesh() == Mesh) { HISMC = H; break; }
                }
                if (!HISMC)
                {
                    HISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(ParentActor);
                    HISMC->SetupAttachment(ParentActor->GetRootComponent());
                    HISMC->SetStaticMesh(Mesh);
                    HISMC->RegisterComponent();
                }

                HISMC->AddInstanceWorldSpace(FTransform(Loc));

                return ESQLitePreparedStatementExecuteRowResult::Continue;
            });
    }

    DB.Close();

    // Update currently loaded cells
    LoadedCells = NewCells;
}

void AGridStreamingManager::UnloadCell(FIntPoint Cell)
{


}

void AGridStreamingManager::LoadCellAsync(FIntPoint Cell)
{
    if (!ParentActor) ParentActor = this;

    // SQL query for this cell
    FString SQL = FString::Printf(TEXT(
        "SELECT PolygonID, X, Y, Z, MeshID FROM PolygonPoints "
        "WHERE GridX=%d AND GridY=%d;"),
        Cell.X, Cell.Y);

    // Mark cell as requested immediately
    RequestedCells.Add(Cell);

    // Run database query on a worker thread
    Async(EAsyncExecution::ThreadPool, [this, SQL, Cell]()
        {
            TArray<FTransform> Transforms;
            TArray<FString> MeshPaths;

            // Use the already open DB instance
            FSQLiteDatabase& LocalDB = DB;

            LocalDB.Execute(*SQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
                {
                    FVector Loc;
                    Statement.GetColumnValueByName(TEXT("X"), Loc.X);
                    Statement.GetColumnValueByName(TEXT("Y"), Loc.Y);
                    Statement.GetColumnValueByName(TEXT("Z"), Loc.Z);
                    FString MeshPath;
                    Statement.GetColumnValueByName(TEXT("MeshID"), MeshPath);

                    Transforms.Add(FTransform(Loc));
                    MeshPaths.Add(MeshPath);

                    return ESQLitePreparedStatementExecuteRowResult::Continue;
                });

            return TTuple<TArray<FTransform>, TArray<FString>>(Transforms, MeshPaths);
        })
        .Next([this, Cell](TTuple<TArray<FTransform>, TArray<FString>> Result)
            {
                // Ensure main thread for HISM
                AsyncTask(ENamedThreads::GameThread, [this, Cell, Result]()
                    {
                        const TArray<FTransform>& Transforms = Result.Get<0>();
                        const TArray<FString>& MeshPaths = Result.Get<1>();

                        if (Transforms.Num() == 0)
                        {
                            RequestedCells.Remove(Cell); // Nothing to load
                            return;
                        }

                        // Preload all meshes using StreamableManager
                        TArray<FSoftObjectPath> MeshesToLoad;
                        for (auto& Path : MeshPaths)
                            MeshesToLoad.Add(FSoftObjectPath(Path));

                        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
                        Streamable.RequestAsyncLoad(MeshesToLoad, FStreamableDelegate::CreateLambda([this, Cell, Transforms, MeshPaths]()
                            {
                                const int32 BatchSize = 50;
                                TArray<TTuple<UStaticMesh*, FTransform>> SpawnQueue;

                                // Prepare spawn queue
                                for (int32 i = 0; i < Transforms.Num(); ++i)
                                {
                                    UStaticMesh* Mesh = Cast<UStaticMesh>(FSoftObjectPath(MeshPaths[i]).TryLoad());
                                    if (!Mesh) continue;
                                    SpawnQueue.Add(MakeTuple(Mesh, Transforms[i]));
                                }

                                // Spawn in batches using timers
                                for (int32 i = 0; i < SpawnQueue.Num(); i += BatchSize)
                                {
                                    TArray<TTuple<UStaticMesh*, FTransform>> Batch;
                                    for (int32 j = i; j < i + BatchSize && j < SpawnQueue.Num(); ++j)
                                        Batch.Add(SpawnQueue[j]);

                                    FTimerHandle TimerHandle;
                                    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, Batch, Cell]()
                                        {
                                            for (auto& Item : Batch)
                                            {
                                                UStaticMesh* Mesh = Item.Get<0>();
                                                FTransform Xf = Item.Get<1>();

                                                // Find or create HISM
                                                UHierarchicalInstancedStaticMeshComponent* HISMC = nullptr;
                                                for (auto* Comp : ParentActor->GetComponents())
                                                {
                                                    if (auto* H = Cast<UHierarchicalInstancedStaticMeshComponent>(Comp))
                                                        if (H->GetStaticMesh() == Mesh) { HISMC = H; break; }
                                                }

                                                if (!HISMC)
                                                {
                                                    HISMC = NewObject<UHierarchicalInstancedStaticMeshComponent>(ParentActor);
                                                    HISMC->SetupAttachment(ParentActor->GetRootComponent());
                                                    HISMC->SetStaticMesh(Mesh);
                                                    HISMC->RegisterComponent();
                                                }

                                                HISMC->AddInstanceWorldSpace(Xf);
                                            }

                                            // After last batch, mark cell loaded
                                            LoadedCells.Add(Cell);
                                            RequestedCells.Remove(Cell);

                                        }, 0.0f, false);
                                }
                            }));
                    });
            });
}
