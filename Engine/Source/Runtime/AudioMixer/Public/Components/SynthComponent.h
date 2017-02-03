// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Sound/SoundWaveProcedural.h"

#include "SynthComponent.generated.h"

#define SYNTH_GENERATOR_TEST_TONE 0

#if SYNTH_GENERATOR_TEST_TONE
#include "DSP/SinOsc.h"
#endif

class USynthComponent;

UCLASS()
class AUDIOMIXER_API USynthSound : public USoundWaveProcedural
{
	GENERATED_UCLASS_BODY()

	void Init(USynthComponent* INSynthComponent, const int32 InNumChannels);

	virtual bool OnGeneratePCMAudio(TArray<uint8>& OutAudio, int32 NumSamples) override;

protected:
	USynthComponent* OwningSynthComponent;
	TArray<int16> GeneratedPCMData;
};

UCLASS(ClassGroup = Synth, hidecategories = (Object, ActorComponent, Physics, Rendering, Mobility, LOD))
class AUDIOMIXER_API USynthComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USynthComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin USceneComponent Interface
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	//~ End USceneComponent Interface

	//~ Begin ActorComponent Interface.
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual bool IsReadyForOwnerToAutoDestroy() const override;
	//~ End ActorComponent Interface.

	//~ Begin UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// Starts the synth generating audio.
	UFUNCTION(BlueprintCallable, Category = "Synth|Components|Audio")
	void Start();

	// Stops the synth generating audio.
	UFUNCTION(BlueprintCallable, Category = "Synth|Components|Audio")
	void Stop();

	/** @return true if this component is currently playing a SoundCue. */
	UFUNCTION(BlueprintCallable, Category = "Synth|Components|Audio")
	bool IsPlaying() const;

	/** Auto destroy this component on completion */
	UPROPERTY()
	uint8 bAutoDestroy : 1;

	/** Stop sound when owner is destroyed */
	UPROPERTY()
	uint8 bStopWhenOwnerDestroyed : 1;

	/** Is this audio component allowed to be spatialized? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation)
	uint8 bAllowSpatialization : 1;

	/** Should the Attenuation Settings asset be used (false) or should the properties set directly on the component be used for attenuation properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation)
	uint8 bOverrideAttenuation : 1;

	/** If bOverrideSettings is false, the asset to use to determine attenuation properties for sounds generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation, meta = (EditCondition = "!bOverrideAttenuation"))
	class USoundAttenuation* AttenuationSettings;

	/** If bOverrideSettings is true, the attenuation properties to use for sounds generated by this component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation, meta = (EditCondition = "bOverrideAttenuation"))
	struct FSoundAttenuationSettings AttenuationOverrides;

	/** What sound concurrency to use for sounds generated by this audio component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Concurrency)
	class USoundConcurrency* ConcurrencySettings;

	/** Sound class this sound belongs to */
	UPROPERTY(EditAnywhere, Category = SoundClass, meta = (DisplayName = "Sound Class"))
	USoundClass* SoundClass;

protected:

	// Method to execute parameter changes on game thread in audio render thread
	void SynthCommand(TFunction<void()> Command);

	// Called when synth is about to start playing
	virtual void OnStart() {}

	// Called when synth is about to stop playing
	virtual void OnStop() {}

	// Override to define the number channels (1 or 2) the synth uses. Called at init, can't change once playing.
	virtual int32 GetNumChannels() const;

	// Override to make synth non-spatialized
	virtual bool IsSpatialized() const;

	// Called when more audio is needed to be generated
	virtual void OnGenerateAudio(TArray<float>& OutAudio) PURE_VIRTUAL(USynthComponent::OnGenerateAudio,);

	// Called by procedural sound wave
	void OnGeneratePCMAudio(TArray<int16>& GeneratedPCMData);

	// Can be set by the derived class, defaults to 2
	int32 NumChannels;

private:

	UPROPERTY()
	USynthSound* Synth;

	UPROPERTY()
	UAudioComponent* AudioComponent;

	void PumpPendingMessages();

	TArray<float> AudioFloatData;

#if SYNTH_GENERATOR_TEST_TONE
	Audio::FSineOsc TestSineLeft;
	Audio::FSineOsc TestSineRight;
#endif

	// Whether or not synth is playing
	bool bIsSynthPlaying;

	TQueue<TFunction<void()>> CommandQueue;

	enum class ESynthEvent : uint8
	{
		None,
		Start,
		Stop
	};

	TQueue<ESynthEvent> PendingSynthEvents;

	friend class USynthSound;
};