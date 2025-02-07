#include "AI/FuBTTask_ActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "FuMacros.h"

struct FFuActivateAbilityMemory
{
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;

	TSet<FGameplayAbilitySpecHandle> ActiveAbilityHandles;

	bool bAnyAbilitySuccessfullyEnded{false};
};

UFuBTTask_ActivateAbility::UFuBTTask_ActivateAbility()
{
	NodeName = "Fu Activate Ability";
	bIgnoreRestartSelf = true;

	INIT_TASK_NODE_NOTIFY_FLAGS();
}

uint16 UFuBTTask_ActivateAbility::GetInstanceMemorySize() const
{
	return sizeof(FFuActivateAbilityMemory);
}

FString UFuBTTask_ActivateAbility::GetStaticDescription() const
{
	FString Description;

	if (bWaitForAbilityEnd)
	{
		Description = bCancelAbilityOnAbort
			              ? TEXT("( wait for end, cancel on abort )") LINE_TERMINATOR
			              : TEXT("( wait for end )") LINE_TERMINATOR;
	}

	return Description + FString::Printf(TEXT("Activate Ability: %s"), *AbilityTag.ToString());
}

#if WITH_EDITOR
FName UFuBTTask_ActivateAbility::GetNodeIconName() const
{
	return TEXT("BTEditor.Graph.BTNode.Task.RunEQSQuery.Icon");
}
#endif

EBTNodeResult::Type UFuBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& BehaviorTree, uint8* NodeMemory)
{
	if (!FU_ENSURE(AbilityTag.IsValid()))
	{
		return EBTNodeResult::Failed;
	}

	auto& Memory{*CastInstanceNodeMemory<FFuActivateAbilityMemory>(NodeMemory)};

	check(!Memory.AbilitySystem.IsValid())
	check(Memory.ActiveAbilityHandles.IsEmpty())
	check(!Memory.bAnyAbilitySuccessfullyEnded)

	const auto* Controller{BehaviorTree.GetAIOwner()};
	const auto* Pawn{IsValid(Controller) ? Controller->GetPawn() : nullptr};
	Memory.AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);

	if (!FU_ENSURE(Memory.AbilitySystem.IsValid()))
	{
		return EBTNodeResult::Failed;
	}

	const auto AbilityEndedHandle{
		Memory.AbilitySystem->OnAbilityEnded.AddLambda([&Memory](const FAbilityEndedData& EndedData)
		{
			if (Memory.ActiveAbilityHandles.Remove(EndedData.AbilitySpecHandle) > 0)
			{
				Memory.bAnyAbilitySuccessfullyEnded |= !EndedData.bWasCancelled;
			}
		})
	};

	{
		// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
		FScopedAbilityListLock AbilitiesScopeLock{*Memory.AbilitySystem};

		for (const auto& AbilitySpecification : Memory.AbilitySystem->GetActivatableAbilities())
		{
			if (!AbilitySpecification.Ability->AbilityTags.HasTag(AbilityTag))
			{
				continue;
			}

			Memory.ActiveAbilityHandles.Add(AbilitySpecification.Handle);

			if (!Memory.AbilitySystem->TryActivateAbility(AbilitySpecification.Handle))
			{
				Memory.ActiveAbilityHandles.Remove(AbilitySpecification.Handle);
			}
		}
	}

	Memory.AbilitySystem->OnAbilityEnded.Remove(AbilityEndedHandle);

	if (!bWaitForAbilityEnd)
	{
		return Memory.bAnyAbilitySuccessfullyEnded || Memory.ActiveAbilityHandles.Num() > 0
			       ? EBTNodeResult::Succeeded
			       : EBTNodeResult::Failed;
	}

	if (Memory.ActiveAbilityHandles.IsEmpty())
	{
		return Memory.bAnyAbilitySuccessfullyEnded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	}

	Memory.AbilitySystem->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded,
	                                                TWeakObjectPtr<UBehaviorTreeComponent>{&BehaviorTree});

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UFuBTTask_ActivateAbility::AbortTask(UBehaviorTreeComponent& BehaviorTree, uint8* NodeMemory)
{
	const auto& Memory{*CastInstanceNodeMemory<FFuActivateAbilityMemory>(NodeMemory)};

	if (bCancelAbilityOnAbort && Memory.AbilitySystem.IsValid())
	{
		Memory.AbilitySystem->OnAbilityEnded.RemoveAll(this);

		for (const auto AbilityHandle : Memory.ActiveAbilityHandles)
		{
			Memory.AbilitySystem->CancelAbilityHandle(AbilityHandle);
		}
	}

	return EBTNodeResult::Aborted;
}

void UFuBTTask_ActivateAbility::OnTaskFinished(UBehaviorTreeComponent& BehaviorTree, uint8* NodeMemory, const EBTNodeResult::Type Result)
{
	auto& Memory{*CastInstanceNodeMemory<FFuActivateAbilityMemory>(NodeMemory)};

	if (Memory.AbilitySystem.IsValid())
	{
		Memory.AbilitySystem->OnAbilityEnded.RemoveAll(this);
		Memory.AbilitySystem = nullptr;
	}

	Memory.ActiveAbilityHandles.Reset();
	Memory.bAnyAbilitySuccessfullyEnded = false;

	Super::OnTaskFinished(BehaviorTree, NodeMemory, Result);
}

void UFuBTTask_ActivateAbility::OnAbilityEnded(const FAbilityEndedData& EndedData,
                                               const TWeakObjectPtr<UBehaviorTreeComponent> BehaviorTree)
{
	if (FU_ENSURE(BehaviorTree.IsValid()))
	{
		auto& Memory{
			*CastInstanceNodeMemory<FFuActivateAbilityMemory>(
				BehaviorTree->GetNodeMemory(this, BehaviorTree->FindInstanceContainingNode(this)))
		};

		if (Memory.ActiveAbilityHandles.Remove(EndedData.AbilitySpecHandle) <= 0)
		{
			return;
		}

		Memory.bAnyAbilitySuccessfullyEnded |= !EndedData.bWasCancelled;

		if (Memory.ActiveAbilityHandles.IsEmpty())
		{
			FinishLatentTask(*BehaviorTree, Memory.bAnyAbilitySuccessfullyEnded ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
		}
	}
}
