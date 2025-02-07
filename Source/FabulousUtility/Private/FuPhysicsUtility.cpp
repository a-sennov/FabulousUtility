#include "FuPhysicsUtility.h"

#include "FuMacros.h"
#include "Components/PrimitiveComponent.h"

void UFuPhysicsUtility::GetReachableActorsInRadius(const UObject* WorldContext, const FVector& Center,
                                                   const float Radius, const FCollisionProfileName& CollisionProfile,
                                                   TMap<AActor*, TArray<FHitResult>>& ReachableActors,
                                                   const TFunctionRef<bool(const FOverlapResult& Overlap)> Predicate,
                                                   const AActor* IgnoredActor)
{
	// Based on UGameplayStatics::ApplyRadialDamageWithFalloff().

	auto* World{WorldContext->GetWorld()};
	if (!FU_ENSURE(IsValid(World)))
	{
		return;
	}

	ECollisionChannel CollisionChannel;
	FCollisionResponseParams CollisionResponse;
	if (!UCollisionProfile::GetChannelAndResponseParams(CollisionProfile.Name, CollisionChannel, CollisionResponse))
	{
		return;
	}

	static TArray<FOverlapResult> Overlaps;
	check(Overlaps.IsEmpty())

	World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity,
	                             CollisionChannel, FCollisionShape::MakeSphere(Radius),
	                             {ANSI_TO_TCHAR(__FUNCTION__), false, IgnoredActor}, CollisionResponse);

	FHitResult Hit;

	ReachableActors.Reset();

	for (const auto& Overlap : Overlaps)
	{
		auto* Actor{Overlap.GetActor()};
		if (!IsValid(Actor))
		{
			continue;
		}

		auto* ActorHits{ReachableActors.Find(Actor)};

		// ReSharper disable once CppRedundantParentheses
		if ((ActorHits == nullptr && !Predicate(Overlap)) || !Overlap.Component.IsValid() ||
		    !IsComponentReachableFromLocation(Overlap.Component.Get(), Center, CollisionChannel, CollisionResponse, Hit, IgnoredActor))
		{
			continue;
		}

		if (ActorHits == nullptr)
		{
			ActorHits = &ReachableActors.Add(Actor);
		}

		ActorHits->Add(Hit);
	}

	Overlaps.Reset();
}

bool UFuPhysicsUtility::IsComponentReachableFromLocation(UPrimitiveComponent* Component, const FVector& Location,
                                                         const ECollisionChannel CollisionChannel,
                                                         const FCollisionResponseParams& CollisionResponse,
                                                         FHitResult& Hit, const AActor* IgnoredActor)
{
	if (!FU_ENSURE(IsValid(Component)))
	{
		Hit.Init();
		return false;
	}

	const auto& ComponentBounds{Component->Bounds};

	static const FVector TraceStartOffset{0.0f, 0.0f, 50.0f};
	const FVector TraceEndOffset{0.0f, 0.0f, ComponentBounds.BoxExtent.Z};

	return IsComponentReachableByTrace(Component, Location,
	                                   ComponentBounds.Origin,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location,
	                                   ComponentBounds.Origin + TraceEndOffset,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location,
	                                   ComponentBounds.Origin - TraceEndOffset,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location + TraceStartOffset,
	                                   ComponentBounds.Origin,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location + TraceStartOffset,
	                                   ComponentBounds.Origin + TraceEndOffset,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location + TraceStartOffset,
	                                   ComponentBounds.Origin - TraceEndOffset,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location - TraceStartOffset,
	                                   ComponentBounds.Origin,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location - TraceStartOffset,
	                                   ComponentBounds.Origin + TraceEndOffset,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor) ||
	       IsComponentReachableByTrace(Component, Location - TraceStartOffset,
	                                   ComponentBounds.Origin - TraceEndOffset,
	                                   CollisionChannel, CollisionResponse, Hit, IgnoredActor);
}

bool UFuPhysicsUtility::IsComponentReachableByTrace(UPrimitiveComponent* Component, const FVector& TraceStart,
                                                    const FVector& TraceEnd, const ECollisionChannel CollisionChannel,
                                                    const FCollisionResponseParams& CollisionResponse,
                                                    FHitResult& Hit, const AActor* IgnoredActor)
{
	// Based on UGameplayStatics::ComponentIsDamageableFrom().

	if (!FU_ENSURE(IsValid(Component)))
	{
		Hit.Init();
		return false;
	}

	if (Component->GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, CollisionChannel,
	                                                    {ANSI_TO_TCHAR(__FUNCTION__), true, IgnoredActor}, CollisionResponse))
	{
		return Hit.Component.Get() == Component;
	}

	const auto FakeHitLocation{Component->GetComponentLocation()};
	const auto FakeHitNormal{(TraceStart - FakeHitLocation).GetSafeNormal()};
	Hit = {Component->GetOwner(), Component, FakeHitLocation, FakeHitNormal};

	return true;
}

bool UFuPhysicsUtility::LineTraceBone(UPrimitiveComponent* Primitive, const FName BoneName,
                                      const FVector& TraceStart, const FVector& TraceEnd, FHitResult& Hit)
{
	if (!FU_ENSURE(IsValid(Primitive)))
	{
		Hit.Init();
		return false;
	}

	const auto* BodyInstance{Primitive->GetBodyInstance(BoneName)};

	return BodyInstance != nullptr && BodyInstance->LineTrace(Hit, TraceStart, TraceEnd, false, true);
}

bool UFuPhysicsUtility::BoxOverlapActors(const UObject* WorldContext, const FVector& Location, const FRotator& Rotation,
                                         const FVector& Extent, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
                                         const TArray<AActor*>& IgnoreActors, TArray<AActor*>& Actors)
{
	const auto* World{WorldContext->GetWorld()};
	if (!FU_ENSURE(IsValid(World)))
	{
		return false;
	}

	const auto* CollisionProfile{UCollisionProfile::Get()};

	FCollisionObjectQueryParams ObjectQueryParameters;
	for (const auto ObjectType : ObjectTypes)
	{
		ObjectQueryParameters.AddObjectTypesToQuery(CollisionProfile->ConvertToCollisionChannel(false, ObjectType));
	}

	FCollisionQueryParams QueryParameters{ANSI_TO_TCHAR(__FUNCTION__), false};
	QueryParameters.AddIgnoredActors(IgnoreActors);

	static TArray<FOverlapResult> Overlaps;
	check(Overlaps.IsEmpty())

	World->OverlapMultiByObjectType(Overlaps, Location, Rotation.Quaternion(), ObjectQueryParameters,
	                                FCollisionShape::MakeBox(Extent), QueryParameters);

	Actors.Reset();

	for (const auto& Overlap : Overlaps)
	{
		auto* Actor{Overlap.GetActor()};

		if (IsValid(Actor) && !Actors.Contains(Actor))
		{
			Actors.Add(Actor);
		}
	}

	Overlaps.Reset();

	return Actors.Num() > 0;
}

bool UFuPhysicsUtility::BoxOverlapComponents(const UObject* WorldContext, const FVector& Location, const FRotator& Rotation,
                                             const FVector& Extent, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
                                             const TArray<AActor*>& IgnoreActors, TArray<UPrimitiveComponent*>& Components)
{
	const auto* World{WorldContext->GetWorld()};
	if (!FU_ENSURE(IsValid(World)))
	{
		return false;
	}

	const auto* CollisionProfile{UCollisionProfile::Get()};

	FCollisionObjectQueryParams ObjectQueryParameters;
	for (const auto ObjectType : ObjectTypes)
	{
		ObjectQueryParameters.AddObjectTypesToQuery(CollisionProfile->ConvertToCollisionChannel(false, ObjectType));
	}

	FCollisionQueryParams QueryParameters{ANSI_TO_TCHAR(__FUNCTION__), false};
	QueryParameters.AddIgnoredActors(IgnoreActors);

	static TArray<FOverlapResult> Overlaps;
	check(Overlaps.IsEmpty())

	World->OverlapMultiByObjectType(Overlaps, Location, Rotation.Quaternion(), ObjectQueryParameters,
	                                FCollisionShape::MakeBox(Extent), QueryParameters);

	Components.Reset();

	for (const auto& Overlap : Overlaps)
	{
		if (Overlap.Component.IsValid())
		{
			Components.Add(Overlap.Component.Get());
		}
	}

	Overlaps.Reset();

	return Components.Num() > 0;
}

bool UFuPhysicsUtility::ConeOverlapActorsSimple(const UObject* WorldContext, const FVector& Location,
                                                const FRotator& Rotation, const float Radius, const float Angle,
                                                const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
                                                const TArray<AActor*>& IgnoreActors, TArray<AActor*>& Actors)
{
	const auto* World{WorldContext->GetWorld()};
	if (!FU_ENSURE(IsValid(World)))
	{
		return false;
	}

	const auto* CollisionProfile{UCollisionProfile::Get()};

	FCollisionObjectQueryParams ObjectQueryParameters;
	for (const auto ObjectType : ObjectTypes)
	{
		ObjectQueryParameters.AddObjectTypesToQuery(CollisionProfile->ConvertToCollisionChannel(false, ObjectType));
	}

	FCollisionQueryParams QueryParameters{ANSI_TO_TCHAR(__FUNCTION__), false};
	QueryParameters.AddIgnoredActors(IgnoreActors);

	static TArray<FOverlapResult> Overlaps;
	check(Overlaps.IsEmpty())

	World->OverlapMultiByObjectType(Overlaps, Location, Rotation.Quaternion(), ObjectQueryParameters,
	                                FCollisionShape::MakeSphere(Radius), QueryParameters);

	const auto Direction{Rotation.Vector()};
	const auto AngleCos{FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(Angle, 0.0f, 180.0f)))};

	Actors.Reset();

	for (const auto& Overlap : Overlaps)
	{
		auto* Actor{Overlap.GetActor()};

		if (IsValid(Actor) && !Actors.Contains(Actor) &&
		    (Direction | (Overlap.Component->GetComponentLocation() - Location).GetSafeNormal()) >= AngleCos)
		{
			Actors.Add(Actor);
		}
	}

	Overlaps.Reset();

	return Actors.Num() > 0;
}

bool UFuPhysicsUtility::ConeOverlapComponentsSimple(const UObject* WorldContext, const FVector& Location,
                                                    const FRotator& Rotation, const float Radius, const float Angle,
                                                    const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
                                                    const TArray<AActor*>& IgnoreActors, TArray<UPrimitiveComponent*>& Components)
{
	const auto* World{WorldContext->GetWorld()};
	if (!FU_ENSURE(IsValid(World)))
	{
		return false;
	}

	const auto* CollisionProfile{UCollisionProfile::Get()};

	FCollisionObjectQueryParams ObjectQueryParameters;
	for (const auto ObjectType : ObjectTypes)
	{
		ObjectQueryParameters.AddObjectTypesToQuery(CollisionProfile->ConvertToCollisionChannel(false, ObjectType));
	}

	FCollisionQueryParams QueryParameters{ANSI_TO_TCHAR(__FUNCTION__), false};
	QueryParameters.AddIgnoredActors(IgnoreActors);

	static TArray<FOverlapResult> Overlaps;
	check(Overlaps.IsEmpty())

	World->OverlapMultiByObjectType(Overlaps, Location, Rotation.Quaternion(), ObjectQueryParameters,
	                                FCollisionShape::MakeSphere(Radius), QueryParameters);

	const auto Direction{Rotation.Vector()};
	const auto AngleCos{FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(Angle, 0.0f, 180.0f)))};

	Components.Reset();

	for (const auto& Overlap : Overlaps)
	{
		if (Overlap.Component.IsValid() &&
		    (Direction | (Overlap.Component->GetComponentLocation() - Location).GetSafeNormal()) >= AngleCos)
		{
			Components.Add(Overlap.Component.Get());
		}
	}

	Overlaps.Reset();

	return Components.Num() > 0;
}
