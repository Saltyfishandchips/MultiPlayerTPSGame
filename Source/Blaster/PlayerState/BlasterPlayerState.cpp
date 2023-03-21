// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	BlasterCharacter = BlasterCharacter == nullptr? Cast<ABlasterCharacter>(GetPawn()): BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr? Cast<ABlasterPlayerController>(BlasterCharacter->Controller): BlasterPlayerController;
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetHUDScore(GetScore());
		}
	}
}

//OnRep只会从服务端发送至客户端，在客户端中运行，因此在服务端中还需要执行一次OnRep里面的功能。
void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	BlasterCharacter = BlasterCharacter == nullptr? Cast<ABlasterCharacter>(GetPawn()): BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr? Cast<ABlasterPlayerController>(BlasterCharacter->Controller): BlasterPlayerController;
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::AddToDefeat(float DefeatAmount)
{
	Defeats += DefeatAmount;
	BlasterCharacter = BlasterCharacter == nullptr? Cast<ABlasterCharacter>(GetPawn()): BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr? Cast<ABlasterPlayerController>(BlasterCharacter->Controller): BlasterPlayerController;
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetHUDDefeat(Defeats);
		}
	}
}

void ABlasterPlayerState::OnRep_Defeat()
{
	BlasterCharacter = BlasterCharacter == nullptr? Cast<ABlasterCharacter>(GetPawn()): BlasterCharacter;
	if (BlasterCharacter)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr? Cast<ABlasterPlayerController>(BlasterCharacter->Controller): BlasterPlayerController;
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetHUDDefeat(Defeats);
		}
	}
}

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);
}

void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Character->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::OnRep_Team()
{
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Character->SetTeamColor(Team);
	}
}




