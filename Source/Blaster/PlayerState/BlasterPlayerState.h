// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlasterCharacter;
class ABlasterPlayerController;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void AddToScore(float ScoreAmount);
	void AddToDefeat(float DefeatAmount);
	
	// RepNotify
	virtual void OnRep_Score() override;
	UFUNCTION()
	void OnRep_Defeat();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
private:
	UPROPERTY()
	ABlasterCharacter* BlasterCharacter;
	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;
	
	UPROPERTY(ReplicatedUsing= OnRep_Defeat)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing= OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;
	UFUNCTION()
	void OnRep_Team();
public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	void SetTeam(ETeam TeamToSet);
	
};
