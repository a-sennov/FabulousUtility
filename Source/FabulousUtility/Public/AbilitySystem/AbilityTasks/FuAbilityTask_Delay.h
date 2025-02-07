#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "FuAbilityTask_Delay.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFuDelayEndedDelegate, int32, LoopIndex);

UCLASS(DisplayName = "Fu Delay Ability Task")
class FABULOUSUTILITY_API UFuAbilityTask_Delay : public UAbilityTask
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient,
		Meta = (AllowPrivateAccess, ClampMin = 0, DisplayName = "Duration", ForceUnits = "s"))
	float Duration1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, DisplayName = "Loop Count", Meta = (AllowPrivateAccess))
	int32 LoopsCount1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient,
		DisplayName = "Skip First Delay", Meta = (AllowPrivateAccess))
	bool bSkipFirstDelay1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (AllowPrivateAccess, ClampMin = 0))
	int32 LoopIndex;

	FTimerHandle TimerHandle;

public:
	UPROPERTY(BlueprintAssignable, Category = "Fabulous Utility|Fu Delay Ability Task")
	FFuDelayEndedDelegate OnLoop;

	UPROPERTY(BlueprintAssignable, Category = "Fabulous Utility|Fu Delay Ability Task")
	FFuDelayEndedDelegate OnDelayEnded;

public:
	UFUNCTION(BlueprintCallable, Category = "Fabulous Utility|Fu Ability", BlueprintInternalUseOnly,
		Meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility"))
	static UFuAbilityTask_Delay* FuWaitForDelay(UGameplayAbility* OwningAbility, float Duration = 0.2f,
	                                            int32 LoopsCount = 0, bool bSkipFirstDelay = false);

protected:
	virtual void Activate() override;

public:
	virtual FString GetDebugString() const override;

protected:
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	void OnTimerEnded();
};
