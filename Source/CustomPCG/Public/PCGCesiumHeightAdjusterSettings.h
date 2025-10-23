// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCGContent.h"
#include "PCGSettings.h"
#include "PCGSubSystem.h"
#include "PCGElement.h"
#include "PCGContext.h"

#include "Async/Async.h" // For AsyncTask
#include <mutex>



#include "PCGCesiumHeightAdjusterSettings.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class CUSTOMPCG_API UPCGCesiumHeightAdjusterSettings : public UPCGSettings
{
	GENERATED_BODY()
	

public:
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override;
	virtual FText GetDefaultNodeTitle() const override;
	virtual FText GetNodeTooltipText() const override;
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif //WITH_EDITOR

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:

	/** Maximum and Minimum heights to perform the trace **/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cesium PCG", meta = (PCG_Overridable))
	float MaxHeightTrace = 100000.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cesium PCG", meta = (PCG_Overridable))
	float MinHeightTrace = -200000.0f;

};
class FPCGCesiumHeightAdjusterElement : public IPCGElement
{

public:
	bool isUpdate = false;




	//bool getIsUpdated();
	//void setIsUpdated(bool isUpdated);


protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;

};
