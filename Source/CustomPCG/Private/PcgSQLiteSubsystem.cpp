#include "PcgSQLiteSubsystem.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

void UPcgSQLiteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    OpenDatabase();
}

void UPcgSQLiteSubsystem::Deinitialize()
{
    CloseDatabase();
    Super::Deinitialize();
}

bool UPcgSQLiteSubsystem::OpenDatabase(const FString& InRelativePath)
{
    FString Relative = InRelativePath;
    if (Relative.StartsWith(TEXT("/")) || Relative.StartsWith(TEXT("\\")))
    {
        Relative = Relative.RightChop(1);
    }

    FString DBPath = FPaths::ProjectContentDir() / Relative;
    FPaths::NormalizeFilename(DBPath);
    DbPathFull = DBPath;

    Lock();
    bool bOk = DB.Open(*DBPath, OpenMode);
    Unlock();

    if (!bOk)
    {
        UE_LOG(LogTemp, Error, TEXT("PcgSQLiteSubsystem: Failed to open/create DB at %s"), *DBPath);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("PcgSQLiteSubsystem: DB opened at %s"), *DBPath);
    return true;
}

void UPcgSQLiteSubsystem::CloseDatabase()
{
    Lock();
    if (DB.IsValid())
    {
        DB.Close();
        UE_LOG(LogTemp, Log, TEXT("PcgSQLiteSubsystem: DB closed"));
    }
    Unlock();
}

bool UPcgSQLiteSubsystem::IsOpen() const
{
    Lock();
    bool b = DB.IsValid();
    Unlock();
    return b;
}

bool UPcgSQLiteSubsystem::Execute(const FString& Sql)
{
   // Lock();
    bool bOk = DB.Execute(*Sql);
    if (!bOk)
    {
        UE_LOG(LogTemp, Error, TEXT("PcgSQLiteSubsystem::Execute failed: %s"), *Sql);
    }
   // Unlock();
    return bOk;
}

bool UPcgSQLiteSubsystem::ExecuteWithCallback(
    const FString& Sql,
    TFunctionRef<ESQLitePreparedStatementExecuteRowResult(const FSQLitePreparedStatement&)> RowCallback)
{
    Lock();

    FSQLitePreparedStatement Statement;
    if (!Statement.Create(DB, *Sql))
    {
        Unlock();
        UE_LOG(LogTemp, Error, TEXT("Failed to create SQLite statement: %s"), *Sql);
        return false;
    }

    int64 Result = Statement.Execute([&](const FSQLitePreparedStatement& Stmt) -> ESQLitePreparedStatementExecuteRowResult
        {
            return RowCallback(Stmt);
        });

    Statement.Destroy();
    Unlock();

    if (Result == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("PcgSQLiteSubsystem::ExecuteWithCallback failed: %s"), *Sql);
        return false;
    }

    return true;
}

bool UPcgSQLiteSubsystem::BeginTransaction()
{
    return Execute(TEXT("BEGIN TRANSACTION;"));
}

bool UPcgSQLiteSubsystem::CommitTransaction()
{
    return Execute(TEXT("COMMIT;"));
}

bool UPcgSQLiteSubsystem::RollbackTransaction()
{
    return Execute(TEXT("ROLLBACK;"));
}

FSQLiteDatabase& UPcgSQLiteSubsystem::GetDatabase()
{
   return DB;
}
