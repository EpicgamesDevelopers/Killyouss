// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UMGPrivatePCH.h"

#define LOCTEXT_NAMESPACE "UMG"

/////////////////////////////////////////////////////
// UButton

UButton::UButton(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	SButton::FArguments ButtonDefaults;

	Style = NULL;

	DesiredSizeScale = ButtonDefaults._DesiredSizeScale.Get();
	ContentScale = ButtonDefaults._ContentScale.Get();

	ColorAndOpacity = FLinearColor::White;
	BackgroundColor = FLinearColor::White;
	//ForegroundColor = FLinearColor::Black;

	ClickMethod = EButtonClickMethod::DownAndUp;
	TouchMethod = EButtonTouchMethod::DownAndUp;

	IsFocusable = true;
}

void UButton::ReleaseNativeWidget()
{
	Super::ReleaseNativeWidget();

	MyButton.Reset();
}

TSharedRef<SWidget> UButton::RebuildWidget()
{
	MyButton = SNew(SButton)
		.ClickMethod(ClickMethod)
		.TouchMethod(TouchMethod)
		.IsFocusable(IsFocusable);

	if ( GetChildrenCount() > 0 )
	{
		Cast<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	}
	
	return MyButton.ToSharedRef();
}

void UButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	TOptional<FSlateSound> OptionalPressedSound;
	if ( PressedSound.GetResourceObject() )
	{
		OptionalPressedSound = PressedSound;
	}

	TOptional<FSlateSound> OptionalHoveredSound;
	if ( HoveredSound.GetResourceObject() )
	{
		OptionalHoveredSound = HoveredSound;
	}

	if ( MyStyle.IsSet() )
	{
		MyButton->SetButtonStyle(&MyStyle.GetValue());
	}
	else
	{
		const FButtonStyle* StylePtr = GetStyle();
		MyButton->SetButtonStyle(StylePtr);
	}

	MyButton->SetColorAndOpacity( ColorAndOpacity );
	MyButton->SetBorderBackgroundColor( BackgroundColor );
	
	MyButton->SetDesiredSizeScale( DesiredSizeScale );
	MyButton->SetContentScale( ContentScale );
	MyButton->SetPressedSound( OptionalPressedSound );
	MyButton->SetHoveredSound( OptionalHoveredSound );

	MyButton->SetOnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked));
}

UClass* UButton::GetSlotClass() const
{
	return UButtonSlot::StaticClass();
}

void UButton::OnSlotAdded(UPanelSlot* Slot)
{
	// Add the child to the live slot if it already exists
	if ( MyButton.IsValid() )
	{
		Cast<UButtonSlot>(Slot)->BuildSlot(MyButton.ToSharedRef());
	}
}

void UButton::OnSlotRemoved(UPanelSlot* Slot)
{
	// Remove the widget from the live slot if it exists.
	if ( MyButton.IsValid() )
	{
		MyButton->SetContent(SNullWidget::NullWidget);
	}
}

void UButton::SetStyle(USlateWidgetStyleAsset* InStyle)
{
	Style = InStyle;
	MyStyle = TOptional<FButtonStyle>();

	const FButtonStyle* StylePtr = GetStyle();

	if ( MyButton.IsValid() )
	{
		MyButton->SetButtonStyle(StylePtr);
	}
}

const FButtonStyle* UButton::GetStyle() const
{
	const FButtonStyle* StylePtr = ( Style != NULL ) ? Style->GetStyle<FButtonStyle>() : NULL;
	if ( StylePtr == NULL )
	{
		SButton::FArguments ButtonDefaults;
		StylePtr = ButtonDefaults._ButtonStyle;
	}

	return StylePtr;
}

void UButton::SetButtonStyle(FButtonStyle InButtonStyle)
{
	MyStyle = InButtonStyle;
}

FButtonStyle UButton::GetButtonStyle()
{
	// If the dynamic style hasn't been set, default it to a clone of the current
	// button style asset.
	if ( !MyStyle.IsSet() )
	{
		const FButtonStyle* StylePtr = GetStyle();
		MyStyle = *StylePtr;
	}
	
	return MyStyle.GetValue();
}

void UButton::SetColorAndOpacity(FLinearColor Color)
{
	ColorAndOpacity = Color;
	if ( MyButton.IsValid() )
	{
		MyButton->SetColorAndOpacity(Color);
	}
}

void UButton::SetBackgroundColor(FLinearColor Color)
{
	BackgroundColor = Color;
	if ( MyButton.IsValid() )
	{
		MyButton->SetBorderBackgroundColor(Color);
	}
}

bool UButton::IsPressed() const
{
	return MyButton->IsPressed();
}

void UButton::PostLoad()
{
	Super::PostLoad();

	if ( GetChildrenCount() > 0 )
	{
		//TODO UMG Pre-Release Upgrade, now buttons have slots of their own.  Convert existing slot to new slot.
		if ( UPanelSlot* PanelSlot = GetContentSlot() )
		{
			UButtonSlot* ButtonSlot = Cast<UButtonSlot>(PanelSlot);
			if ( ButtonSlot == NULL )
			{
				ButtonSlot = ConstructObject<UButtonSlot>(UButtonSlot::StaticClass(), this);
				ButtonSlot->Content = GetContentSlot()->Content;
				ButtonSlot->Content->Slot = ButtonSlot;
				Slots[0] = ButtonSlot;
			}
		}
	}
}

FReply UButton::SlateHandleClicked()
{
	OnClicked.Broadcast();

	return FReply::Handled();
}

#if WITH_EDITOR

const FSlateBrush* UButton::GetEditorIcon()
{
	return FUMGStyle::Get().GetBrush("Widget.Button");
}

#endif

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
