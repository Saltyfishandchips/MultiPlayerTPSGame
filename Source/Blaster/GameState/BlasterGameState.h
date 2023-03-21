// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	/*
	 * 团队模式
	 */

	void RedTeamScores();
	void BlueTeamScores();
	
	UPROPERTY()
	TArray<ABlasterPlayerState*> RedTeam;
	UPROPERTY()
	TArray<ABlasterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing= OnRep_RedTeamScore)
	float RedTeamScore = 0.f;
	UFUNCTION()
	void OnRep_RedTeamScore();
	
	UPROPERTY(ReplicatedUsing= OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;
	UFUNCTION()
	void OnRep_BlueTeamScore();
	
private:

	float TopScore = 0.f;
};
