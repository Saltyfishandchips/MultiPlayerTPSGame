// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnTomenuWidget.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/GameModeBase.h"

bool UReturnTomenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (ReturnToMenuButton)
	{
		ReturnToMenuButton->OnClicked.AddDynamic(this, &ThisClass::ReturnButtonClicked);
	}
	return true;
}

void UReturnTomenuWidget::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnToMenuButton->SetIsEnabled(true);
		return;
	}
	
	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameModeBase = World->GetAuthGameMode<AGameModeBase>();
		if (GameModeBase) //Server
		{
			GameModeBase->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr? Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()): PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	
	}
}


void UReturnTomenuWidget::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr? Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()): PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	const UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
		if (Subsystem)
		{
			Subsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		}
	}
}

void UReturnTomenuWidget::MenuSetDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr? Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()): PlayerController;
		if (PlayerController)
		{
			const FInputModeGameOnly InputMode;
			PlayerController->SetInputMode(InputMode);
			PlayerController->SetShowMouseCursor(false);
		}
		if (ReturnToMenuButton && ReturnToMenuButton->OnClicked.IsBound())
		{
			ReturnToMenuButton->OnClicked.RemoveDynamic(this, &UReturnTomenuWidget::ReturnButtonClicked);
		}
		if (Subsystem && Subsystem->MultiplayerOnDestroySessionComplete.IsBound())
		{
			Subsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnTomenuWidget::OnDestroySession);
		}
	}
}

void UReturnTomenuWidget::ReturnButtonClicked()
{
	ReturnToMenuButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			ABlasterCharacter* BlasterCharacter = FirstPlayerController->GetPawn<ABlasterCharacter>();
			if (BlasterCharacter)
			{
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &ThisClass::OnPlayerLeftGame);
			}
			else
			{
				ReturnToMenuButton->SetIsEnabled(true);
			}
		}
	}
}

void UReturnTomenuWidget::OnPlayerLeftGame()
{
	if (Subsystem)
	{
		Subsystem->DestroySession();
	}
}