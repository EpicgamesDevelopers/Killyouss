// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "PlayTimeLimitModule.h"

#include "PlayTimeLimitImpl.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/ConfigCacheIni.h"

IMPLEMENT_MODULE(FPlayTimeLimitModule, PlayTimeLimit);

DEFINE_LOG_CATEGORY(LogPlayTimeLimit);

void FPlayTimeLimitModule::StartupModule()
{
	GConfig->GetBool(TEXT("PlayTimeLimit"), TEXT("bEnabled"), bPlayTimeLimitEnabled, GEngineIni);
	if (bPlayTimeLimitEnabled)
	{
		FPlayTimeLimitImpl::Get().Initialize();
	}
}

void FPlayTimeLimitModule::ShutdownModule()
{
	if (bPlayTimeLimitEnabled)
	{
		FPlayTimeLimitImpl::Get().Shutdown();
	}
}

bool FPlayTimeLimitModule::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	bool bWasHandled = false;

	// Ignore any execs that don't start with PlayTimeLimit
	if (FParse::Command(&Cmd, TEXT("PlayTimeLimit")))
	{
		if (bPlayTimeLimitEnabled)
		{
			bWasHandled = true; // Set to false in the catch-all else block
			FPlayTimeLimitImpl& PlayTimeLimit = FPlayTimeLimitImpl::Get();
			if (FParse::Command(&Cmd, TEXT("DUMP")))
			{
				PlayTimeLimit.DumpState();
			}
			else if (FParse::Command(&Cmd, TEXT("NOTIFYNOW")))
			{
				PlayTimeLimit.NotifyNow();
			}
			else if (FParse::Command(&Cmd, TEXT("SETUSERLIMITS")))
			{
				// Usage: SETUSERLIMITS <usernum> <has limit = [true, false]> <current play time limits>
				const int32 LocalUserNum = FCString::Atoi(*FParse::Token(Cmd, false));
				const bool bHasLimit = FCString::ToBool(*FParse::Token(Cmd, false));
				const double CurrentPlayTimeMinutes = FCString::Atof(*FParse::Token(Cmd, false));

				IOnlineSubsystem* const OnlineSubsystem = IOnlineSubsystem::Get();
				if (OnlineSubsystem)
				{
					const IOnlineIdentityPtr IdentityInt = OnlineSubsystem->GetIdentityInterface();
					if (IdentityInt.IsValid())
					{
						const TSharedPtr<const FUniqueNetId> UserId = IdentityInt->GetUniquePlayerId(LocalUserNum);
						if (UserId.IsValid())
						{
							PlayTimeLimit.MockUser(*UserId, bHasLimit, CurrentPlayTimeMinutes);
						}
						else
						{
							UE_LOG(LogPlayTimeLimit, Warning, TEXT("SETUSERLIMITS: Could not get player id from user num=%d, ensure you are logged in first"), LocalUserNum);
						}
					}
					else
					{
						UE_LOG(LogPlayTimeLimit, Warning, TEXT("SETUSERLIMITS: Missing Identity interface"));
					}
				}
				else
				{
					UE_LOG(LogPlayTimeLimit, Warning, TEXT("SETUSERLIMITS: Missing OnlineSubsystem"));
				}
			}
			else
			{
				bWasHandled = false;
			}
		}
		else
		{
			UE_LOG(LogPlayTimeLimit, Warning, TEXT("PlayTimeLimit is not enabled by config file"));
		}
	}
	return bWasHandled;
}
