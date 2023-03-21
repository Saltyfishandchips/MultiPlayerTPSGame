// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();\
		check(Subsystem)

		
		if (NumberOfPlayers == Subsystem->DesireNumPublicConnections)
		{
			
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;

				FString MatchType = Subsystem->DesiredMatchTyped;
				if (MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}
				else if (MatchType == "Team")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterTeamMap?listen"));
				}
			}
		}
	}
}
