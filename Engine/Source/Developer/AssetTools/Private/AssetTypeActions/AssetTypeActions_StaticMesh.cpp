// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "AssetToolsPrivatePCH.h"

#include "Editor/StaticMeshEditor/Public/StaticMeshEditorModule.h"

#include "Editor/DestructibleMeshEditor/Public/DestructibleMeshEditorModule.h"
#include "Editor/DestructibleMeshEditor/Public/IDestructibleMeshEditor.h"

#include "ApexDestructibleAssetImport.h"
#include "Engine/DestructibleMesh.h"

#include "FbxMeshUtils.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "Engine/StaticMesh.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_StaticMesh::GetActions( const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder )
{
	auto Meshes = GetTypedWeakObjectPtrs<UStaticMesh>(InObjects);

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("AssetTypeActions_StaticMesh", "ObjectContext_CreateDestructibleMesh", "Create Destructible Mesh"),
		NSLOCTEXT("AssetTypeActions_StaticMesh", "ObjectContext_CreateDestructibleMeshTooltip", "Creates a DestructibleMesh from the StaticMesh and opens it in the DestructibleMesh editor."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.DestructibleComponent"),
		FUIAction(
			FExecuteAction::CreateSP( this, &FAssetTypeActions_StaticMesh::ExecuteCreateDestructibleMesh, Meshes ),
			FCanExecuteAction()
			)
		);

	MenuBuilder.AddSubMenu(
		NSLOCTEXT("AssetTypeActions_StaticMesh", "StaticMesh_ImportLOD", "Import LOD"),
		NSLOCTEXT("AssetTypeActions_StaticMesh", "StaticMesh_ImportLODtooltip", "Imports meshes into the LODs"),
		FNewMenuDelegate::CreateSP( this, &FAssetTypeActions_StaticMesh::GetImportLODMenu, Meshes )
	);
}

void FAssetTypeActions_StaticMesh::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Mesh = Cast<UStaticMesh>(*ObjIt);
		if (Mesh != NULL)
		{
			IStaticMeshEditorModule* StaticMeshEditorModule = &FModuleManager::LoadModuleChecked<IStaticMeshEditorModule>( "StaticMeshEditor" );
			StaticMeshEditorModule->CreateStaticMeshEditor(Mode, EditWithinLevelEditor, Mesh);
		}
	}
}

UThumbnailInfo* FAssetTypeActions_StaticMesh::GetThumbnailInfo(UObject* Asset) const
{
	UStaticMesh* StaticMesh = CastChecked<UStaticMesh>(Asset);
	UThumbnailInfo* ThumbnailInfo = StaticMesh->ThumbnailInfo;
	if ( ThumbnailInfo == NULL )
	{
		ThumbnailInfo = ConstructObject<USceneThumbnailInfo>(USceneThumbnailInfo::StaticClass(), StaticMesh);
		StaticMesh->ThumbnailInfo = ThumbnailInfo;
	}

	return ThumbnailInfo;
}

void FAssetTypeActions_StaticMesh::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto StaticMesh = CastChecked<UStaticMesh>(Asset);
		if (StaticMesh->AssetImportData)
		{
			OutSourceFilePaths.Add(FReimportManager::ResolveImportFilename(StaticMesh->AssetImportData->SourceFilePath, StaticMesh));
		}
	}
}

void FAssetTypeActions_StaticMesh::GetImportLODMenu(class FMenuBuilder& MenuBuilder,TArray<TWeakObjectPtr<UStaticMesh>> Objects)
{
	check(Objects.Num() > 0);
	auto First = Objects[0];
	UStaticMesh* StaticMesh = First.Get();

	for(int32 LOD = 1;LOD<=First->GetNumLODs();++LOD)
	{
		FText LODText = FText::AsNumber( LOD );
		FText Description = FText::Format( NSLOCTEXT("AssetTypeActions_StaticMesh", "Reimport LOD (number)", "Reimport LOD {0}"), LODText );
		FText ToolTip = NSLOCTEXT("AssetTypeActions_StaticMesh", "ReimportTip", "Reimport over existing LOD");
		if(LOD == First->GetNumLODs())
		{
			Description = FText::Format( NSLOCTEXT("AssetTypeActions_StaticMesh", "LOD (number)", "LOD {0}"), LODText );
			ToolTip = NSLOCTEXT("AssetTypeActions_StaticMesh", "NewImportTip", "Import new LOD");
		}

		MenuBuilder.AddMenuEntry( Description, ToolTip, FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic( &FbxMeshUtils::ImportMeshLODDialog, Cast<UObject>(StaticMesh), LOD) )) ;
	}
}

void FAssetTypeActions_StaticMesh::ExecuteCreateDestructibleMesh(TArray<TWeakObjectPtr<UStaticMesh>> Objects)
{
	TArray< UObject* > Assets;
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Object = (*ObjIt).Get();
		if ( Object )
		{
			FText ErrorMsg;
			FDestructibleMeshEditorModule& DestructibleMeshEditorModule = FModuleManager::LoadModuleChecked<FDestructibleMeshEditorModule>( "DestructibleMeshEditor" );
			UDestructibleMesh* DestructibleMesh = DestructibleMeshEditorModule.CreateDestructibleMeshFromStaticMesh(Object->GetOuter(), Object, NAME_None, Object->GetFlags(), ErrorMsg);
			if ( DestructibleMesh )
			{
				FAssetEditorManager::Get().OpenEditorForAsset(DestructibleMesh);
				Assets.Add(DestructibleMesh);
			}
			else if ( !ErrorMsg.IsEmpty() )
			{
				FNotificationInfo ErrorNotification( ErrorMsg );
				FSlateNotificationManager::Get().AddNotification(ErrorNotification);
			}
		}
	}
	if ( Assets.Num() > 0 )
	{
		FAssetTools::Get().SyncBrowserToAssets(Assets);
	}
}

#undef LOCTEXT_NAMESPACE
