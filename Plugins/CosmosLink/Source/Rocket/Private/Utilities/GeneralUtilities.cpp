// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "GeneralUtilities.h"

#include "RocketModule.h"


bool RocketGeneralUtilities::IsInEditorAndNotPlaying() {
	if (!IsInGameThread())
	{
		UE_LOG(LogRocket, Error, TEXT("You are not on the main thread."));
		return false;
	}
	if (!GIsEditor)
	{
		UE_LOG(LogRocket, Error, TEXT("You are not in the Editor."));
		return false;
	}
	if (GEditor->PlayWorld || GIsPlayInEditorWorld)
	{
		UE_LOG(LogRocket, Error, TEXT("The Editor is currently in a play mode."));
		return false;
	}
	return true;
}