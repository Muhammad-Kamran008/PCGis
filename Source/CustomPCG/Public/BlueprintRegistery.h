// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BlueprintRegistery.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMPCG_API UBlueprintRegistery : public UDataAsset
{
	GENERATED_BODY()


public:
	// Array of Blueprint class soft references
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TSoftClassPtr<AActor>> BlueprintClasses;
	
};
