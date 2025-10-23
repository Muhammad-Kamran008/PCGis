#include "CoreMinimal.h"
#include "PCGContent.h"
#include "PCGSettings.h"
#include "PCGSubSystem.h"
#include "PCGElement.h"
#include "PCGContext.h"
#include "PCGDBPointsReaderSettings.generated.h"

UCLASS(BlueprintType)
class CUSTOMPCG_API UPCGDBPointsReaderSettings : public UPCGSettings
{
	GENERATED_BODY()


public:
#if WITH_EDITOR
    virtual FName GetDefaultNodeName() const override { return FName(TEXT("PCGDBPointsReader")); }
    virtual FText GetDefaultNodeTitle() const override { return FText::FromString("PCG DB Points Reader"); }
    virtual FText GetNodeTooltipText() const override { return FText::FromString("Reads polygon points from SQLite DB by PolygonID"); }
    virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

protected:
    virtual TArray<FPCGPinProperties> InputPinProperties() const override;

    virtual TArray<FPCGPinProperties> OutputPinProperties() const override;


    virtual FPCGElementPtr CreateElement() const override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|DB")
    FString ShapefileID;


};
class FPCGDBPointsReaderElement : public IPCGElement
{

public:
	bool isUpdate = false;

protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

};


