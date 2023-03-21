// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetElimAnnouncement(FString AttackerName, FString VictimName);
	
public:

	UPROPERTY(meta=(BindWidget))
	class UHorizontalBox* Elimbox;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ElimText;
};
