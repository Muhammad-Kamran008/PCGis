// Fill out your copyright notice in the Description page of Project Settings.


#include "PCGDBPointsReaderSettings.h"
#include "Data/PCGPointData.h"
#include "PCGComponent.h"
#include "PCGParamData.h"
#include "SQLiteDatabase.h"

TArray<FPCGPinProperties> UPCGDBPointsReaderSettings::InputPinProperties() const
{
	return {
	FPCGPinProperties(TEXT("PolygonID"), EPCGDataType::Param),
	FPCGPinProperties(TEXT("InputNodeOutput"), EPCGDataType::Spatial) 
	};
}


TArray<FPCGPinProperties> UPCGDBPointsReaderSettings::OutputPinProperties() const
{
	return { FPCGPinProperties(TEXT("Points"), EPCGDataType::Spatial) };
}

FPCGElementPtr UPCGDBPointsReaderSettings::CreateElement() const
{
	return MakeShared<FPCGDBPointsReaderElement>();
}

bool FPCGDBPointsReaderElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGDBPointsReaderElement::Execute);
	check(Context);

	if (!Context->SourceComponent.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("DBPointsReader: Invalid SourceComponent"));
		return true;
	}

	UObject* SourceComponentObj = Context->SourceComponent.Get();
	if (!SourceComponentObj) return true;

	FString PolygonID;
	const TArray<FPCGTaggedData> Inputs = Context->InputData.GetParamsByPin(TEXT("PolygonID"));
	if (Inputs.Num() > 0)
	{

		const UPCGParamData* ParamData = Cast<const UPCGParamData>(Inputs[0].Data);
		if (ParamData && ParamData->Metadata)
		{
			// Use GetConstTypedAttribute<FString> to access string attributes
			if (const FPCGMetadataAttribute<FString>* Attr = ParamData->Metadata->GetConstTypedAttribute<FString>(TEXT("PolygonID")))
			{
				if (Attr->GetNumberOfEntries() > 0)
				{
					PolygonID = Attr->GetValue(0); // index 0 for the first entry
				}
			}
		}		if (PolygonID.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("DBPointsReader node: PolygonID is empty"));
			return true;
		}

	}

	//if (PolygonID.IsEmpty())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("DBPointsReader: PolygonID is empty"));
	//	return true;
	//}

	TArray<FPCGPoint> Points;
	FSQLiteDatabase DB;
	const FString DBPath = FPaths::ProjectContentDir() / TEXT("PCGData/PolygonData.db");

	if (!DB.Open(*DBPath))
	{
		UE_LOG(LogTemp, Error, TEXT("DBPointsReader: Failed to open database at %s"), *DBPath);
		return true;
	}

	//const FString SQL = FString::Printf(
	//	TEXT("SELECT X, Y, Z, MeshID FROM PolygonPoints WHERE PolygonID='%s';"), *PolygonID);

    const FString SQL = FString::Printf(
	TEXT("SELECT X, Y, Z, MeshID FROM PolygonPoints WHERE PolygonID=1;"));

	DB.Execute(*SQL, [&](const FSQLitePreparedStatement& Statement) -> ESQLitePreparedStatementExecuteRowResult
		{
			FVector Loc;
			if (!Statement.GetColumnValueByName(TEXT("X"), Loc.X) ||
				!Statement.GetColumnValueByName(TEXT("Y"), Loc.Y) ||
				!Statement.GetColumnValueByName(TEXT("Z"), Loc.Z))
			{
				//UE_LOG(LogTemp, Warning, TEXT("DBPointsReader: Failed to read point from DB for PolygonID %s"), *PolygonID);
				return ESQLitePreparedStatementExecuteRowResult::Continue;
			}

			FPCGPoint P;
			P.Transform.SetLocation(Loc);
			Points.Add(P);
			return ESQLitePreparedStatementExecuteRowResult::Continue;
		});

	DB.Close();

	//UE_LOG(LogTemp, Log, TEXT("DBPointsReader: Loaded %d points for PolygonID %s"), Points.Num(), *PolygonID);

	// --- Step 3: Output PCG point data ---
	UPCGPointData* OutputData = NewObject<UPCGPointData>(SourceComponentObj);
	OutputData->GetMutablePoints() = Points;

	FPCGTaggedData& Output = Context->OutputData.TaggedData.AddDefaulted_GetRef();
	Output.Data = OutputData;

	return true; // synchronous execution
}
