#pragma once

#include "Engine/CollisionProfile.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FuPhysicsUtility.generated.h"

UCLASS()
class FABULOUSUTILITY_API UFuPhysicsUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static void GetReachableActorsInRadius(const UObject* WorldContext, const FVector& Center,
	                                       float Radius, const FCollisionProfileName& CollisionProfile,
	                                       TMap<AActor*, TArray<FHitResult>>& ReachableActors,
	                                       TFunctionRef<bool(const FOverlapResult& Overlap)> Predicate,
	                                       const AActor* IgnoredActor = nullptr);

	static bool IsComponentReachableFromLocation(UPrimitiveComponent* Component, const FVector& Location,
	                                             ECollisionChannel CollisionChannel, const FCollisionResponseParams& CollisionResponse,
	                                             FHitResult& Hit, const AActor* IgnoredActor = nullptr);

	static bool IsComponentReachableByTrace(UPrimitiveComponent* Component, const FVector& TraceStart,
	                                        const FVector& TraceEnd, ECollisionChannel CollisionChannel,
	                                        const FCollisionResponseParams& CollisionResponse,
	                                        FHitResult& Hit, const AActor* IgnoredActor = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Fabulous Utility|Fu Physics Utility",
		Meta = (ExpandBoolAsExecs = "ReturnValue", AutoCreateRefTerm = "TraceStart, TraceEnd"))
	static bool LineTraceBone(UPrimitiveComponent* Primitive, FName BoneName,
	                          const FVector& TraceStart, const FVector& TraceEnd, FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Fabulous Utility|Fu Physics Utility",
		Meta = (WorldContext = "WorldContext", AutoCreateRefTerm = "Location, Rotation, Extent, IgnoreActors"))
	static bool BoxOverlapActors(const UObject* WorldContext, const FVector& Location, const FRotator& Rotation,
	                             const FVector& Extent, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	                             const TArray<AActor*>& IgnoreActors, TArray<AActor*>& Actors);

	UFUNCTION(BlueprintCallable, Category = "Fabulous Utility|Fu Physics Utility",
		Meta = (WorldContext = "WorldContext", AutoCreateRefTerm = "Location, Rotation, Extent, IgnoreActors"))
	static bool BoxOverlapComponents(const UObject* WorldContext, const FVector& Location, const FRotator& Rotation,
	                                 const FVector& Extent, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	                                 const TArray<AActor*>& IgnoreActors, TArray<UPrimitiveComponent*>& Components);

	UFUNCTION(BlueprintCallable, Category = "Fabulous Utility|Fu Physics Utility",
		Meta = (WorldContext = "WorldContext", AutoCreateRefTerm = "Location, Rotation, IgnoreActors"))
	static bool ConeOverlapActorsSimple(const UObject* WorldContext, const FVector& Location, const FRotator& Rotation,
	                                    float Radius, float Angle, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	                                    const TArray<AActor*>& IgnoreActors, TArray<AActor*>& Actors);

	UFUNCTION(BlueprintCallable, Category = "Fabulous Utility|Fu Physics Utility",
		Meta = (WorldContext = "WorldContext", AutoCreateRefTerm = "Location, Rotation, IgnoreActors"))
	static bool ConeOverlapComponentsSimple(const UObject* WorldContext, const FVector& Location, const FRotator& Rotation,
	                                        float Radius, float Angle, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	                                        const TArray<AActor*>& IgnoreActors, TArray<UPrimitiveComponent*>& Components);
};
