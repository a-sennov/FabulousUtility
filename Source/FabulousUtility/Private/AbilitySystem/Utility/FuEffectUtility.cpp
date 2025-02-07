#include "AbilitySystem/Utility/FuEffectUtility.h"

#include "FuMacros.h"
#include "AbilitySystem/FuAbilitySystemComponent.h"

const FGameplayTagContainer& UFuEffectUtility::GetEffectOwnedTags(const TSubclassOf<UGameplayEffect> EffectClass)
{
	if (FU_ENSURE(IsValid(EffectClass)))
	{
		return EffectClass.GetDefaultObject()->InheritableOwnedTagsContainer.CombinedTags;
	}

	static const FGameplayTagContainer None;
	check(None.IsEmpty())

	return None;
}

int32 UFuEffectUtility::GetEffectStackCountByClass(const UFuAbilitySystemComponent* AbilitySystem,
                                                   const TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)) || !FU_ENSURE(IsValid(EffectClass)) ||
	    !FU_ENSURE(EffectClass.GetDefaultObject()->StackingType != EGameplayEffectStackingType::None))
	{
		return 0;
	}

	auto MaxCount{0};

	for (const auto& ActiveEffect : &AbilitySystem->GetActiveEffects())
	{
		if (ActiveEffect.Spec.Def->GetClass() == EffectClass)
		{
			MaxCount = FMath::Max(MaxCount, ActiveEffect.Spec.StackCount);
		}
	}

	return MaxCount;
}

bool UFuEffectUtility::HasAnyActiveEffectsByQuery(const UFuAbilitySystemComponent* AbilitySystem,
                                                  const FGameplayEffectQuery& EffectQuery)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return false;
	}

	for (const auto& ActiveEffect : &AbilitySystem->GetActiveEffects())
	{
		if (EffectQuery.Matches(ActiveEffect))
		{
			return true;
		}
	}

	return false;
}

void UFuEffectUtility::GetActiveEffectsByQuery(const UFuAbilitySystemComponent* AbilitySystem,
                                               const FGameplayEffectQuery& EffectQuery, TArray<FActiveGameplayEffect>& ActiveEffects)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return;
	}

	for (const auto& ActiveEffect : &AbilitySystem->GetActiveEffects())
	{
		if (EffectQuery.Matches(ActiveEffect))
		{
			ActiveEffects.Add(ActiveEffect);
		}
	}
}

bool UFuEffectUtility::HasAnyActiveEffectsWithTag(const UFuAbilitySystemComponent* AbilitySystem,
                                                  const FGameplayTag& EffectTag, const bool bIgnoreInhibitedEffects)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return false;
	}

	for (const auto& ActiveEffect : &AbilitySystem->GetActiveEffects())
	{
		if (ActiveEffect.bIsInhibited && !bIgnoreInhibitedEffects)
		{
			continue;
		}

		if (ActiveEffect.Spec.Def->InheritableOwnedTagsContainer.CombinedTags.HasTag(EffectTag) ||
		    ActiveEffect.Spec.DynamicGrantedTags.HasTag(EffectTag))
		{
			return true;
		}
	}

	return false;
}

bool UFuEffectUtility::HasAnyActiveEffectsWithAnyTags(const UFuAbilitySystemComponent* AbilitySystem,
                                                      const FGameplayTagContainer& EffectTags, const bool bIgnoreInhibitedEffects)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return false;
	}

	for (const auto& ActiveEffect : &AbilitySystem->GetActiveEffects())
	{
		if (ActiveEffect.bIsInhibited && !bIgnoreInhibitedEffects)
		{
			continue;
		}

		if (ActiveEffect.Spec.Def->InheritableOwnedTagsContainer.CombinedTags.HasAny(EffectTags) ||
		    ActiveEffect.Spec.DynamicGrantedTags.HasAny(EffectTags))
		{
			return true;
		}
	}

	return false;
}

int32 UFuEffectUtility::GetEffectsCountWithTag(const UFuAbilitySystemComponent* AbilitySystem,
                                               const FGameplayTag& EffectTag, const bool bIgnoreInhibitedEffects)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return false;
	}

	auto Count{0};

	for (const auto& ActiveEffect : &AbilitySystem->GetActiveEffects())
	{
		if (ActiveEffect.bIsInhibited && !bIgnoreInhibitedEffects)
		{
			continue;
		}

		if (ActiveEffect.Spec.Def->InheritableOwnedTagsContainer.CombinedTags.HasTag(EffectTag) ||
		    ActiveEffect.Spec.DynamicGrantedTags.HasTag(EffectTag))
		{
			Count += ActiveEffect.Spec.StackCount;
		}
	}

	return Count;
}

int32 UFuEffectUtility::GetEffectsCountWithAnyTags(const UFuAbilitySystemComponent* AbilitySystem,
                                                   const FGameplayTagContainer& EffectTags, const bool bIgnoreInhibitedEffects)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return 0;
	}

	auto Count{0};

	for (const auto& ActiveEffect : &AbilitySystem->GetActiveEffects())
	{
		if (ActiveEffect.bIsInhibited && !bIgnoreInhibitedEffects)
		{
			continue;
		}

		if (ActiveEffect.Spec.Def->InheritableOwnedTagsContainer.CombinedTags.HasAny(EffectTags) ||
		    ActiveEffect.Spec.DynamicGrantedTags.HasAny(EffectTags))
		{
			Count += ActiveEffect.Spec.StackCount;
		}
	}

	return Count;
}

void UFuEffectUtility::GetEffectTimeRemainingAndDurationByTag(const UFuAbilitySystemComponent* AbilitySystem,
                                                              const FGameplayTag& EffectTag, float& TimeRemaining, float& Duration)
{
	GetActiveEffectTimeRemainingAndDurationByTag(AbilitySystem, EffectTag, TimeRemaining, Duration);
}

const FActiveGameplayEffect* UFuEffectUtility::GetActiveEffectTimeRemainingAndDurationByTag(
	const UFuAbilitySystemComponent* AbilitySystem, const FGameplayTag& EffectTag, float& TimeRemaining, float& Duration)
{
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return nullptr;
	}

	const auto& ActiveEffects{AbilitySystem->GetActiveEffects()};

	TimeRemaining = -1.0f;
	Duration = -1.0f;

	const FActiveGameplayEffect* Result{nullptr};
	const auto Time{ActiveEffects.GetWorldTime()};

	for (auto& ActiveEffect : &ActiveEffects)
	{
		if (ActiveEffect.Spec.Def->InheritableOwnedTagsContainer.CombinedTags.HasTag(EffectTag) ||
		    ActiveEffect.Spec.DynamicGrantedTags.HasTag(EffectTag))
		{
			const auto OtherDuration{ActiveEffect.GetDuration()};
			const auto OtherTimeRemaining{OtherDuration + ActiveEffect.StartWorldTime - Time};

			if (OtherTimeRemaining > TimeRemaining)
			{
				Result = &ActiveEffect;

				TimeRemaining = OtherTimeRemaining;
				Duration = OtherDuration;
			}
		}
	}

	return Result;
}

bool UFuEffectUtility::IsEffectActive(const FActiveGameplayEffectHandle EffectHandle)
{
	const auto* AbilitySystem{EffectHandle.GetOwningAbilitySystemComponent()};

	return IsValid(AbilitySystem) && AbilitySystem->GetActiveGameplayEffect(EffectHandle) != nullptr;
}

bool UFuEffectUtility::SwitchIsEffectActive(const FActiveGameplayEffectHandle EffectHandle)
{
	return IsEffectActive(EffectHandle);
}

void UFuEffectUtility::RemoveActiveEffect(FActiveGameplayEffectHandle EffectHandle, const int32 StacksToRemove)
{
	auto* AbilitySystem{Cast<UFuAbilitySystemComponent>(EffectHandle.GetOwningAbilitySystemComponent())};
	if (IsValid(AbilitySystem))
	{
		AbilitySystem->RemoveActiveGameplayEffect(EffectHandle, StacksToRemove);
	}
}

void UFuEffectUtility::RecalculateEffectModifiers(FActiveGameplayEffectHandle EffectHandle)
{
	// https://github.com/tranek/GASDocumentation#concepts-ge-definition

	auto* AbilitySystem{Cast<UFuAbilitySystemComponent>(EffectHandle.GetOwningAbilitySystemComponent())};
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return;
	}

	auto& ActiveEffects{AbilitySystem->GetActiveEffects()};

	const auto* ActiveEffect{ActiveEffects.GetActiveGameplayEffect(EffectHandle)};
	if (ActiveEffect != nullptr)
	{
		ActiveEffects.SetActiveGameplayEffectLevel(EffectHandle, ActiveEffect->Spec.GetLevel());
	}
}

void UFuEffectUtility::SetEffectDuration(FActiveGameplayEffectHandle EffectHandle, float Duration)
{
	// https://github.com/tranek/GASDocumentation#concepts-ge-duration

	auto* AbilitySystem{Cast<UFuAbilitySystemComponent>(EffectHandle.GetOwningAbilitySystemComponent())};
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return;
	}

	auto& ActiveEffects{AbilitySystem->GetActiveEffects()};

	auto* ActiveEffect{ActiveEffects.GetActiveGameplayEffect(EffectHandle)};
	if (ActiveEffect == nullptr)
	{
		return;
	}

	Duration = FMath::Max(0.01f, Duration);
	const auto Time{ActiveEffects.GetWorldTime() - ActiveEffect->StartWorldTime};

	ActiveEffect->Spec.Duration = Duration;

	if (Time > Duration)
	{
		ActiveEffect->StartServerWorldTime = ActiveEffects.GetServerWorldTime() - Duration;
		ActiveEffect->CachedStartServerWorldTime = ActiveEffect->StartServerWorldTime;
		ActiveEffect->StartWorldTime = ActiveEffects.GetWorldTime() - Duration;
	}

	ActiveEffects.CheckDuration(EffectHandle);

	ActiveEffect->EventSet.OnTimeChanged.Broadcast(ActiveEffect->Handle, ActiveEffect->StartWorldTime, ActiveEffect->Spec.Duration);

	AbilitySystem->OnGameplayEffectDurationChange(*ActiveEffect);

	ActiveEffects.MarkItemDirty(*ActiveEffect);
}

void UFuEffectUtility::SetEffectTimeRemaining(FActiveGameplayEffectHandle EffectHandle, float TimeRemaining)
{
	auto* AbilitySystem{Cast<UFuAbilitySystemComponent>(EffectHandle.GetOwningAbilitySystemComponent())};
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return;
	}

	auto& ActiveEffects{AbilitySystem->GetActiveEffects()};

	auto* ActiveEffect{ActiveEffects.GetActiveGameplayEffect(EffectHandle)};
	if (ActiveEffect == nullptr)
	{
		return;
	}

	TimeRemaining = FMath::Max(0.0f, TimeRemaining);
	const auto Time{ActiveEffect->GetDuration() - TimeRemaining};

	if (Time >= 0.0f)
	{
		ActiveEffect->StartServerWorldTime = ActiveEffects.GetServerWorldTime() - Time;
		ActiveEffect->CachedStartServerWorldTime = ActiveEffect->StartServerWorldTime;
		ActiveEffect->StartWorldTime = ActiveEffects.GetWorldTime() - Time;
	}
	else
	{
		// If the time remaining is greater than the duration, then adjust the duration to match the time remaining.

		ActiveEffect->Spec.Duration -= Time;

		ActiveEffect->StartServerWorldTime = ActiveEffects.GetServerWorldTime() - TimeRemaining;
		ActiveEffect->CachedStartServerWorldTime = ActiveEffect->StartServerWorldTime;
		ActiveEffect->StartWorldTime = ActiveEffects.GetWorldTime() - TimeRemaining;
	}

	ActiveEffects.CheckDuration(EffectHandle);

	ActiveEffect->EventSet.OnTimeChanged.Broadcast(ActiveEffect->Handle, ActiveEffect->StartWorldTime, ActiveEffect->GetDuration());

	AbilitySystem->OnGameplayEffectDurationChange(*ActiveEffect);

	ActiveEffects.MarkItemDirty(*ActiveEffect);
}

void UFuEffectUtility::IncreaseEffectTimeRemaining(FActiveGameplayEffectHandle EffectHandle, const float AdditionalTimeRemaining)
{
	auto* AbilitySystem{Cast<UFuAbilitySystemComponent>(EffectHandle.GetOwningAbilitySystemComponent())};
	if (!FU_ENSURE(IsValid(AbilitySystem)))
	{
		return;
	}

	const auto& ActiveEffects{AbilitySystem->GetActiveEffects()};

	const auto* ActiveEffect{ActiveEffects.GetActiveGameplayEffect(EffectHandle)};
	if (ActiveEffect != nullptr)
	{
		SetEffectTimeRemaining(EffectHandle, ActiveEffect->GetTimeRemaining(ActiveEffects.GetWorldTime()) + AdditionalTimeRemaining);
	}
}
