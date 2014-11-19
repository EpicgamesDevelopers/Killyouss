// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "AddContentDialogPCH.h"

#include "ImageWrapper.h"
#include "ModuleManager.h"

#define LOCTEXT_NAMESPACE "ContentSourceViewModel"

const FString DefaultLanguageCode = "en";

FContentSourceViewModel::FContentSourceViewModel(TSharedPtr<IContentSource> ContentSourceIn)
{
	ContentSource = ContentSourceIn;
	SetupBrushes();
	Category = FCategoryViewModel(ContentSource->GetCategory());
}

TSharedPtr<IContentSource> FContentSourceViewModel::GetContentSource()
{
	return ContentSource;
}

FText FContentSourceViewModel::GetName()
{
	FString CurrentLanguage = FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName();
	if (NameText.GetTwoLetterLanguage() != CurrentLanguage)
	{
		NameText = ChooseLocalizedText(ContentSource->GetLocalizedNames(), CurrentLanguage);
	}
	return NameText.GetText();
}

FText FContentSourceViewModel::GetDescription()
{
	FString CurrentLanguage = FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName();
	if (DescriptionText.GetTwoLetterLanguage() != CurrentLanguage)
	{
		DescriptionText = ChooseLocalizedText(ContentSource->GetLocalizedDescriptions(), CurrentLanguage);
	}
	return DescriptionText.GetText();
}

FCategoryViewModel FContentSourceViewModel::GetCategory()
{
	return Category;
}

TSharedPtr<FSlateBrush> FContentSourceViewModel::GetIconBrush()
{
	return IconBrush;
}

TArray<TSharedPtr<FSlateBrush>>* FContentSourceViewModel::GetScreenshotBrushes()
{
	return &ScreenshotBrushes;
}

void FContentSourceViewModel::SetupBrushes()
{
	FString IconBrushName = GetName().ToString() + "_" + ContentSource->GetIconData()->GetName();
	IconBrush = CreateBrushFromRawData(FName(*IconBrushName), *ContentSource->GetIconData()->GetData());

	for (TSharedPtr<FImageData> ScreenshotData : ContentSource->GetScreenshotData())
	{
		FString ScreenshotBrushName = GetName().ToString() + "_" + ScreenshotData->GetName();
		ScreenshotBrushes.Add(CreateBrushFromRawData(FName(*ScreenshotBrushName), *ScreenshotData->GetData()));
	}
}

TSharedPtr<FSlateDynamicImageBrush> FContentSourceViewModel::CreateBrushFromRawData(FName ResourceName, const TArray< uint8 >& RawData) const
{
	TSharedPtr< FSlateDynamicImageBrush > Brush;

	uint32 BytesPerPixel = 4;
	int32 Width = 0;
	int32 Height = 0;

	bool bSucceeded = false;
	TArray<uint8> DecodedImage;
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawData.GetData(), RawData.Num()))
	{
		Width = ImageWrapper->GetWidth();
		Height = ImageWrapper->GetHeight();

		const TArray<uint8>* RawImageData = NULL;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawImageData))
		{
			DecodedImage.AddUninitialized(Width * Height * BytesPerPixel);
			DecodedImage = *RawImageData;
			bSucceeded = true;
		}
	}

	if (bSucceeded)
	{
		Brush = FSlateDynamicImageBrush::CreateWithImageData(ResourceName, FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()), DecodedImage);
	}

	return Brush;
}

FLocalizedText FContentSourceViewModel::ChooseLocalizedText(TArray<FLocalizedText> Choices, FString LanguageCode)
{
	FLocalizedText Default;
	for (const FLocalizedText& Choice : Choices)
	{
		if (Choice.GetTwoLetterLanguage() == LanguageCode)
		{
			return Choice;
			break;
		}
		else if (Choice.GetTwoLetterLanguage() == DefaultLanguageCode)
		{
			Default = Choice;
		}
	}
	return Default;
}

#undef LOCTEXT_NAMESPACE
