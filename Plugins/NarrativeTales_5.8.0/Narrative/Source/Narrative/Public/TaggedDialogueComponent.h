// Copyright Narrative Tools 2025. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/TextProperty.h" //Fixes a build error complaining about incomplete type UTextProperty
#include "Components/ActorComponent.h"
#include "Dialogue.h"
#include "Engine/DataAsset.h"
#include "TaggedDialogueComponent.generated.h"

//Represents a tagged dialogue - this is essentially a dialogue that can be kicked off via a tag "TaggedDialogue.Taunt, TaggedDialogue.Greet, etc. "
USTRUCT(BlueprintType)
struct NARRATIVE_API FTaggedDialogue
{
	GENERATED_BODY()

	FTaggedDialogue() 
	{
		Cooldown = 30.f;
		MaxDistance = 5000.f;
		
		PlayParams.bOverride_bFreeMovement = true;
		PlayParams.bFreeMovement = true;
		PlayParams.bOverride_bUnskippable = false;
		PlayParams.bUnskippable = false;
	};

	/** The tag that will kick off this dialogue. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tagged Dialogue", meta = (Categories = "Narrative.TaggedDialogue"))
	FGameplayTag Tag;

	/** The dialogue to begin */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tagged Dialogue")
	TSoftClassPtr<class UDialogue> Dialogue;

	/** The dialogue play params */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tagged Dialogue")
	FDialoguePlayParams PlayParams;

	/** The amount of time we should cooldown before playing this dialogue again. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tagged Dialogue")
	float Cooldown;

	/** Tagged dialogue wont play unless we're within this range from it */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tagged Dialogue")
	float MaxDistance;

	/** Tags that will be required for the NPC to begin this tagged dialogue */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tagged Dialogue")
	FGameplayTagContainer RequiredTags;

	/** Tags that if owned by the NPC, will prevent this dialogue beginning. For example, we wouldn't want to greet a player if we were fighting someone. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Tagged Dialogue")
	FGameplayTagContainer BlockedTags;
};

/**
 * Contains various dialogues to play when a NPC greets, threatens, investigates, and so on. 
 */
UCLASS()
class NARRATIVE_API UTaggedDialogueSet : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UTaggedDialogueSet(const FObjectInitializer& ObjectInitializer);

	/** The NPCs tagged dialogues these are essentially dialogue, usually free movement that can be kicked off via a tag "TaggedDialogue.Taunt, TaggedDialogue.Greet, etc. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Dialogue")
	TArray<FTaggedDialogue> TaggedDialogues;

};

/**
* Add this component to an NPC or actor you want to have tagged dialogue. 
* Tagged Dialogue Component acts as the storage for a Tagged Dialogue Set.
*/
UCLASS( ClassGroup=(Narrative), DisplayName = "Tagged Dialogue Component", meta=(BlueprintSpawnableComponent) )
class NARRATIVE_API UTaggedDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	// Sets default values for this component's properties
	UTaggedDialogueComponent();

public:

	/** Return the tagged dialogue array this actor has. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tagged Dialogue")
	TSoftObjectPtr<class UTaggedDialogueSet> TaggedDialogueSet;

	/** Return the tagged dialogue array this actor has. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tagged Dialogue")
	bool PlayTaggedDialogue(FGameplayTag Tag, AActor* DialogueInstigator);
	virtual bool PlayTaggedDialogue_Implementation(FGameplayTag Tag, AActor* DialogueInstigator);

	//Notify the NPC a tagged dialogue should try play - this is blueprint implementable 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tagged Dialogue")
	bool ExecutePlayTaggedDialogue(FTaggedDialogue Dialogue, AActor* DialogueInstigator);
	virtual bool ExecutePlayTaggedDialogue_Implementation(FTaggedDialogue Dialogue, AActor* DialogueInstigator);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tagged Dialogue")
	TMap<FGameplayTag, float> LastPlayTimes;
};
