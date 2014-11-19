// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "AIModulePrivate.h"
#include "BlueprintNodeHelpers.h"
#include "BehaviorTree/Decorators/BTDecorator_BlueprintBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

UBTDecorator_BlueprintBase::UBTDecorator_BlueprintBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	UClass* StopAtClass = UBTDecorator_BlueprintBase::StaticClass();
	ReceiveTickImplementations = FBTNodeBPImplementationHelper::CheckEventImplementationVersion(TEXT("ReceiveTick"), TEXT("ReceiveTickAI"), this, StopAtClass);
	ReceiveExecutionStartImplementations = FBTNodeBPImplementationHelper::CheckEventImplementationVersion(TEXT("ReceiveExecutionStart"), TEXT("ReceiveExecutionStartAI"), this, StopAtClass);
	ReceiveExecutionFinishImplementations = FBTNodeBPImplementationHelper::CheckEventImplementationVersion(TEXT("ReceiveExecutionFinish"), TEXT("ReceiveExecutionFinishAI"), this, StopAtClass);
	ReceiveObserverActivatedImplementations = FBTNodeBPImplementationHelper::CheckEventImplementationVersion(TEXT("ReceiveObserverActivated"), TEXT("ReceiveObserverActivatedAI"), this, StopAtClass);
	ReceiveObserverDeactivatedImplementations = FBTNodeBPImplementationHelper::CheckEventImplementationVersion(TEXT("ReceiveObserverDeactivated"), TEXT("ReceiveObserverDeactivatedAI"), this, StopAtClass);
	ReceiveConditionCheckImplementations = FBTNodeBPImplementationHelper::CheckEventImplementationVersion(TEXT("ReceiveConditionCheck"), TEXT("ReceiveConditionCheckAI"), this, StopAtClass);

	bNotifyBecomeRelevant = ReceiveObserverActivatedImplementations != 0;
	bNotifyCeaseRelevant = ReceiveObserverDeactivatedImplementations != 0;
	bNotifyTick = ReceiveTickImplementations != 0;
	bNotifyActivation = ReceiveExecutionStartImplementations != 0;
	bNotifyDeactivation = ReceiveExecutionFinishImplementations != 0;
	bShowPropertyDetails = true;

	// all blueprint based nodes must create instances
	bCreateNodeInstance = true;

	InitializeProperties();
}

void UBTDecorator_BlueprintBase::InitializeProperties()
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UClass* StopAtClass = UBTDecorator_BlueprintBase::StaticClass();
		BlueprintNodeHelpers::CollectPropertyData(this, StopAtClass, PropertyData);

		if (GetFlowAbortMode() != EBTFlowAbortMode::None)
		{
			// find all blackboard key selectors and store their names
			BlueprintNodeHelpers::CollectBlackboardSelectors(this, StopAtClass, ObservedKeyNames);
		}
	}
	else
	{
		UBTDecorator_BlueprintBase* CDO = (UBTDecorator_BlueprintBase*)(GetClass()->GetDefaultObject());
		if (CDO && CDO->ObservedKeyNames.Num())
		{
			bNotifyBecomeRelevant = true;
			bNotifyCeaseRelevant = true;
		}
	}
}

void UBTDecorator_BlueprintBase::PostInitProperties()
{
	Super::PostInitProperties();
	NodeName = BlueprintNodeHelpers::GetNodeName(this);
	BBKeyObserver = FOnBlackboardChange::CreateUObject(this, &UBTDecorator_BlueprintBase::OnBlackboardChange);
}

void UBTDecorator_BlueprintBase::SetOwner(AActor* InActorOwner)
{
	ActorOwner = InActorOwner;
	AIOwner = Cast<AAIController>(InActorOwner);
}

void UBTDecorator_BlueprintBase::OnBecomeRelevant(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory)
{
	if (AIOwner != nullptr && ReceiveObserverActivatedImplementations & FBTNodeBPImplementationHelper::AISpecific)
	{
		ReceiveObserverActivatedAI(AIOwner, AIOwner->GetPawn());
	}
	else if (ReceiveObserverActivatedImplementations & FBTNodeBPImplementationHelper::Generic)
	{
		ReceiveObserverActivated(ActorOwner);
	}

	UBlackboardComponent* BlackboardComp = OwnerComp->GetBlackboardComponent();
	UBTDecorator_BlueprintBase* CDO = (UBTDecorator_BlueprintBase*)(GetClass()->GetDefaultObject());
	if (BlackboardComp && CDO)
	{
		for (int32 NameIndex = 0; NameIndex < CDO->ObservedKeyNames.Num(); NameIndex++)
		{
			const FBlackboard::FKey KeyID = BlackboardComp->GetKeyID(CDO->ObservedKeyNames[NameIndex]);
			if (KeyID != FBlackboard::InvalidKey)
			{
				BlackboardComp->RegisterObserver(KeyID, BBKeyObserver);
			}
		}
	}
}

void UBTDecorator_BlueprintBase::OnCeaseRelevant(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp->GetBlackboardComponent();
	UBTDecorator_BlueprintBase* CDO = (UBTDecorator_BlueprintBase*)(GetClass()->GetDefaultObject());
	if (BlackboardComp && CDO)
	{
		for (int32 NameIndex = 0; NameIndex < CDO->ObservedKeyNames.Num(); NameIndex++)
		{
			const FBlackboard::FKey KeyID = BlackboardComp->GetKeyID(CDO->ObservedKeyNames[NameIndex]);
			if (KeyID != FBlackboard::InvalidKey)
			{
				BlackboardComp->UnregisterObserver(KeyID, BBKeyObserver);
			}
		}
	}
		
	if (AIOwner != nullptr && ReceiveObserverDeactivatedImplementations & FBTNodeBPImplementationHelper::AISpecific)
	{
		ReceiveObserverDeactivatedAI(AIOwner, AIOwner->GetPawn());
	}
	else if (ReceiveObserverDeactivatedImplementations & FBTNodeBPImplementationHelper::Generic)
	{
		ReceiveObserverDeactivated(ActorOwner);
	}
}

void UBTDecorator_BlueprintBase::OnNodeActivation(FBehaviorTreeSearchData& SearchData)
{
	if (AIOwner != nullptr && ReceiveExecutionStartImplementations & FBTNodeBPImplementationHelper::AISpecific)
	{
		ReceiveExecutionStartAI(AIOwner, AIOwner->GetPawn());
	}
	else if (ReceiveExecutionStartImplementations & FBTNodeBPImplementationHelper::Generic)
	{
		ReceiveExecutionStart(ActorOwner);
	}
}

void UBTDecorator_BlueprintBase::OnNodeDeactivation(FBehaviorTreeSearchData& SearchData, EBTNodeResult::Type NodeResult)
{
	if (AIOwner != nullptr && ReceiveExecutionFinishImplementations & FBTNodeBPImplementationHelper::AISpecific)
	{
		ReceiveExecutionFinishAI(AIOwner, AIOwner->GetPawn(), NodeResult);
	}
	else if (ReceiveExecutionFinishImplementations & FBTNodeBPImplementationHelper::Generic)
	{
		ReceiveExecutionFinish(ActorOwner, NodeResult);
	}
}

void UBTDecorator_BlueprintBase::TickNode(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (AIOwner != nullptr && ReceiveTickImplementations & FBTNodeBPImplementationHelper::AISpecific)
	{
		ReceiveTickAI(AIOwner, AIOwner->GetPawn(), DeltaSeconds);
	}
	else if (ReceiveTickImplementations & FBTNodeBPImplementationHelper::Generic)
	{
		ReceiveTick(ActorOwner, DeltaSeconds);
	}
}

bool UBTDecorator_BlueprintBase::CalculateRawConditionValue(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) const
{
	CurrentCallResult = false;
	if (ReceiveConditionCheckImplementations != 0)
	{
		// can't use const functions with blueprints
		UBTDecorator_BlueprintBase* MyNode = (UBTDecorator_BlueprintBase*)this;

		if (AIOwner != nullptr && ReceiveConditionCheckImplementations & FBTNodeBPImplementationHelper::AISpecific)
		{
			MyNode->ReceiveConditionCheckAI(MyNode->AIOwner, MyNode->AIOwner->GetPawn());
		}
		else if (ReceiveConditionCheckImplementations & FBTNodeBPImplementationHelper::Generic)
		{
			MyNode->ReceiveConditionCheck(MyNode->ActorOwner);
		}
	}

	return CurrentCallResult;
}

void UBTDecorator_BlueprintBase::FinishConditionCheck(bool bAllowExecution)
{
	CurrentCallResult = bAllowExecution;
}

bool UBTDecorator_BlueprintBase::IsDecoratorExecutionActive() const
{
	UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(GetOuter());
	const bool bIsActive = OwnerComp->IsExecutingBranch(GetMyNode(), GetChildIndex());
	return bIsActive;
}

bool UBTDecorator_BlueprintBase::IsDecoratorObserverActive() const
{
	UBehaviorTreeComponent* OwnerComp = Cast<UBehaviorTreeComponent>(GetOuter());
	const bool bIsActive = OwnerComp->IsAuxNodeActive(this);
	return bIsActive;
}

FString UBTDecorator_BlueprintBase::GetStaticDescription() const
{
	FString ReturnDesc = Super::GetStaticDescription();

	UBTDecorator_BlueprintBase* CDO = (UBTDecorator_BlueprintBase*)(GetClass()->GetDefaultObject());
	if (bShowPropertyDetails && CDO)
	{
		UClass* StopAtClass = UBTDecorator_BlueprintBase::StaticClass();
		FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, CDO->PropertyData);
		if (PropertyDesc.Len())
		{
			ReturnDesc += TEXT(":\n\n");
			ReturnDesc += PropertyDesc;
		}
	}

	return ReturnDesc;
}

void UBTDecorator_BlueprintBase::DescribeRuntimeValues(const UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	UBTDecorator_BlueprintBase* CDO = (UBTDecorator_BlueprintBase*)(GetClass()->GetDefaultObject());
	if (CDO && CDO->PropertyData.Num())
	{
		BlueprintNodeHelpers::DescribeRuntimeValues(this, CDO->PropertyData, Values);
	}
}

void UBTDecorator_BlueprintBase::OnBlackboardChange(const UBlackboardComponent* Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* BehaviorComp = Blackboard ? (UBehaviorTreeComponent*)Blackboard->GetBrainComponent() : NULL;
	if (BehaviorComp)
	{
		BehaviorComp->RequestExecution(this);		
	}
}

void UBTDecorator_BlueprintBase::OnInstanceDestroyed(UBehaviorTreeComponent* OwnerComp)
{
	// force dropping all pending latent actions associated with this blueprint
	BlueprintNodeHelpers::AbortLatentActions(OwnerComp, this);
}

#if WITH_EDITOR

FName UBTDecorator_BlueprintBase::GetNodeIconName() const
{
	if(ReceiveConditionCheckImplementations != 0)
	{
		return FName("BTEditor.Graph.BTNode.Decorator.Conditional.Icon");
	}
	else
	{
		return FName("BTEditor.Graph.BTNode.Decorator.NonConditional.Icon");
	}
}

bool UBTDecorator_BlueprintBase::UsesBlueprint() const
{
	return true;
}

#endif // WITH_EDITOR
