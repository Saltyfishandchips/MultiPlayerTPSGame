// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef MULTIPLAYERSESSIONS_MultiplayerSessionSubsystem_generated_h
#error "MultiplayerSessionSubsystem.generated.h already included, missing '#pragma once' in MultiplayerSessionSubsystem.h"
#endif
#define MULTIPLAYERSESSIONS_MultiplayerSessionSubsystem_generated_h

#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_25_DELEGATE \
struct _Script_MultiPlayerSessions_eventMultiplayerOnDestroySessionComplete_Parms \
{ \
	bool bWasSuccessful; \
}; \
static inline void FMultiplayerOnDestroySessionComplete_DelegateWrapper(const FMulticastScriptDelegate& MultiplayerOnDestroySessionComplete, bool bWasSuccessful) \
{ \
	_Script_MultiPlayerSessions_eventMultiplayerOnDestroySessionComplete_Parms Parms; \
	Parms.bWasSuccessful=bWasSuccessful ? true : false; \
	MultiplayerOnDestroySessionComplete.ProcessMulticastDelegate<UObject>(&Parms); \
}


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_24_DELEGATE \
struct _Script_MultiPlayerSessions_eventMultiplayerOnStartSessionComplete_Parms \
{ \
	bool bWasSuccessful; \
}; \
static inline void FMultiplayerOnStartSessionComplete_DelegateWrapper(const FMulticastScriptDelegate& MultiplayerOnStartSessionComplete, bool bWasSuccessful) \
{ \
	_Script_MultiPlayerSessions_eventMultiplayerOnStartSessionComplete_Parms Parms; \
	Parms.bWasSuccessful=bWasSuccessful ? true : false; \
	MultiplayerOnStartSessionComplete.ProcessMulticastDelegate<UObject>(&Parms); \
}


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_21_DELEGATE \
struct _Script_MultiPlayerSessions_eventMultiplayerOnCreateSessionComplete_Parms \
{ \
	bool bWasSuccessful; \
}; \
static inline void FMultiplayerOnCreateSessionComplete_DelegateWrapper(const FMulticastScriptDelegate& MultiplayerOnCreateSessionComplete, bool bWasSuccessful) \
{ \
	_Script_MultiPlayerSessions_eventMultiplayerOnCreateSessionComplete_Parms Parms; \
	Parms.bWasSuccessful=bWasSuccessful ? true : false; \
	MultiplayerOnCreateSessionComplete.ProcessMulticastDelegate<UObject>(&Parms); \
}


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_SPARSE_DATA
#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_RPC_WRAPPERS
#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_RPC_WRAPPERS_NO_PURE_DECLS
#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUMultiplayerSessionSubsystem(); \
	friend struct Z_Construct_UClass_UMultiplayerSessionSubsystem_Statics; \
public: \
	DECLARE_CLASS(UMultiplayerSessionSubsystem, UGameInstanceSubsystem, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/MultiPlayerSessions"), NO_API) \
	DECLARE_SERIALIZER(UMultiplayerSessionSubsystem)


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_INCLASS \
private: \
	static void StaticRegisterNativesUMultiplayerSessionSubsystem(); \
	friend struct Z_Construct_UClass_UMultiplayerSessionSubsystem_Statics; \
public: \
	DECLARE_CLASS(UMultiplayerSessionSubsystem, UGameInstanceSubsystem, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/MultiPlayerSessions"), NO_API) \
	DECLARE_SERIALIZER(UMultiplayerSessionSubsystem)


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UMultiplayerSessionSubsystem(const FObjectInitializer& ObjectInitializer); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UMultiplayerSessionSubsystem) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UMultiplayerSessionSubsystem); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UMultiplayerSessionSubsystem); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UMultiplayerSessionSubsystem(UMultiplayerSessionSubsystem&&); \
	NO_API UMultiplayerSessionSubsystem(const UMultiplayerSessionSubsystem&); \
public:


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UMultiplayerSessionSubsystem(UMultiplayerSessionSubsystem&&); \
	NO_API UMultiplayerSessionSubsystem(const UMultiplayerSessionSubsystem&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UMultiplayerSessionSubsystem); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UMultiplayerSessionSubsystem); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(UMultiplayerSessionSubsystem)


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_PRIVATE_PROPERTY_OFFSET
#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_30_PROLOG
#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_PRIVATE_PROPERTY_OFFSET \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_SPARSE_DATA \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_RPC_WRAPPERS \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_INCLASS \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_PRIVATE_PROPERTY_OFFSET \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_SPARSE_DATA \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_RPC_WRAPPERS_NO_PURE_DECLS \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_INCLASS_NO_PURE_DECLS \
	MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h_33_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> MULTIPLAYERSESSIONS_API UClass* StaticClass<class UMultiplayerSessionSubsystem>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID MultiPlayerTPSGame_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_MultiplayerSessionSubsystem_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
