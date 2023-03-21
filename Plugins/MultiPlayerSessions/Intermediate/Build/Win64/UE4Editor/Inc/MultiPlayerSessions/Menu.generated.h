// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef MULTIPLAYERSESSIONS_Menu_generated_h
#error "Menu.generated.h already included, missing '#pragma once' in Menu.h"
#endif
#define MULTIPLAYERSESSIONS_Menu_generated_h

#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_SPARSE_DATA
#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execJoinButtonClicked); \
	DECLARE_FUNCTION(execHostButtonClicked); \
	DECLARE_FUNCTION(execOnStartSession); \
	DECLARE_FUNCTION(execOnDestroySession); \
	DECLARE_FUNCTION(execOnCreateSession); \
	DECLARE_FUNCTION(execMenuSetUp);


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execJoinButtonClicked); \
	DECLARE_FUNCTION(execHostButtonClicked); \
	DECLARE_FUNCTION(execOnStartSession); \
	DECLARE_FUNCTION(execOnDestroySession); \
	DECLARE_FUNCTION(execOnCreateSession); \
	DECLARE_FUNCTION(execMenuSetUp);


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUMenu(); \
	friend struct Z_Construct_UClass_UMenu_Statics; \
public: \
	DECLARE_CLASS(UMenu, UUserWidget, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/MultiPlayerSessions"), NO_API) \
	DECLARE_SERIALIZER(UMenu)


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_INCLASS \
private: \
	static void StaticRegisterNativesUMenu(); \
	friend struct Z_Construct_UClass_UMenu_Statics; \
public: \
	DECLARE_CLASS(UMenu, UUserWidget, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/MultiPlayerSessions"), NO_API) \
	DECLARE_SERIALIZER(UMenu)


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UMenu(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UMenu) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UMenu); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UMenu); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UMenu(UMenu&&); \
	NO_API UMenu(const UMenu&); \
public:


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UMenu(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UMenu(UMenu&&); \
	NO_API UMenu(const UMenu&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UMenu); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UMenu); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UMenu)


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_PRIVATE_PROPERTY_OFFSET \
	FORCEINLINE static uint32 __PPO__HostButton() { return STRUCT_OFFSET(UMenu, HostButton); } \
	FORCEINLINE static uint32 __PPO__JoinButton() { return STRUCT_OFFSET(UMenu, JoinButton); } \
	FORCEINLINE static uint32 __PPO__NumPublicConnections() { return STRUCT_OFFSET(UMenu, NumPublicConnections); } \
	FORCEINLINE static uint32 __PPO__MatchType() { return STRUCT_OFFSET(UMenu, MatchType); }


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_15_PROLOG
#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_PRIVATE_PROPERTY_OFFSET \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_SPARSE_DATA \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_RPC_WRAPPERS \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_INCLASS \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_PRIVATE_PROPERTY_OFFSET \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_SPARSE_DATA \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_RPC_WRAPPERS_NO_PURE_DECLS \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_INCLASS_NO_PURE_DECLS \
	Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h_18_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> MULTIPLAYERSESSIONS_API UClass* StaticClass<class UMenu>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID Blaster_Plugins_MultiPlayerSessions_Source_MultiPlayerSessions_Public_Menu_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
