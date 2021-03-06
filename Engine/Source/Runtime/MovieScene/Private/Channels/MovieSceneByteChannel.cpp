// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Channels/MovieSceneByteChannel.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Curves/IntegralCurve.h"
#include "MovieSceneFwd.h"
#include "Misc/FrameRate.h"
#include "MovieSceneFrameMigration.h"

bool FMovieSceneByteChannel::SerializeFromMismatchedTag(const FPropertyTag& Tag, FArchive& Ar)
{
	static const FName IntegralCurveName("IntegralCurve");
	if (Tag.Type == NAME_StructProperty && Tag.StructName == IntegralCurveName)
	{
		FIntegralCurve IntegralCurve;
		FIntegralCurve::StaticStruct()->SerializeItem(Ar, &IntegralCurve, nullptr);

		if (IntegralCurve.GetDefaultValue() != MAX_int32)
		{
			bHasDefaultValue = true;
			// We cast rather than clamp here as the old integer curve used to wrap around
			DefaultValue = static_cast<uint8>(IntegralCurve.GetDefaultValue());
		}

		Times.Reserve(IntegralCurve.GetNumKeys());
		Values.Reserve(IntegralCurve.GetNumKeys());

		FFrameRate LegacyFrameRate = GetLegacyConversionFrameRate();

		int32 Index = 0;
		for (auto It = IntegralCurve.GetKeyIterator(); It; ++It)
		{
			FFrameNumber KeyTime = UpgradeLegacyMovieSceneTime(nullptr, LegacyFrameRate, It->Time);

			// We cast rather than clamp here as the old integer curve used to wrap around
			uint8 Val = static_cast<uint8>(It->Value);
			ConvertInsertAndSort<uint8>(Index++, KeyTime, Val, Times, Values);
		}
		return true;
	}

	return false;
}

bool FMovieSceneByteChannel::Evaluate(FFrameTime InTime, uint8& OutValue) const
{
	if (Times.Num())
	{
		const int32 Index = FMath::Max(0, Algo::UpperBound(Times, InTime.FrameNumber)-1);
		OutValue = Values[Index];
		return true;
	}
	else if (bHasDefaultValue)
	{
		OutValue = DefaultValue;
		return true;
	}

	return false;
}

void FMovieSceneByteChannel::GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles)
{
	GetData().GetKeys(WithinRange, OutKeyTimes, OutKeyHandles);
}

void FMovieSceneByteChannel::GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes)
{
	GetData().GetKeyTimes(InHandles, OutKeyTimes);
}

void FMovieSceneByteChannel::SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes)
{
	GetData().SetKeyTimes(InHandles, InKeyTimes);
}

void FMovieSceneByteChannel::DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles)
{
	GetData().DuplicateKeys(InHandles, OutNewHandles);
}

void FMovieSceneByteChannel::DeleteKeys(TArrayView<const FKeyHandle> InHandles)
{
	GetData().DeleteKeys(InHandles);
}

void FMovieSceneByteChannel::ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate)
{
	GetData().ChangeFrameResolution(SourceRate, DestinationRate);
}

TRange<FFrameNumber> FMovieSceneByteChannel::ComputeEffectiveRange() const
{
	return GetData().GetTotalRange();
}

int32 FMovieSceneByteChannel::GetNumKeys() const
{
	return Times.Num();
}

void FMovieSceneByteChannel::Reset()
{
	Times.Reset();
	Values.Reset();
	KeyHandles.Reset();
	bHasDefaultValue = false;
}

void FMovieSceneByteChannel::Optimize(const FKeyDataOptimizationParams& InParameters)
{
	MovieScene::Optimize(this, InParameters);
}

void FMovieSceneByteChannel::Offset(FFrameNumber DeltaPosition)
{
	GetData().Offset(DeltaPosition);
}

void FMovieSceneByteChannel::ClearDefault()
{
	bHasDefaultValue = false;
}