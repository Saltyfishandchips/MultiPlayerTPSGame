// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"
#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncement(FString AttackerName, FString VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s 击杀了 %s"), *AttackerName, *VictimName);
	if (ElimText)
	{
		ElimText->SetText(FText::FromString(ElimAnnouncementText));
	}
	
}
