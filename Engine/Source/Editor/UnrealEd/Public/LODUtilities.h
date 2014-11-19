// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

class USkeletalMesh;

//////////////////////////////////////////////////////////////////////////
// FSkeletalMeshUpdateContext


struct FSkeletalMeshUpdateContext
{
	USkeletalMesh*				SkeletalMesh;
	TArray<UActorComponent*>	AssociatedComponents;

	FExecuteAction				OnLODChanged;
};

//////////////////////////////////////////////////////////////////////////
// FLODUtilities

class UNREALED_API FLODUtilities
{
public:
	/** Removes a particular LOD from the SkeletalMesh. 
	*
	* @param UpdateContext - The skeletal mesh and actor components to operate on.
	* @param DesiredLOD   - The LOD index to remove the LOD from.
	*/
	static void RemoveLOD( FSkeletalMeshUpdateContext& UpdateContext, int32 DesiredLOD );

	/**
	 *	Simplifies the static mesh based upon various user settings.
	 *
	 * @param UpdateContext - The skeletal mesh and actor components to operate on.
	 * @param InSettings   - The user settings passed from the Simplification tool.
	 */
	static void SimplifySkeletalMesh( FSkeletalMeshUpdateContext& UpdateContext, TArray<FSkeletalMeshOptimizationSettings> &InSettings );

	/**
	 * Refresh LOD Change
	 * 
	 * LOD has changed, it will have to notify all SMC that uses this SM
	 * and ask them to refresh LOD
	 *
	 * @param	InSkeletalMesh	SkeletalMesh that LOD has been changed for
	 */
	static void RefreshLODChange(const USkeletalMesh* SkeletalMesh);

private:
	FLODUtilities() {}
};