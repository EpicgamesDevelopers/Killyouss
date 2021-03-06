// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MeshEditorCommands.h"
#include "RemoveVertex.generated.h"


/** Attempts to remove the selected vertex from the polygon it belongs to, keeping the polygon intact */
UCLASS()
class URemoveVertexCommand : public UMeshEditorInstantCommand
{
	GENERATED_BODY()

protected:

	// Overrides
	virtual EEditableMeshElementType GetElementType() const override
	{
		return EEditableMeshElementType::Vertex;
	}
	virtual void RegisterUICommand( class FBindingContext* BindingContext ) override;
	virtual void Execute( class IMeshEditorModeEditingContract& MeshEditorMode ) override;
	virtual void AddToVRRadialMenuActionsMenu( class IMeshEditorModeUIContract& MeshEditorMode, class FMenuBuilder& MenuBuilder, TSharedPtr<FUICommandList> CommandList, const FName TEMPHACK_StyleSetName, class UVREditorMode* VRMode ) override;

};
