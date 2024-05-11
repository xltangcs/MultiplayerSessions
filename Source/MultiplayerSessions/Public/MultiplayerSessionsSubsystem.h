// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "MultiplayerSessionsSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnCreateSessionCompleteDelegate, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FSubsystemOnFindSessionsCompleteDelegate, const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FSubsystemOnJoinSessionCompleteDelegate, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnDestroySessionCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemOnStartSessionCompleteDelegate, bool, bWasSuccessful);


UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();

	void CreateSession(int32 NumpublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);	
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

	FSubsystemOnCreateSessionCompleteDelegate SubsystemOnCreateSessionCompleteDelegate;
	FSubsystemOnFindSessionsCompleteDelegate SubsystemOnFindSessionsCompleteDelegate;
	FSubsystemOnJoinSessionCompleteDelegate SubsystemOnJoinSessionCompleteDelegate;
	FSubsystemOnDestroySessionCompleteDelegate SubsystemOnDestroySessionCompleteDelegate;
	FSubsystemOnStartSessionCompleteDelegate SubsystemOnStartSessionCompleteDelegate;

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);


private:
	IOnlineSessionPtr OnlineSessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	bool bCreateSessionOnDestroy {false};
	int32 LastNumPublicConnections;
	FString LastMatchType;
	

	// Complete Delegate
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;	
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;	
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;		
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;	

	// Delegate Handle
	FDelegateHandle CreateSessionCompleteDelegateHandle;				
	FDelegateHandle FindSessionsCompleteDelegateHandle;					
	FDelegateHandle JoinSessionCompleteDelegateHandle;					
	FDelegateHandle DestroySessionCompleteDelegateHandle;				
	FDelegateHandle StartSessionCompleteDelegateHandle;					
};
