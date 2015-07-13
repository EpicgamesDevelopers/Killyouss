// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "AnimGraphPrivatePCH.h"
#include "AnimGraphNode_ObserveBone.h"
#include "CompilerResultsLog.h"
#include "SGraphNode.h"
#include "KismetNodeInfoContext.h"
#include "KismetDebugUtilities.h"

#define LOCTEXT_NAMESPACE "ObserveBone"

/////////////////////////////////////////////////////
// SGraphNodeObserveBone

class SGraphNodeObserveBone : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeObserveBone){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UAnimGraphNode_ObserveBone* InNode)
	{
		GraphNode = InNode;

		SetCursor(EMouseCursor::CardinalCross);

		UpdateGraphNode();
	}

	// SNodePanel::SNode interface
	void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
	// End of SNodePanel::SNode interface

	// SGraphNode interface
	virtual void UpdateGraphNode() override
	{
		SGraphNode::UpdateGraphNode();

		// Prevent the comment bubble from being displayed
// 		GetOrAddSlot(ENodeZone::TopCenter)
// 		[
// 			SNullWidget::NullWidget
// 		];
	}
	// End of SGraphNode interface

	static FString PrettyVectorToString(const FVector& Vector, const FString& PerComponentPrefix);
};

void SGraphNodeObserveBone::GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const
{
	const FLinearColor TimelineBubbleColor(0.7f, 0.5f, 0.5f);
	FKismetNodeInfoContext* K2Context = (FKismetNodeInfoContext*)Context;

	// Display the observed bone transform status bubble
 	if (UObject* ActiveObject = K2Context->ActiveObjectBeingDebugged)
 	{
		UProperty* NodeProperty = FKismetDebugUtilities::FindClassPropertyForNode(K2Context->SourceBlueprint, GraphNode);
		if (UStructProperty* StructProperty = Cast<UStructProperty>(NodeProperty))
 		{
 			UClass* ContainingClass = StructProperty->GetTypedOuter<UClass>();
 			if (ActiveObject->IsA(ContainingClass) && (StructProperty->Struct == FAnimNode_ObserveBone::StaticStruct()))
 			{
				FAnimNode_ObserveBone* ObserveBone = StructProperty->ContainerPtrToValuePtr<FAnimNode_ObserveBone>(ActiveObject);

				check(ObserveBone);
				const FString Message = FString::Printf(TEXT("%s\n%s\n%s"),
					*PrettyVectorToString(ObserveBone->Translation, TEXT("T")),
					*PrettyVectorToString(ObserveBone->Rotation.Euler(), TEXT("R")),
					*PrettyVectorToString(ObserveBone->Scale, TEXT("S")));

				new (Popups) FGraphInformationPopupInfo(NULL, TimelineBubbleColor, Message);
			}
 			else
 			{
				const FString ErrorText = FString::Printf(*LOCTEXT("StaleDebugData", "Stale debug data\nProperty is on %s\nDebugging a %s").ToString(), *ContainingClass->GetName(), *ActiveObject->GetClass()->GetName());
				new (Popups) FGraphInformationPopupInfo(NULL, TimelineBubbleColor, ErrorText);
 			}
 		}
	}

	SGraphNode::GetNodeInfoPopups(Context, Popups);
}

FString SGraphNodeObserveBone::PrettyVectorToString(const FVector& Vector, const FString& PerComponentPrefix)
{
	const FString ReturnString = FString::Printf(TEXT("%sX=%.2f, %sY=%.2f, %sZ=%.2f"), *PerComponentPrefix, Vector.X, *PerComponentPrefix, Vector.Y, *PerComponentPrefix, Vector.Z);
	return ReturnString;
}

/////////////////////////////////////////////////////
// UAnimGraphNode_ObserveBone

TSharedPtr<SGraphNode> UAnimGraphNode_ObserveBone::CreateVisualWidget()
{
	return SNew(SGraphNodeObserveBone, this);
}

UAnimGraphNode_ObserveBone::UAnimGraphNode_ObserveBone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAnimGraphNode_ObserveBone::ValidateAnimNodeDuringCompilation(USkeleton* ForSkeleton, FCompilerResultsLog& MessageLog)
{
	if (ForSkeleton->GetReferenceSkeleton().FindBoneIndex(Node.BoneToObserve.BoneName) == INDEX_NONE)
	{
		MessageLog.Warning(*LOCTEXT("NoBoneToObserve", "@@ - You must pick a bone to observe").ToString(), this);
	}

	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);
}

FText UAnimGraphNode_ObserveBone::GetControllerDescription() const
{
	return LOCTEXT("AnimGraphNode_ObserveBone", "Observe Bone");
}

FText UAnimGraphNode_ObserveBone::GetTooltipText() const
{
	return LOCTEXT("AnimGraphNode_ObserveBone_Tooltip", "Observes a bone for debugging purposes");
}

FText UAnimGraphNode_ObserveBone::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (((TitleType == ENodeTitleType::ListView) || (TitleType == ENodeTitleType::MenuTitle)) && (Node.BoneToObserve.BoneName == NAME_None))
	{
		return GetControllerDescription();
	}
	else
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ControllerDescription"), GetControllerDescription());
		Args.Add(TEXT("BoneName"), FText::FromName(Node.BoneToObserve.BoneName));

		return FText::Format(LOCTEXT("AnimGraphNode_ObserveBone_Title", "{ControllerDescription}: {BoneName}"), Args);
	}
}

int32 UAnimGraphNode_ObserveBone::GetWidgetCoordinateSystem(const USkeletalMeshComponent* SkelComp)
{
	switch (Node.DisplaySpace)
	{
	default:
	case BCS_ParentBoneSpace:
		//@TODO: No good way of handling this one
		return COORD_World;
	case BCS_BoneSpace:
		return COORD_Local;
	case BCS_ComponentSpace:
	case BCS_WorldSpace:
		return COORD_World;
	}
}

FVector UAnimGraphNode_ObserveBone::GetWidgetLocation(const USkeletalMeshComponent* SkelComp, FAnimNode_SkeletalControlBase* AnimNode)
{
	USkeleton* Skeleton = SkelComp->SkeletalMesh->Skeleton;
	FVector WidgetLoc = FVector::ZeroVector;

	int32 MeshBoneIndex = SkelComp->GetBoneIndex(Node.BoneToObserve.BoneName);

	if (MeshBoneIndex != INDEX_NONE)
	{
		const FTransform BoneTM = SkelComp->GetBoneTransform(MeshBoneIndex);
		WidgetLoc = BoneTM.GetLocation();
	}
	
	return WidgetLoc;
}

int32 UAnimGraphNode_ObserveBone::GetWidgetMode(const USkeletalMeshComponent* SkelComp)
{
	return (int32)FWidget::WM_Translate;
}

FName UAnimGraphNode_ObserveBone::FindSelectedBone()
{
	return Node.BoneToObserve.BoneName;
}

FLinearColor UAnimGraphNode_ObserveBone::GetNodeTitleColor() const
{
	return FLinearColor(0.7f, 0.7f, 0.7f);
}

void UAnimGraphNode_ObserveBone::CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const
{
	if (Pin->PinName == GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_SkeletalControlBase, Alpha))
	{
		Pin->bHidden = true;
	}
}

#undef LOCTEXT_NAMESPACE
