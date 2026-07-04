// Copyright Narrative Tools 2025. 

#pragma once

#include "CoreMinimal.h"
#include "ConnectionDrawingPolicy.h"

class FNarrativeGeometryHelper
{

public: 
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
	static FVector2f HorizontalBottomOf(const FGeometry& SomeGeometry)
	{
		const FVector2f GeometryDrawSize = SomeGeometry.GetDrawSize();
		return FVector2f(
			SomeGeometry.AbsolutePosition.X + (GeometryDrawSize.X / 2.f),
			SomeGeometry.AbsolutePosition.Y + GeometryDrawSize.Y);
	};

	static FVector2f HorizontalTopOf(const FGeometry& SomeGeometry)
	{
		const FVector2f GeometryDrawSize = SomeGeometry.GetDrawSize();
		return FVector2f(
			SomeGeometry.AbsolutePosition.X + (GeometryDrawSize.X / 2.f),
			SomeGeometry.AbsolutePosition.Y);
	};
#else
	static FVector2d HorizontalBottomOf(const FGeometry& SomeGeometry)
	{
		const FVector2d GeometryDrawSize = SomeGeometry.GetDrawSize();
		return FVector2d(
			SomeGeometry.AbsolutePosition.X + (GeometryDrawSize.X / 2.f),
			SomeGeometry.AbsolutePosition.Y + GeometryDrawSize.Y);
	};

	static FVector2d HorizontalTopOf(const FGeometry& SomeGeometry)
	{
		const FVector2d GeometryDrawSize = SomeGeometry.GetDrawSize();
		return FVector2d(
			SomeGeometry.AbsolutePosition.X + (GeometryDrawSize.X / 2.f),
			SomeGeometry.AbsolutePosition.Y);
	};
#endif

};

class FDialogueGraphConnectionDrawingPolicy : public FConnectionDrawingPolicy
{

public:
	FDialogueGraphConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
		: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements)
	{
		TransitionColor = FLinearColor(0.92f, 0.72f, 0.2f, 1.f);
		TransitionTime = 1000.f;
		BacklinkImage = ArrowImage;
		ArrowImage = nullptr; // dont draw arrows
		
	}

	void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params);

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
	using FGraphLocationType = FVector2f;
#else
	using FGraphLocationType = FVector2D;
#endif
	
	virtual FGraphLocationType ComputeSplineTangent(const FGraphLocationType& Start, const FGraphLocationType& End) const override;
	virtual void DrawConnection(int32 LayerId, const FGraphLocationType& Start, const FGraphLocationType& End, const FConnectionParams& Params) override;
	virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FGraphLocationType& StartPoint, const FGraphLocationType& EndPoint, UEdGraphPin* Pin) override;

	virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params);

	const FSlateBrush* BacklinkImage;

	FLinearColor TransitionColor;
	float TransitionTime; //in ms
};
