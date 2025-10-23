#include "PCGManager.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "PCGPointContent.h"
#include "PCGPolygonContent.h"
#include "FShapeFileReader.h"
#include "Engine/World.h" 
#include <GridStreamingManager.h>
#include <PcgSQLiteSubsystem.h>

void APCGManager::InitializeDatabase()
{
    // DB file path
    FString DBPath = FPaths::ProjectContentDir() / TEXT("PCGData/PolygonData.db");

    // Open DB (will create file if it doesn't exist)
    if (!DB.Open(*DBPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open/create SQLite DB at %s"), *DBPath);
        return;
    }

    // --- 1. Create table for polygon points ---
    FString CreatePointsTableSQL = TEXT(R"(
        CREATE TABLE IF NOT EXISTS PolygonPoints (
            ShapefileID TEXT,
            PolygonID INTEGER,
            PointIndex INTEGER,
            X REAL,
            Y REAL,
            Z REAL,
            MeshID TEXT,
            PRIMARY KEY(ShapefileID, PolygonID, PointIndex)
        );
    )");

    if (!DB.Execute(*CreatePointsTableSQL))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create PolygonPoints table"));
    }

    FString CreateFeaturesTableSQL = TEXT(R"(
    CREATE TABLE IF NOT EXISTS PolygonFeatures (
        ShapefileID TEXT,
        PolygonID INTEGER,
        Name TEXT,
        Type TEXT,
        Scale REAL,
        Model TEXT,
        State TEXT,
        Foliage TEXT,
        Density REAL,
        Area REAL,
        Pnts REAL,
        KindID INTEGER,
        KindDesc TEXT,
        DomainID INTEGER,
        DomainDesc TEXT,
        CountryID INTEGER,
        CountryDes TEXT,
        CategoryID INTEGER,
        CategoryDe TEXT,
        SubCategID INTEGER,
        SubCategDe TEXT,
        SpecificID INTEGER,
        SpecificDe TEXT,
        EntityEnum TEXT,
        BoxExtentX REAL,  
        BoxExtentY REAL,  
        BoxExtentZ REAL,  
        PRIMARY KEY(ShapefileID, PolygonID)
    );
)");


    if (!DB.Execute(*CreateFeaturesTableSQL))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create PolygonFeatures table"));
    }


    // --- 3. Create table for shapefile metadata (last modified timestamps) ---
    FString CreateMetadataTableSQL = TEXT(R"(
        CREATE TABLE IF NOT EXISTS ShapefileMetadata (
            ShapefileID TEXT PRIMARY KEY,
            LastModified INT
        );
    )");

    if (!DB.Execute(*CreateMetadataTableSQL))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ShapefileMetadata table"));
    }

    DB.Close();

    UE_LOG(LogTemp, Log, TEXT("SQLite DB initialized successfully at %s"), *DBPath);
}

APCGManager::APCGManager()
{
   // PrimaryActorTick.bCanEverTick = true;
}

void APCGManager::BeginPlay()
{
    Super::BeginPlay();

    // Ensure subsystem is available and DB is opened
    if (GetGameInstance())
    {
        UPcgSQLiteSubsystem* DBSubsystem = GetGameInstance()->GetSubsystem<UPcgSQLiteSubsystem>();
        if (!DBSubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("APCGManager::BeginPlay - DB subsystem not available."));
            return;
        }
        if (!DBSubsystem->IsOpen())
        {
            if (!DBSubsystem->OpenDatabase()) {
                UE_LOG(LogTemp, Error, TEXT("APCGManager::BeginPlay - Failed to open DB via subsystem."));
                return;
            }
        }

        // Create tables once (moved from previous InitializeDatabase implementation)
        const FString CreatePointsTableSQL = TEXT(R"(
            CREATE TABLE IF NOT EXISTS PolygonPoints (
                ShapefileID TEXT,
                PolygonID INTEGER,
                PointIndex INTEGER,
                X REAL,
                Y REAL,
                Z REAL,
                MeshID TEXT,
                PRIMARY KEY(ShapefileID, PolygonID, PointIndex)
            );
        )");


        FString CreateFeaturesTableSQL = TEXT(R"(
    CREATE TABLE IF NOT EXISTS PolygonFeatures (
        ShapefileID TEXT,
        PolygonID INTEGER,
        Name TEXT,
        Type TEXT,
        Scale REAL,
        Model TEXT,
        State TEXT,
        Foliage TEXT,
        Density REAL,
        Area REAL,
        Pnts REAL,
        KindID INTEGER,
        KindDesc TEXT,
        DomainID INTEGER,
        DomainDesc TEXT,
        CountryID INTEGER,
        CountryDes TEXT,
        CategoryID INTEGER,
        CategoryDe TEXT,
        SubCategID INTEGER,
        SubCategDe TEXT,
        SpecificID INTEGER,
        SpecificDe TEXT,
        EntityEnum TEXT,
        BoxExtentX REAL,  
        BoxExtentY REAL,  
        BoxExtentZ REAL,  
        PRIMARY KEY(ShapefileID, PolygonID)
    );
)");



        const FString CreateMetadataTableSQL = TEXT(R"(
            CREATE TABLE IF NOT EXISTS ShapefileMetadata (
                ShapefileID TEXT PRIMARY KEY,
                LastModified INT
            );
        )");

        DBSubsystem->Execute(CreatePointsTableSQL);
        DBSubsystem->Execute(CreateFeaturesTableSQL);
        DBSubsystem->Execute(CreateMetadataTableSQL);

        UE_LOG(LogTemp, Log, TEXT("APCGManager: Database initialized (via subsystem)"));
    }

 

    LoadDataforPCGPolygon();
}

void APCGManager::LoadDataforPCGPoint()
{
    FShapeFileReader::LoadDllForGDAL();

    FString ShapefileFolder = FPaths::ProjectContentDir() + TEXT("Shapefiles/PointData/");
    FPaths::NormalizeDirectoryName(ShapefileFolder);

    if (!FPaths::DirectoryExists(ShapefileFolder))
    {
        UE_LOG(LogTemp, Error, TEXT("Shapefile folder does not exist: %s"), *ShapefileFolder);
        return;
    }

    TArray<FString> ShapefilePaths;
    IFileManager::Get().FindFilesRecursive(ShapefilePaths, *ShapefileFolder, TEXT("*.shp"), true, false);

    if (ShapefilePaths.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT(" No shapefiles found in directory: %s"), *ShapefileFolder);
        return;
    }

    for (const FString& ShapefilePath : ShapefilePaths)
    {
        UE_LOG(LogTemp, Log, TEXT(" Loading shapefile: %s"), *ShapefilePath);

        FShapeRawData RawData = FShapeFileReader::ReadShapefileRawData(ShapefilePath);

        APCGPointContent* PointContent = NewObject<APCGPointContent>(this, APCGPointContent::StaticClass());
        if (!PointContent)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create APCGPointContent for file: %s"), *ShapefilePath);
            continue;
        }

        PointContent->InitializePCGPointData(RawData);
        PointContent->SpawnPCGPointData();
    }
}

void APCGManager::LoadDataforPCGPolygon()
{
    FShapeFileReader::LoadDllForGDAL();

    FString ShapefileFolder = FPaths::ProjectContentDir() / TEXT("Shapefiles/PolygonData/");
    TArray<FString> ShapefilePaths;
    IFileManager::Get().FindFilesRecursive(ShapefilePaths, *ShapefileFolder, TEXT("*.shp"), true, false);

    // Get the SQLite subsystem from GameInstance
    UPcgSQLiteSubsystem* SQLite = GetGameInstance()->GetSubsystem<UPcgSQLiteSubsystem>();
    if (!SQLite)
    {
        UE_LOG(LogTemp, Error, TEXT("SQLite subsystem not available!"));
        return;
    }

    // --- Remove deleted shapefiles from DB ---
    TArray<FString> DBShapefiles;
    const FString ListSQL = TEXT("SELECT ShapefileID FROM ShapefileMetadata;");
    SQLite->ExecuteWithCallback(ListSQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
        {
            FString ShapefileID;
            Statement.GetColumnValueByName(TEXT("ShapefileID"), ShapefileID);
            DBShapefiles.Add(ShapefileID);
            return ESQLitePreparedStatementExecuteRowResult::Continue;
        });

    TArray<FString> AllShapefiles;
    IFileManager::Get().FindFilesRecursive(AllShapefiles, *ShapefileFolder, TEXT("*.shp"), true, false);

    for (const FString& DBFile : DBShapefiles)
    {
        bool bFileExists = AllShapefiles.ContainsByPredicate([&](const FString& FoundFile)
            {
                return FPaths::GetBaseFilename(FoundFile) == DBFile;
            });

        if (!bFileExists)
        {
            const FString DeletePointsSQL = FString::Printf(TEXT("DELETE FROM PolygonPoints WHERE ShapefileID='%s';"), *DBFile);
            const FString DeleteMetaSQL = FString::Printf(TEXT("DELETE FROM ShapefileMetadata WHERE ShapefileID='%s';"), *DBFile);
            const FString DeleteFeaturesSQL = FString::Printf(TEXT("DELETE FROM PolygonFeatures WHERE ShapefileID='%s';"), *DBFile);

            SQLite->ExecuteWithCallback(DeletePointsSQL, [](const FSQLitePreparedStatement&) { return ESQLitePreparedStatementExecuteRowResult::Continue; });
            SQLite->ExecuteWithCallback(DeleteMetaSQL, [](const FSQLitePreparedStatement&) { return ESQLitePreparedStatementExecuteRowResult::Continue; });
            SQLite->ExecuteWithCallback(DeleteFeaturesSQL, [](const FSQLitePreparedStatement&) { return ESQLitePreparedStatementExecuteRowResult::Continue; });

            UE_LOG(LogTemp, Log, TEXT("Deleted DB entries for removed shapefile: %s"), *DBFile);
        }
    }

    // --- Process each shapefile ---
    for (const FString& ShapefilePath : ShapefilePaths)
    {
        const FString ShapefileName = FPaths::GetBaseFilename(ShapefilePath);
        const int64 FileTimestamp = IFileManager::Get().GetTimeStamp(*ShapefilePath).ToUnixTimestamp();
        bool bNeedsGeneration = true;

        // Check last modified timestamp in DB
        const FString CheckSQL = FString::Printf(TEXT("SELECT LastModified FROM ShapefileMetadata WHERE ShapefileID='%s';"), *ShapefileName);
        SQLite->ExecuteWithCallback(CheckSQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
            {
                int64 SavedTimestamp = 0;
                Statement.GetColumnValueByName(TEXT("LastModified"), SavedTimestamp);

                UE_LOG(LogTemp, Log, TEXT("SavedTimestamp: %lld"), SavedTimestamp);
                UE_LOG(LogTemp, Log, TEXT("FileTimestamp: %lld"), FileTimestamp);

                if (SavedTimestamp == FileTimestamp)
                {
                    bNeedsGeneration = false;
                }
                else
                {
                    const FString DeletePointsSQL = FString::Printf(TEXT("DELETE FROM PolygonPoints WHERE ShapefileID='%s';"), *ShapefileName);
                    SQLite->ExecuteWithCallback(DeletePointsSQL, [](const FSQLitePreparedStatement&) { return ESQLitePreparedStatementExecuteRowResult::Continue; });
                    UE_LOG(LogTemp, Log, TEXT("Deleted old DB points for modified shapefile: %s"), *ShapefileName);
                }

                return ESQLitePreparedStatementExecuteRowResult::Continue;
            });

        // --- Spawn and process polygon content ---
        FActorSpawnParameters Params;
        Params.Owner = this;
        APCGPolygonContent* PolygonContent = GetWorld()->SpawnActor<APCGPolygonContent>(
            APCGPolygonContent::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params);

        if (bNeedsGeneration)
        {
            // Read shapefile raw data
            FShapeRawData RawData = FShapeFileReader::ReadShapefileRawData(ShapefilePath);

            // Insert/Update metadata
            const FString UpdateMetaSQL = FString::Printf(
                TEXT("INSERT OR REPLACE INTO ShapefileMetadata (ShapefileID, LastModified) VALUES ('%s', %lld);"),
                *ShapefileName, FileTimestamp);

            SQLite->ExecuteWithCallback(UpdateMetaSQL, [](const FSQLitePreparedStatement&) { return ESQLitePreparedStatementExecuteRowResult::Continue; });

            // Initialize polygon data and spawn
            PolygonContent->InitializeContent();
            PolygonContent->InitializePCGPolygonData(RawData);
           // PolygonContent->InsertPolygonsFeaturesToDB(RawData.CollectionName);
            PolygonContent->SpawnPCGPolygonData();
        }
        else
        {
            PolygonContent->InitializeContent();
            PolygonContent->SpawnHiGenActorsFromDatabase(ShapefileName);
        }
    }
}

