// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Framework/Application/NavigationConfig.h"
#include "Types/SlateEnums.h"
#include "Input/Events.h"
#include "Misc/App.h"

FNavigationConfig::FNavigationConfig()
	: bTabNavigation(true)
	, bKeyNavigation(true)
	, bAnalogNavigation(true)
	, AnalogNavigationHorizontalThreshold(0.40f)
	, AnalogNavigationVerticalThreshold(0.50f)
{
	AnalogHorizontalKey = EKeys::Gamepad_LeftX;
	AnalogVerticalKey = EKeys::Gamepad_LeftY;

	KeyEventRules.Emplace(EKeys::Left, EUINavigation::Left);
	KeyEventRules.Emplace(EKeys::Gamepad_DPad_Left, EUINavigation::Left);

	KeyEventRules.Emplace(EKeys::Right, EUINavigation::Right);
	KeyEventRules.Emplace(EKeys::Gamepad_DPad_Right, EUINavigation::Right);

	KeyEventRules.Emplace(EKeys::Up, EUINavigation::Up);
	KeyEventRules.Emplace(EKeys::Gamepad_DPad_Up, EUINavigation::Up);

	KeyEventRules.Emplace(EKeys::Down, EUINavigation::Down);
	KeyEventRules.Emplace(EKeys::Gamepad_DPad_Down, EUINavigation::Down);
}

FNavigationConfig::~FNavigationConfig()
{
}

void FNavigationConfig::OnRegister()
{
	UserNavigationState.Reset();
}

void FNavigationConfig::OnUnregister()
{
}

void FNavigationConfig::OnUserRemoved(int32 UserIndex)
{
	UserNavigationState.Remove(UserIndex);
}

EUINavigation FNavigationConfig::GetNavigationDirectionFromKey(const FKeyEvent& InKeyEvent) const
{
	if (const EUINavigation* Rule = KeyEventRules.Find(InKeyEvent.GetKey()))
	{
		if (bKeyNavigation)
		{
			return *Rule;
		}
	}
	else if (bTabNavigation && InKeyEvent.GetKey() == EKeys::Tab )
	{
		//@TODO: Really these uses of input should be at a lower priority, only occurring if nothing else handled them
		// For now this code prevents consuming them when some modifiers are held down, allowing some limited binding
		const bool bAllowEatingKeyEvents = !InKeyEvent.IsControlDown() && !InKeyEvent.IsAltDown() && !InKeyEvent.IsCommandDown();

		if ( bAllowEatingKeyEvents )
		{
			return ( InKeyEvent.IsShiftDown() ) ? EUINavigation::Previous : EUINavigation::Next;
		}
	}

	return EUINavigation::Invalid;
}

EUINavigation FNavigationConfig::GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent)
{
	if (bAnalogNavigation)
	{
		EUINavigation DesiredNavigation = GetNavigationDirectionFromAnalogInternal(InAnalogEvent);
		if (DesiredNavigation != EUINavigation::Invalid)
		{
			FUserNavigationState& UserState = UserNavigationState.FindOrAdd(InAnalogEvent.GetUserIndex());
			FAnalogNavigationState& AnalogState = UserState.AnalogNavigationState.FindOrAdd(DesiredNavigation);

			const float RepeatRate = GetRepeatRateForPressure( FMath::Abs(InAnalogEvent.GetAnalogValue()), FMath::Max(AnalogState.Repeats - 1, 0));
			if (FApp::GetCurrentTime() - AnalogState.LastNavigationTime > RepeatRate)
			{
				AnalogState.LastNavigationTime = FApp::GetCurrentTime();
				AnalogState.Repeats++;
				return DesiredNavigation;
			}
		}
	}

	return EUINavigation::Invalid;
}

EUINavigation FNavigationConfig::GetNavigationDirectionFromAnalogInternal(const FAnalogInputEvent& InAnalogEvent)
{
	if (bAnalogNavigation)
	{
		FUserNavigationState& UserState = UserNavigationState.FindOrAdd(InAnalogEvent.GetUserIndex());

		if (InAnalogEvent.GetKey() == AnalogHorizontalKey)
		{
			if (InAnalogEvent.GetAnalogValue() < -AnalogNavigationHorizontalThreshold)
			{
				return EUINavigation::Left;
			}
			else if (InAnalogEvent.GetAnalogValue() > AnalogNavigationHorizontalThreshold)
			{
				return EUINavigation::Right;
			}
			else
			{
				UserState.AnalogNavigationState.Add(EUINavigation::Left, FAnalogNavigationState());
				UserState.AnalogNavigationState.Add(EUINavigation::Right, FAnalogNavigationState());
			}
		}
		else if (InAnalogEvent.GetKey() == AnalogVerticalKey)
		{
			if (InAnalogEvent.GetAnalogValue() > AnalogNavigationVerticalThreshold)
			{
				return EUINavigation::Up;
			}
			else if (InAnalogEvent.GetAnalogValue() < -AnalogNavigationVerticalThreshold)
			{
				return EUINavigation::Down;
			}
			else
			{
				UserState.AnalogNavigationState.Add(EUINavigation::Up, FAnalogNavigationState());
				UserState.AnalogNavigationState.Add(EUINavigation::Down, FAnalogNavigationState());
			}
		}
	}

	return EUINavigation::Invalid;
}

float FNavigationConfig::GetRepeatRateForPressure(float InPressure, int32 InRepeats) const
{
	const float RepeatRate = (InRepeats == 0) ? 0.5f : 0.25f;
	if (InPressure > 0.90f)
	{
		return RepeatRate * 0.5f;
	}

	return RepeatRate;
}