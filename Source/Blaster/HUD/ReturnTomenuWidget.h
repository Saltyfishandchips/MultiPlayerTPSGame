// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Blueprint/UserWidget.h"
#include "ReturnTomenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UReturnTomenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();
	void MenuSetDown();

protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();
	
private:
	UPROPERTY()
	class ABlasterPlayerController* PlayerController;

	UPROPERTY(meta=(BindWidget))
	class UButton* ReturnToMenuButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionSubsystem* Subsystem;
};
