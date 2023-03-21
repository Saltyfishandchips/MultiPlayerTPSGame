
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

class ABlasterHUD;

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABlasterPlayerController();
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDefeat(int32 Defeat);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDWarmupCountdown(float CountdownTime);
	void SetHUDGrenade(int32 Grenades);
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	void HideTeamScores();
	void InitTeamScores();
	void SetHUDRedTeamScore(int32 RedScore);
	void SetHUDBlueTeamScore(int32 BlueScore);
	
	//同步客户端和服务端之间的时间
	virtual float GetServerTime();
	//尽早同步时间
	virtual void ReceivedPlayer() override;

	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HandleMatchStarted(bool bTeamsMatch = false);
	void HandleCooldown();
	
	float SingleRoundTripTime = 0;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	void ShowReturnToMenu();
	
	void SetHUDTime();
	void PollInit();
	
	/*
	 * 同步客户端和服务器的时间
	 */
	//向服务端发送请求，输入参数为当前客户端的时间
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//服务端发回时间
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f;// 客户端与服务端时间的Delta

	UPROPERTY(EditAnywhere, Category="Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	//客户端可能会在游戏进行到一半时加入，因此我们需要使用ClientRPC下发数据
	//这只会在Client加入时执行一次
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float LevelStarting);

	void HighPingWarning();
	void StopHighPingWarning();
	float HighPingRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
	float PingAnimationRunningTime = 0.f;
	
	void CheckPing(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing= OnRep_ShowTeamScores)
	bool bShowTeamScores = false;
	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetScoreInfoText(TArray<class ABlasterPlayerState*> TopPlayers);
	FString SetTeamScoreInfoText();
	
private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	/*
	 * 返回菜单的Widget
	 */
	UPROPERTY(EditAnywhere, Category="HUD")
	TSubclassOf<class UUserWidget> ReturnToMenu;
	UPROPERTY()
	class UReturnTomenuWidget* ReturnToMenuWidget;
	bool bReturnToMenuOpen = false;
	
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;
	
	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing= OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;


	//HUD在初始化时可能Controller还没有产生，因此先缓存初始数据，当HUD有效时赋值
	float HUDMaxHealth;
	float HUDHealth;
	bool bInitializeHealth = false;
	float HUDMaxShield;
	float HUDShield;
	bool bInitializeShield = false;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
};


