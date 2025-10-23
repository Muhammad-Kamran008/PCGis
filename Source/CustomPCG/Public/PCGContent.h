#pragma once

#include "CoreMinimal.h"
#include "windows/WindowsWindow.h"
#undef OPAQUE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Cesium3DTileset.h"
#include "GameFramework/Actor.h"
#include "GISShapeTypes.h"
#include "CesiumGeoreference.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "PCGComponent.h"
#include "GameFramework/Pawn.h"         
#include "Engine/Engine.h"              
#include "UObject/UObjectGlobals.h" // MakeUniqueObjectName
#include "Engine/World.h"
#include "UObject/TopLevelAssetPath.h"
#include "FShapeFileReader.h"
#include "PCGContent.generated.h"




struct FPCGLatLonPoint
{
    double Longitude;
    double Latitude;

    FPCGLatLonPoint() : Longitude(0.0), Latitude(0.0) {}
    FPCGLatLonPoint(double InLongitude, double InLatitude)
        : Longitude(InLongitude), Latitude(InLatitude) {
    }
};





UCLASS()
class CUSTOMPCG_API APCGContent : public AActor
{
	GENERATED_BODY()
	
public:	
	APCGContent();
    virtual void BeginPlay() override;
	virtual void InitializeContent() PURE_VIRTUAL(APCGContent::InitializeContent, );


};
