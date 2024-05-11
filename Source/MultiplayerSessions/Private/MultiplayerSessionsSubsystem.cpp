// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	if (onlineSubsystem)
	{
		OnlineSessionInterface = onlineSubsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumpublicConnections, FString MatchType)
{
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr) {						// �����ǰ���ڻỰ
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumpublicConnections;
		LastMatchType = MatchType;
		DestroySession();
		return;
		//OnlineSessionInterface->DestroySession(NAME_GameSession);	// ���ٻỰ
	}
	// ����ί�о�����Ա�˺��Ƴ�ί���б�
	CreateSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);	// ���ί�е��Ự�ӿڵ�ί���б�

	// FOnlineSessionSettings ��ͷ�ļ� "OnlineSessionSettings.h" ��
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());	// �����Ự���ã����ú��� MakeShareable ��ʼ��

	// �Ự���ó�Ա�������ļ����壺https://docs.unrealengine.com/5.3/en-US/API/Plugins/OnlineSubsystem/FOnlineSessionSettings/
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;	// �Ự���ã�����ҵ�����ϵͳ����Ϊ ��NULL������ʹ�� LAN ���ӣ�����ʹ��
	LastSessionSettings->NumPublicConnections = NumpublicConnections;	// �Ự���ã�������󹫹�������Ϊ����������� NumpublicConnections
	LastSessionSettings->bAllowJoinInProgress = true;					// �Ự���ã��ڻỰ����ʱ����������Ҽ���
	LastSessionSettings->bAllowJoinViaPresence = true;					// �Ự���ã�Steam ʹ�� Presence �����Ự���ڵ�����ȷ��������������
	LastSessionSettings->bShouldAdvertise = true;						// �Ự���ã����� Steam �����Ự
	LastSessionSettings->bUsesPresence = true;							// �Ự���ã�������ʾ�û� Presence ��Ϣ
	LastSessionSettings->bUseLobbiesIfAvailable = true;					// �Ự���ã�����ѡ�� Lobby API��Steam ֧�� Lobby API��
	LastSessionSettings->BuildUniqueId = 1;
	
	// void FOnlineSessionSettings::Set(FName Key, const FString& Value, EOnlineDataAdvertisementType::Type InType);
	LastSessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);	// �Ự���ã�ƥ������

	// �����Ự
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();	// ��ȡ�������ָ��
	/*
	SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(),			// ��һ����������Ϊ strut FUniqueNetIdRepl�������̳��� struct FUniqueNetIdWrapper
																						// �����װ����������������� *������ʾ * ����һ������ *UniquenetId
									NAME_GameSession,									// �ڶ�����������Ϊ FName SessionName����Ϸ�Ự����
									*LastSessionSettings);								// ��������������Ϊ const FOnlineSessionSettings &NewSessionSettings
	*/
	if (!OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings)) {
		// ����Ự����ʧ�ܣ���ί���Ƴ�ί���б�
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		SubsystemOnCreateSessionCompleteDelegate.Broadcast(false);
	}


}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!OnlineSessionInterface.IsValid()) {
		return;
	}

	// ����ί�о�����Ա�˺��Ƴ�ί���б�
	FindSessionsCompleteDelegateHandle = OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);	// ���ί�е��Ự�ӿڵ�ί���б�

	// https://docs.unrealengine.com/5.0/en-US/API/Plugins/OnlineSubsystem/FOnlineSessionSearch/
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());	// ���ú��� MakeShareable ��ʼ���Ự��������ָ�롣

	// �Ự��������
	LastSessionSearch->MaxSearchResults = 10000;															// �Ự�������ã����������������������óɽϸߵ�����
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;	// �Ự���ã�����ҵ�����ϵͳ����Ϊ ��NULL������ʹ�� LAN ���ӣ�����ʹ��
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);				// �Ự�������ã���ѯ���ã�ȷ���κβ��ҵ��ĻỰ��ʹ���� Presence

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();	//��ȡ�������ָ��
	// SessionInterface->FindSessions()��https://docs.unrealengine.com/5.0/en-US/API/Plugins/OnlineSubsystem/Interfaces/IOnlineSession/FindSessions/2/	
	if (!OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef())) {
		// ��������Ựʧ�ܣ���ί���Ƴ�ί���б�
		OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		// �㲥�����Ự����������ʧ����Ϣ���Զ������ϵͳί��
		SubsystemOnFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!OnlineSessionInterface.IsValid()) {	// ����Ự�ӿ���Ч
		SubsystemOnJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);	// �㲥����ʧ����Ϣ���Զ������ϵͳί��
		return;
	}

	// ����ί�о�����Ա�˺��Ƴ�ί���б�
	JoinSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	// ����Ự
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();	//��ȡ�������ָ��
	if (!OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult)) {
		// �������Ựʧ�ܣ���ί���Ƴ�ί���б�
		OnlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		// �㲥����Ựʧ����Ϣ���Զ������ϵͳί��
		SubsystemOnJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!OnlineSessionInterface.IsValid()) {	// ����Ự�ӿ���Ч
		SubsystemOnDestroySessionCompleteDelegate.Broadcast(false);	// �㲥�Ự����ʧ����Ϣ���Զ������ϵͳί��
		return;
	}

	// ����ί�о�����Ա�˺��Ƴ�ί���б�
	DestroySessionCompleteDelegateHandle = OnlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);	// ���ί�е��Ự�ӿڵ�ί���б�
	
	// ���ٻỰ
	if (!OnlineSessionInterface->DestroySession(NAME_GameSession)) {
		// ������ٻỰʧ�ܣ���ί���Ƴ�ί���б�
		OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		
		// �㲥�Ự����ʧ����Ϣ���Զ������ϵͳί��
		SubsystemOnDestroySessionCompleteDelegate.Broadcast(false);	
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionInterface) {
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	// �㲥�Ự�����ɹ���Ϣ���Զ������ϵͳί��
	SubsystemOnCreateSessionCompleteDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (OnlineSessionInterface) {
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	// ����������Ϊ�գ��㲥�����Ự����Ŀ������ʧ����Ϣ���Զ������ϵͳί��
	if (LastSessionSearch->SearchResults.Num() <= 0) {
		SubsystemOnFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	// �㲥�����Ự����ķǿ�����ͳɹ���Ϣ���Զ������ϵͳί��
	SubsystemOnFindSessionsCompleteDelegate.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (OnlineSessionInterface) {
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	// �㲥����Ự�Ľ�����Զ������ϵͳί��
	SubsystemOnJoinSessionCompleteDelegate.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionInterface) {
		OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	// ����ϴε��� CreateSession() ʱ��ǰ�ĻỰ��Ҫ�������ҸûỰ�Ѿ����ٳɹ�
	if (bCreateSessionOnDestroy && bWasSuccessful) {
		bCreateSessionOnDestroy = false;						// �ָ���ʼֵ
		CreateSession(LastNumPublicConnections, LastMatchType);	// �����»Ự
	}

	// �㲥���ٻỰ�Ľ�����Զ������ϵͳί��
	SubsystemOnDestroySessionCompleteDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
