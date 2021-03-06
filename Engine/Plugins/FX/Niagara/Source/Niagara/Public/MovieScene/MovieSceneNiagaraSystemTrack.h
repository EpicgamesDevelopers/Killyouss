// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "MovieScene/MovieSceneNiagaraTrack.h"
#include "MovieSceneNiagaraSystemTrack.generated.h"

UCLASS(MinimalAPI)
class UMovieSceneNiagaraSystemTrack : public UMovieSceneNiagaraTrack
{
	GENERATED_BODY()

public:
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;
	virtual void PostCompile(FMovieSceneEvaluationTrack& OutTrack, const FMovieSceneTrackCompilerArgs& Args) const override;
	virtual FMovieSceneTrackSegmentBlenderPtr GetTrackSegmentBlender() const override;
};