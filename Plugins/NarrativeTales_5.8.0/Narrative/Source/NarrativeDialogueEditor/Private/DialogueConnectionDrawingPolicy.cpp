// Copyright Narrative Tools 2025. 

#include "DialogueConnectionDrawingPolicy.h"
#include "NarrativeDialogueSettings.h"
#include <EdGraph/EdGraphNode.h>
#include "DialogueEditorSettings.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
using FGraphLocationType = FVector2f;
#else
using FGraphLocationType = FVector2D;
#endif

FGraphLocationType FDialogueGraphConnectionDrawingPolicy::ComputeSplineTangent(const FGraphLocationType& Start, const FGraphLocationType& End) const
{

	if (const UNarrativeDialogueSettings* DialogueSettings = GetDefault<UNarrativeDialogueSettings>())
	{
		const FGraphLocationType DeltaPos = End - Start;
		const bool bGoingForward = DialogueSettings->bEnableVerticalWiring ? DeltaPos.Y >= 0.f : DeltaPos.X >= 0.0f;

		if (const UDialogueEditorSettings* DialogueEditorSettings = GetDefault<UDialogueEditorSettings>())
		{
			const float ClampedTensionX = FMath::Min<float>(FMath::Abs<float>(DeltaPos.X), bGoingForward ? DialogueEditorSettings->ForwardSplineHorizontalDeltaRange : DialogueEditorSettings->BackwardSplineHorizontalDeltaRange);
			const float ClampedTensionY = FMath::Min<float>(FMath::Abs<float>(DeltaPos.Y), bGoingForward ? DialogueEditorSettings->ForwardSplineVerticalDeltaRange : DialogueEditorSettings->BackwardSplineVerticalDeltaRange);

			FVector2D Result;

			if (bGoingForward)
			{
				Result = (ClampedTensionX * DialogueEditorSettings->ForwardSplineTangentFromHorizontalDelta) + (ClampedTensionY * DialogueEditorSettings->ForwardSplineTangentFromVerticalDelta);
			}
			else
			{
				Result = (ClampedTensionX * DialogueEditorSettings->BackwardSplineTangentFromHorizontalDelta) + (ClampedTensionY * DialogueEditorSettings->BackwardSplineTangentFromVerticalDelta);
			}

			if (DialogueSettings->bEnableVerticalWiring)
			{
				return FGraphLocationType(Result.GetRotated(90.f));
			}
			else
			{
				return FGraphLocationType(Result);
			}

		}


	}

	return FConnectionDrawingPolicy::ComputeSplineTangent(Start, End);

}

void FDialogueGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params)
{
	Params.WireColor = FLinearColor::White;
	Params.WireThickness = 2.f;

	UEdGraphNode* OutNode = OutputPin ? OutputPin->GetOwningNode() : nullptr;
	UEdGraphNode* InNode = InputPin ? InputPin->GetOwningNode() : nullptr;

	if (const UNarrativeDialogueSettings* DialogueSettings = GetDefault<UNarrativeDialogueSettings>())
	{
		if (const UDialogueEditorSettings* DialogueEditorSettings = GetDefault<UDialogueEditorSettings>())
		{
			if (OutNode && InNode)
			{
				if (DialogueSettings->bEnableVerticalWiring ? OutNode->NodePosY > InNode->NodePosY : OutNode->NodePosX > InNode->NodePosX)
				{
					Params.WireColor = DialogueEditorSettings->BacklinkWireColor;
				}
			}
		}
	}
}

void FDialogueGraphConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FGraphLocationType& Start, const FGraphLocationType& End, const FConnectionParams& Params)
{
	if (const UNarrativeDialogueSettings* DialogueSettings = GetDefault<UNarrativeDialogueSettings>())
	{
		const bool bLinkingBackwards = DialogueSettings->bEnableVerticalWiring ? Start.Y > End.Y : Start.X > End.X;

		//If we're linking backwards, actually draw two splines that meet halfway, to act like a reroute pin might.
		//Basically just makes backlinks beautiful instead of messy and hard to follow
		if (bLinkingBackwards)
		{
			FGraphLocationType Halfway = (Start + End) / 2;

			if (DialogueSettings->bEnableVerticalWiring)
			{
				Halfway.X -= FMath::Min(150.f * ZoomFactor + (FMath::Abs(Start.Y - End.Y) / 2.f), 400.f);
			}
			else
			{
				Halfway.Y -= FMath::Min(150.f * ZoomFactor + (FMath::Abs(Start.X - End.X) / 2.f), 400.f);
			}

			//Draw backlinks with pins changed to make the reroute nodes style themselves a lot more nicely 
			FConnectionParams NewParam = Params;
			NewParam.EndDirection = EGPD_Output;
			NewParam.StartDirection = EGPD_Output;
			FConnectionDrawingPolicy::DrawConnection(LayerId, Halfway, Start, NewParam);
			NewParam.EndDirection = EGPD_Input;
			NewParam.StartDirection = EGPD_Input;
			FConnectionDrawingPolicy::DrawConnection(LayerId, End, Halfway, NewParam);
			
			//// Draw the arrow
			if (BacklinkImage != nullptr)
			{
				FGraphLocationType ArrowPoint = Halfway - ArrowRadius * 2.f;

				const float Angle = DialogueSettings->bEnableVerticalWiring ? -HALF_PI : PI;

				FSlateDrawElement::MakeRotatedBox(
					DrawElementsList,
					ArrowLayerID,
					FPaintGeometry(ArrowPoint, (BacklinkImage->ImageSize * 2.f) * ZoomFactor, ZoomFactor),
					BacklinkImage,
					ESlateDrawEffect::None,
					Angle,
					TOptional<FGraphLocationType>(),
					FSlateDrawElement::RelativeToElement,
					Params.WireColor
				);
			}
		}
		else
		{
			FConnectionDrawingPolicy::DrawConnection(LayerId, Start, End, Params);
		}
	}


}


void FDialogueGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FGraphLocationType& StartPoint, const FGraphLocationType& EndPoint, UEdGraphPin* Pin)
{
	bool bDrawVerticalWiring = false;

	if (const UNarrativeDialogueSettings* DialogueSettings = GetDefault<UNarrativeDialogueSettings>())
	{
		 bDrawVerticalWiring = DialogueSettings->bEnableVerticalWiring;
	}

	FGraphLocationType AdjustedStartPoint = StartPoint;

	if (bDrawVerticalWiring)
	{
		AdjustedStartPoint.X -= PinGeometry.GetDrawSize().X / 2;
		AdjustedStartPoint.Y -= PinGeometry.GetDrawSize().Y / 2;
	}
	
	FConnectionDrawingPolicy::DrawPreviewConnector(PinGeometry, AdjustedStartPoint, EndPoint, Pin);
}

void FDialogueGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
	bool bDrawVerticalWiring = false;

	if (const UNarrativeDialogueSettings* DialogueSettings = GetDefault<UNarrativeDialogueSettings>())
	{
		 bDrawVerticalWiring = DialogueSettings->bEnableVerticalWiring;
	}

	if (bDrawVerticalWiring)
	{

		const FGraphLocationType StartPoint = FNarrativeGeometryHelper::HorizontalBottomOf(StartGeom);
		const FGraphLocationType EndPoint = FNarrativeGeometryHelper::HorizontalTopOf(EndGeom);

		FConnectionDrawingPolicy::DrawSplineWithArrow(StartPoint, EndPoint, Params);
	}
	else
	{
		//@TODO: These values should be pushed into the Slate style, they are compensating for a bit of
		// empty space inside of the pin brush images.
		const float StartFudgeX = 4.0f;
		const float EndFudgeX = 4.0f;
		const FGraphLocationType StartPoint = FGeometryHelper::VerticalMiddleRightOf(StartGeom) - FVector2D(StartFudgeX, 0.0f);
		const FGraphLocationType EndPoint = FGeometryHelper::VerticalMiddleLeftOf(EndGeom) - FVector2D(ArrowRadius.X - EndFudgeX, 0);

		FConnectionDrawingPolicy::DrawSplineWithArrow(StartPoint, EndPoint, Params);
	}
}
