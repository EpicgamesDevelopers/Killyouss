// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkit.h"

#include "PropertyEditorDelegates.h"
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "IDetailChildrenBuilder.h"

/**
 * The location of a property name relative to its editor widget                   
 */
namespace EPropertyNamePlacement
{
	enum Type
	{
		/** Do not show the property name */
		Hidden,
		/** Show the property name to the left of the widget */
		Left,
		/** Show the property name to the right of the widget */
		Right,
		/** Inside the property editor edit box (unused for properties that dont have edit boxes ) */
		Inside,
	};
}


/**
 * Potential results from accessing the values of properties                   
 */
namespace FPropertyAccess
{
	enum Result
	{
		/** Multiple values were found so the value could not be read */
		MultipleValues,
		/** Failed to set or get the value */
		Fail,
		/** Successfully set the got the value */
		Success,
	};
}



class IPropertyHandle;
class SPropertyTreeViewImpl;
class SWindow;
class IPropertyTableCellPresenter;
class IPropertyTypeCustomization;
class IDetailsView;

/**
 * Base class for adding an extra data to identify a custom property type
 */
class IPropertyTypeIdentifier
{
public:
	virtual ~IPropertyTypeIdentifier() {}

/**
	 * Called to identify if a property type is customized
	 *
	 * @param IPropertyHandle	Handle to the property being tested
	 */
	virtual bool IsPropertyTypeCustomized( const IPropertyHandle& PropertyHandle ) const = 0;
};

/**
 * Callback executed to query the custom layout of details
 */
struct FDetailLayoutCallback
{
	/** Delegate to call to query custom layout of details */
	FOnGetDetailCustomizationInstance DetailLayoutDelegate;
	/** The order of this class in the map of callbacks to send (callbacks sent in the order they are received) */
	int32 Order;
};

struct FPropertyTypeLayoutCallback
{
	FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate;

	TSharedPtr<IPropertyTypeIdentifier> PropertyTypeIdentifier;

	bool IsValid() { return PropertyTypeLayoutDelegate.IsBound(); }

	TSharedRef<IPropertyTypeCustomization> GetCustomizationInstance() const;
};


struct FPropertyTypeLayoutCallbackList
{
	/** The base callback is a registered callback with a null identifier */
	FPropertyTypeLayoutCallback BaseCallback;

	/** List of registered callbacks with a non null identifier */
	TArray< FPropertyTypeLayoutCallback > IdentifierList;

	void Add( const FPropertyTypeLayoutCallback& NewCallback );
	
	void Remove( const TSharedPtr<IPropertyTypeIdentifier>& InIdentifier );

	const FPropertyTypeLayoutCallback& Find( const IPropertyHandle& PropertyHandle );
};

typedef TMap< TWeakObjectPtr<UStruct>, FDetailLayoutCallback > FCustomDetailLayoutMap;
typedef TMap< FName, FDetailLayoutCallback > FCustomDetailLayoutNameMap;

/** This is a multimap as there many be more than one customization per property type */
typedef TMap< FName, FPropertyTypeLayoutCallbackList > FCustomPropertyTypeLayoutMap;


class FPropertyEditorModule : public IModuleInterface
{
	friend class SPropertyTreeView;
	friend class SDetailsView;
	friend class SStructureDetailsView;
	friend class SDetailsViewBase;
public:
	
	/**
	 * Called right after the module has been loaded                   
	 */
	virtual void StartupModule();

	/**
	 * Called by the module manager right before this module is unloaded
	 */
	virtual void ShutdownModule();

	/**
	 * Refreshes property windows with a new list of objects to view
	 * 
	 * @param NewObjectList	The list of objects each property window should view
	 */
	virtual void UpdatePropertyViews( const TArray<UObject*>& NewObjectList );

	/**
	 * Replaces objects being viewed by open property views with new objects
	 *
	 * @param OldToNewObjectMap	A mapping between object to replace and its replacement
	 */
	virtual void ReplaceViewedObjects( const TMap<UObject*, UObject*>& OldToNewObjectMap );

	/**
	 * Removes deleted objects from property views that are observing them
	 *
	 * @param DeletedObjects	The objects to delete
	 */
	virtual void RemoveDeletedObjects( TArray<UObject*>& DeletedObjects );

	/**
	 * Returns true if there is an unlocked detail view
	 */
	virtual bool HasUnlockedDetailViews() const;

	/**
	 * Registers a custom detail layout delegate for a specific class
	 *
	 * @param ClassName	The name of the class that the custom detail layout is for
	 * @param DetailLayoutDelegate	The delegate to call when querying for custom detail layouts for the classes properties
	 */
	virtual void RegisterCustomClassLayout( FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate );

	/**
	 * Unregisters a custom detail layout delegate for a specific class name
	 *
	 * @param ClassName	The class name with the custom detail layout delegate to remove
	 */
	virtual void UnregisterCustomClassLayout( FName ClassName );

	/**
	 * Registers a property type customization
	 * A property type is a specific UProperty type, a struct, or enum type
	 *
	 * @param PropertyTypeName		The name of the property type to customize.  For structs and enums this is the name of the struct class or enum	(not StructProperty or ByteProperty) 
	 * @param PropertyTypeLayoutDelegate	The delegate to call when querying for a custom layout of the property type
	 * @param Identifier			An identifier to use to differentiate between two customizations on the same type
	 */
	virtual void RegisterCustomPropertyTypeLayout( FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate, TSharedPtr<IPropertyTypeIdentifier> Identifier = nullptr, TSharedPtr<IDetailsView> ForSpecificInstance = nullptr );

	/**
	 * Unregisters a custom detail layout for a properrty type
	 *
	 * @param PropertyTypeName 	The name of the property type that was registered
	 * @param Identifier 		An identifier to use to differentiate between two customizations on the same type
	 */
	virtual void UnregisterCustomPropertyTypeLayout( FName PropertyTypeName, TSharedPtr<IPropertyTypeIdentifier> InIdentifier = nullptr, TSharedPtr<IDetailsView> ForSpecificInstance = nullptr );

	/**
	 * Customization modules should call this when that module has been unloaded, loaded, etc...
	 * so the property module can clean up its data.  Needed to support dynamic reloading of modules
	 */
	virtual void NotifyCustomizationModuleChanged();

	/**
	 * Creates a new detail view
	 *
 	 * @param DetailsViewArgs		The struct containing all the user definable details view arguments
	 * @return The new detail view
	 */
	virtual TSharedRef<class IDetailsView> CreateDetailView( const struct FDetailsViewArgs& DetailsViewArgs );

	/**
	 * Find an existing detail view
	 *
 	 * @param ViewIdentifier	The name of the details view to find
	 * @return The existing detail view, or null if it wasn't found
	 */
	virtual TSharedPtr<class IDetailsView> FindDetailView( const FName ViewIdentifier ) const;

	/**
	 *  Convenience method for creating a new floating details window (a details view with its own top level window)
	 *
	 * @param InObjects			The objects to create the detail view for.
	 * @param bIsLockable		True if the property view can be locked.
	 * @return The new details view window.
	 */
	virtual TSharedRef<SWindow> CreateFloatingDetailsView( const TArray< UObject* >& InObjects, bool bIsLockable );

	/**
	 * Creates a standalone widget for a single property
	 *
	 * @param InObject			The object to view
	 * @param InPropertyName	The name of the property to display
	 * @param InitParams		Optional init params for a single property
	 * @return The new property if valid or null
	 */
	virtual TSharedPtr<class ISinglePropertyView> CreateSingleProperty( UObject* InObject, FName InPropertyName, const struct FSinglePropertyParams& InitParams );

	virtual TSharedRef<class IStructureDetailsView> CreateStructureDetailView(const struct FDetailsViewArgs& DetailsViewArgs, TSharedPtr<class FStructOnScope> StructData, bool bShowObjects, const FString& CustomName = FString());

	/**
	 * Creates a property change listener that notifies users via a  delegate when a property on an object changes
	 *
	 * @return The new property change listener
	 */
	virtual TSharedRef<class IPropertyChangeListener> CreatePropertyChangeListener();

	virtual TSharedRef< class IPropertyTable > CreatePropertyTable();

	virtual TSharedRef< SWidget > CreatePropertyTableWidget( const TSharedRef< class IPropertyTable >& PropertyTable );

	virtual TSharedRef< SWidget > CreatePropertyTableWidget( const TSharedRef< class IPropertyTable >& PropertyTable, const TArray< TSharedRef< class IPropertyTableCustomColumn > >& Customizations );
	virtual TSharedRef< class IPropertyTableWidgetHandle > CreatePropertyTableWidgetHandle( const TSharedRef< IPropertyTable >& PropertyTable );
	virtual TSharedRef< class IPropertyTableWidgetHandle > CreatePropertyTableWidgetHandle( const TSharedRef< IPropertyTable >& PropertyTable, const TArray< TSharedRef< class IPropertyTableCustomColumn > >& Customizations );

	virtual TSharedRef< IPropertyTableCellPresenter > CreateTextPropertyCellPresenter( const TSharedRef< class FPropertyNode >& InPropertyNode, const TSharedRef< class IPropertyTableUtilities >& InPropertyUtilities, 
		const FSlateFontInfo* InFontPtr = NULL);

	/**
	 *
	 */
	virtual TSharedRef< FAssetEditorToolkit > CreatePropertyEditorToolkit( const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit );
	virtual TSharedRef< FAssetEditorToolkit > CreatePropertyEditorToolkit( const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray< UObject* >& ObjectsToEdit );
	virtual TSharedRef< FAssetEditorToolkit > CreatePropertyEditorToolkit( const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray< TWeakObjectPtr< UObject > >& ObjectsToEdit );

	FPropertyTypeLayoutCallback GetPropertyTypeCustomization(const UProperty* InProperty,const IPropertyHandle& PropertyHandle, TSharedPtr<IDetailsView> DetailsViewInstance );
	bool IsCustomizedStruct(const UStruct* Struct, TSharedPtr<IDetailsView> DetailsViewInstance ) const;

private:

	/**
	 * Creates and returns a property view widget for embedding property views in other widgets
	 * NOTE: At this time these MUST not be referenced by the caller of CreatePropertyView when the property module unloads
	 * 
	 * @param	InObject						The UObject that the property view should observe(Optional)
	 * @param	bAllowFavorites					Whether the property view should save favorites
	 * @param	bIsLockable						Whether or not the property view is lockable
	 * @param	bAllowSearch					Whether or not the property window allows searching it
	 * @param	InNotifyHook					Notify hook to call on some property change events
	 * @param	ColumnWidth						The width of the name column
	 * @param	OnPropertySelectionChanged		Delegate for notifying when the property selection has changed.
	 * @return	The newly created SPropertyTreeViewImpl widget
	 */
	virtual TSharedRef<SPropertyTreeViewImpl> CreatePropertyView( UObject* InObject, bool bAllowFavorites, bool bIsLockable, bool bHiddenPropertyVisibility, bool bAllowSearch, bool ShowTopLevelNodes, FNotifyHook* InNotifyHook, float InNameColumnWidth, FOnPropertySelectionChanged OnPropertySelectionChanged, FOnPropertyClicked OnPropertyMiddleClicked, FConstructExternalColumnHeaders ConstructExternalColumnHeaders, FConstructExternalColumnCell ConstructExternalColumnCell );

private:
	/** All created detail views */
	TArray< TWeakPtr<class SDetailsView> > AllDetailViews;
	/** All created single property views */
	TArray< TWeakPtr<class SSingleProperty> > AllSinglePropertyViews;
	/** A mapping of class names to detail layout delegates, called when querying for custom detail layouts */
	FCustomDetailLayoutNameMap ClassNameToDetailLayoutNameMap;
	/** A mapping of property names to property type layout delegates, called when querying for custom property layouts */
	FCustomPropertyTypeLayoutMap GlobalPropertyTypeToLayoutMap;
	/** Instanced property type customization mapped to a specific details view */
	TMap< TWeakPtr<IDetailsView>, FCustomPropertyTypeLayoutMap > InstancePropertyTypeLayoutMap;
};
