// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SQLiteDatabase.h" 
#include "PcgSQLiteSubsystem.generated.h"


UCLASS()
class CUSTOMPCG_API UPcgSQLiteSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    bool OpenDatabase(const FString& InRelativePath = TEXT("PCGData/PolygonData.db"));
    void CloseDatabase();

    bool IsOpen() const;

    bool Execute(const FString& Sql);

    bool ExecuteWithCallback(const FString& Sql, TFunctionRef<ESQLitePreparedStatementExecuteRowResult(const FSQLitePreparedStatement&)> RowCallback);

    // Execute SQL with row callback (for SELECTs). The callback receives the prepared statement
    // and should return ESQLitePreparedStatementExecuteRowResult::Continue/Stop
    //bool ExecuteWithCallback(const FString& Sql, TFunctionRef<ESQLitePreparedStatementExecuteRowResult(const FSQLitePreparedStatement&)> RowCallback);


    bool BeginTransaction();
    bool CommitTransaction();
    bool RollbackTransaction();

    FSQLiteDatabase& GetDatabase();

private:
    FString DbPathFull;

    FSQLiteDatabase DB;

    mutable FCriticalSection DbCriticalSection;

    ESQLiteDatabaseOpenMode OpenMode = ESQLiteDatabaseOpenMode::ReadWriteCreate;

    void Lock() const { DbCriticalSection.Lock(); }
    void Unlock() const { DbCriticalSection.Unlock(); }
};
