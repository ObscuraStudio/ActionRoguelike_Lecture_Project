// Fill out your copyright notice in the Description page of Project Settings.


#include "DragonIKTraceManagerComponent.h"

#include "DragonIK_Library.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ScopeLock.h"

// Sets default values for this component's properties
UDragonIKTraceManagerComponent::UDragonIKTraceManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UDragonIKTraceManagerComponent::IssueTraceInput(FDragonIKTraceMParams& TraceParams)
{
	test_count++;

	if (TraceParams.TraceIdentifier.IsNone())
	{
		return;
	}

	// This can be called from animation worker threads.
	// We only update buffers here; traces are executed on the game thread in TickComponent.
	{
		FScopeLock Lock(&PendingCS);

		hand_shaked = true;
		TraceParams.allow = true;

		if (!Solver_Markers.Contains(TraceParams.TraceIdentifier))
		{
			Solver_Markers.AddUnique(TraceParams.TraceIdentifier);
		}

		// Keep legacy arrays updated for compatibility (even though they are slower).
		PendingTraces_SetValueOnKey(PendingTraces, TraceParams.TraceIdentifier, TraceParams);
		PendingMap.Add(TraceParams.TraceIdentifier, TraceParams);
	}

	// Return the latest cached result if available.
	{
		FScopeLock Lock(&ResultsCS);
		if (const FDragonIKTraceKeyValuePair* Pair = ResultsMap.Find(TraceParams.TraceIdentifier))
		{
			TraceParams.hit_result = Pair->hit_result;
		}
		else if (Hitmap_FindKeyExists(HitResultMap, TraceParams.TraceIdentifier))
		{
			// Fallback for legacy storage.
			TraceParams.hit_result = Hitmap_GetValueFromKey(HitResultMap, TraceParams.TraceIdentifier);
		}
	}
}



bool UDragonIKTraceManagerComponent::Hitmap_FindKeyExists(const TArray<FDragonIKTraceKeyValuePair>& array, FName key) const
{


	if (array.Num() > 0)
	{
		for (int i = 0; i < array.Num(); i++)
		{
			if (array[i].Key == key)
			{
				return true;
			}
		}
	}

	return false;
}

FHitResult UDragonIKTraceManagerComponent::Hitmap_GetValueFromKey(const TArray<FDragonIKTraceKeyValuePair>& array, FName key) const
{

	if (array.Num() > 0)
	{
		for (int i = 0; i < array.Num(); i++)
		{
			if (array[i].Key == key)
			{
				return array[i].hit_result;
			}
		}
	}

	return FHitResult();
}

void UDragonIKTraceManagerComponent::Hitmap_SetValueOnKey(TArray<FDragonIKTraceKeyValuePair>& array, FName key,
	FHitResult value, FName bone_input)
{

	{
		bool key_found = false;
		for (int i = 0; i < array.Num(); i++)
		{
			if (array[i].Key == key)
			{
				array[i].hit_result = value;
				key_found = true;
				return;
			}
		}

		if (key_found == false)
		{
			FDragonIKTraceKeyValuePair trace_value_pair = FDragonIKTraceKeyValuePair();
			trace_value_pair.Key = key;
			trace_value_pair.hit_result = value;
			trace_value_pair.BoneName = bone_input;

			array.Add(trace_value_pair);
		}
	}
	/*	else
		{
			array = TArray<FDragonIKTraceKeyValuePair>();
		}*/

}

bool UDragonIKTraceManagerComponent::PendingTraces_FindKeyExists(const TArray<FDragonIKTraceParamKeyValuePair>& array, FName key) const
{
	if (array.Num() > 0)
	{
		for (int i = 0; i < array.Num(); i++)
		{
			if (array[i].Key == key)
			{
				return true;
			}
		}
	}

	return false;
}

FDragonIKTraceMParams UDragonIKTraceManagerComponent::PendingTraces_GetValueFromKey(const TArray<FDragonIKTraceParamKeyValuePair>& array, FName key) const
{
	if (array.Num() > 0)
	{
		for (int i = 0; i < array.Num(); i++)
		{
			if (array[i].Key == key)
			{
				return array[i].trace_params;
			}
		}
	}

	return FDragonIKTraceMParams();
}

void UDragonIKTraceManagerComponent::PendingTraces_SetValueOnKey(TArray<FDragonIKTraceParamKeyValuePair>& array, FName key, FDragonIKTraceMParams value)
{
	{
		bool key_found = false;
		for (int i = 0; i < array.Num(); i++)
		{
			if (array[i].Key == key)
			{
				array[i].trace_params = value;
				key_found = true;
				return;
			}
		}

		if (key_found == false)
		{
			FDragonIKTraceParamKeyValuePair trace_value_pair = FDragonIKTraceParamKeyValuePair();
			trace_value_pair.Key = key;
			trace_value_pair.trace_params = value;

			array.Add(trace_value_pair);
		}
	}
}

// Called when the game starts
void UDragonIKTraceManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	{
		FScopeLock LockPending(&PendingCS);
		FScopeLock LockResults(&ResultsCS);

		Solver_Markers.Reset();
		PendingTraces.Reset();
		HitResultMap.Reset();

		PendingMap.Reset();
		ResultsMap.Reset();

		// Avoid churn; typical usage is a handful to a few dozen traces.
		Solver_Markers.Reserve(64);
		PendingTraces.Reserve(64);
		HitResultMap.Reserve(128);
	}

	OnDragonikFootHitData.AddDynamic(this, &UDragonIKTraceManagerComponent::SpineSolverTraceData);

	// ...

}


// Called every frame
void UDragonIKTraceManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Is_Enabled)
	{
		return;
	}

	TArray<FName> Solver_Const_Markers;
	TMap<FName, FDragonIKTraceMParams> PendingLocal;
	bool bHandShakedLocal = false;

	{
		FScopeLock Lock(&PendingCS);
		bHandShakedLocal = hand_shaked;
		Solver_Const_Markers = Solver_Markers;
		PendingLocal = PendingMap;
	}

	if (!bHandShakedLocal || Solver_Const_Markers.Num() == 0 || PendingLocal.Num() == 0)
	{
		return;
	}

	TArray<FDragonIKTraceKeyValuePair> UpdatedPairs;
	UpdatedPairs.Reserve(Solver_Const_Markers.Num());

	for (int32 i = 0; i < Solver_Const_Markers.Num(); i++)
	{
		const FName Marker = Solver_Const_Markers[i];
		if (!Marker.IsValid() || Marker.IsNone())
		{
			continue;
		}

		const FDragonIKTraceMParams* CurrentParam = PendingLocal.Find(Marker);
		if (CurrentParam == nullptr)
		{
			// Legacy fallback: use the marker as key into PendingTraces.
			FScopeLock Lock(&PendingCS);
			const FDragonIKTraceMParams legacy_param = PendingTraces_GetValueFromKey(PendingTraces, Marker);
			PendingLocal.Add(Marker, legacy_param);
			CurrentParam = PendingLocal.Find(Marker);
			if (CurrentParam == nullptr)
			{
				continue;
			}
		}

		FHitResult HitResult;
		TArray<AActor*> actor_to_ignore;
		actor_to_ignore.Add(GetOwner());

		const FString solver_type = Marker.ToString().Left(1);
		const bool bShowDebug = (solver_type == "F" && bShow_Foot_Trace_Lines_InGame) || (solver_type == "S" && bShow_Spine_Trace_Lines_InGame);
		const EDrawDebugTrace::Type DrawType = bShowDebug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

		if (CurrentParam->TraceType == EDragonIKTraceMType::LineTrace)
		{
			UKismetSystemLibrary::LineTraceSingle(GetWorld(), CurrentParam->StartLocation, CurrentParam->EndLocation, CurrentParam->ChannelInput, CurrentParam->trace_complex, actor_to_ignore, DrawType, HitResult, true);
		}
		else if (CurrentParam->TraceType == EDragonIKTraceMType::SphereTrace)
		{
			UKismetSystemLibrary::SphereTraceSingle(GetWorld(), CurrentParam->StartLocation, CurrentParam->EndLocation, CurrentParam->SphereRadius, CurrentParam->ChannelInput, CurrentParam->trace_complex, actor_to_ignore, DrawType, HitResult, true);
		}

		FDragonIKTraceKeyValuePair Pair;
		Pair.Key = Marker;
		Pair.BoneName = CurrentParam->bone_text;
		Pair.hit_result = HitResult;
		UpdatedPairs.Add(Pair);
	}

	{
		FScopeLock Lock(&ResultsCS);
		for (const FDragonIKTraceKeyValuePair& Pair : UpdatedPairs)
		{
			ResultsMap.Add(Pair.Key, Pair);
		}

		// Rebuild the legacy array to match the newest results.
		HitResultMap.Reset(ResultsMap.Num());
		for (const TPair<FName, FDragonIKTraceKeyValuePair>& KV : ResultsMap)
		{
			HitResultMap.Add(KV.Value);
		}
	}

	{
		TArray<FDragonIKTraceKeyValuePair> footsolver_hitdata;
		{
			FScopeLock Lock(&ResultsCS);
			footsolver_hitdata.Reserve(ResultsMap.Num());
			for (const TPair<FName, FDragonIKTraceKeyValuePair>& KV : ResultsMap)
			{
				if (KV.Key.ToString().Left(1).Equals("F"))
				{
					footsolver_hitdata.Add(KV.Value);
				}
			}
		}
		OnDragonikFootHitData.Broadcast(footsolver_hitdata);
	}



}

