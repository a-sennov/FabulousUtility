#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "FuBTTask_ActivateAbility.generated.h"

struct FAbilityEndedData;

UCLASS(DisplayName = "Fu Activate Ability", Meta = (ShowWorldContextPin))
class FABULOUSUTILITY_API UFuBTTask_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Settings")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bWaitForAbilityEnd{true};

	UPROPERTY(EditAnywhere, Category = "Settings", Meta = (EditCondition = "bWaitForAbilityEnd"))
	bool bCancelAbilityOnAbort;

public:
	UFuBTTask_ActivateAbility();

	virtual uint16 GetInstanceMemorySize() const override;

	virtual FString GetStaticDescription() const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& BehaviorTree, uint8* NodeMemory) override;

protected:
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& BehaviorTree, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& BehaviorTree, uint8* NodeMemory, EBTNodeResult::Type Result) override;

private:
	void OnAbilityEnded(const FAbilityEndedData& EndedData, TWeakObjectPtr<UBehaviorTreeComponent> BehaviorTree);
};
