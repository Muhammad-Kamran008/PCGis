// GISShapeTypes.h

#pragma once

#include "CoreMinimal.h"
#include "GISShapeTypes.generated.h"

USTRUCT(BlueprintType)
struct FPolygonRing
{
    GENERATED_BODY()

    TArray<FVector> Points;
};

USTRUCT(BlueprintType)
struct FShapeAttribute
{
    GENERATED_BODY()
    FString Key;

    FString Value;
};

USTRUCT(BlueprintType)
struct FShapeFeature
{
    GENERATED_BODY()


    TArray<FPolygonRing> PolygonRings;

    TArray<FShapeAttribute> Attributes;

};


USTRUCT(BlueprintType)
struct FShapeFeatureGroup
{
    GENERATED_BODY()

    FString CollectionName;

    TArray<FShapeFeature> Features;
};



