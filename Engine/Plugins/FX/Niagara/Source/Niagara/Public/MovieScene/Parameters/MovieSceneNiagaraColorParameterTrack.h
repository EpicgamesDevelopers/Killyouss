// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MovieScene/Parameters/MovieSceneNiagaraParameterTrack.h"
#include "MovieSceneNiagaraColorParameterTrack.generated.h"

/** A track for animating color niagara parameters. */
UCLASS(MinimalAPI)
class UMovieSceneNiagaraColorParameterTrack : public UMovieSceneNiagaraParameterTrack
{
	GENERATED_BODY()

public:
	/** UMovieSceneTrack interface. */
	virtual UMovieSceneSection* CreateNewSection() override;
	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;
};