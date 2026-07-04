// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "TraceHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Editor/EditorEngine.h"
#include "Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "HitProxies.h"
#include "SEditorViewport.h"

AActor * FRocketTraceHelper::GetActorUnderMousePosition(const FVector2D &ScreenPosition) {

	if (!GEditor)
	{
		return nullptr;
	}

	FLevelEditorViewportClient* ViewportClient = static_cast<FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());

	if (!ViewportClient)
	{
		return nullptr;
	}

	// Get the position of the viewport within the screen
	TSharedPtr<SEditorViewport> ViewportWidget = ViewportClient->GetEditorViewportWidget();
	if (!ViewportWidget.IsValid())
	{
		return nullptr;
	}

	const FGeometry& ViewportGeometry = ViewportWidget->GetCachedGeometry();
	
	FVector2D LocalMousePosition = ViewportGeometry.AbsoluteToLocal(ScreenPosition);

	// Convert local position to integer coordinates for GetHitProxy
	int32 MouseX = static_cast<int32>(LocalMousePosition.X);
	int32 MouseY = static_cast<int32>(LocalMousePosition.Y);

	// Get the hit proxy under the cursor
	HHitProxy* HitProxy = ViewportClient->Viewport->GetHitProxy(MouseX, MouseY);

	if (HitProxy && HitProxy->IsA(HActor::StaticGetType()))
	{
		HActor* ActorProxy = static_cast<HActor*>(HitProxy);
		AActor* HitActor = ActorProxy->Actor;

		// Check if the hit actor has a static mesh component
		if (HitActor && HitActor->FindComponentByClass<UStaticMeshComponent>())
		{
			return HitActor;
		}
	}
	return nullptr;
}

