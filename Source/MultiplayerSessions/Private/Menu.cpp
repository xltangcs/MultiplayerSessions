// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton) {
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if (JoinButton) {
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();	// 调用父类的 NativeDestruct() 函数
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(	// 添加调试信息到屏幕上
				-1,				// 使用 -1 不会覆盖前面的调试信息
				15.f,			// 调试信息的显示时间
				FColor::Yellow,	// 字体颜色：黄色
				FString::Printf(TEXT("Session created Successfully!"))	// 打印会话创建成功消息
			);
		}

		// 会话创建成功后传送至关卡 Lobby
		UWorld* World = GetWorld();
		if (World) {
			// Uworld->ServerTravel：https://docs.unrealengine.com/5.0/en-US/API/Runtime/Engine/Engine/UWorld/ServerTravel/
			World->ServerTravel(PathToLobby);	// 作为监听服务器打开 Lobby 关卡
		}
	}
	else {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(	// 添加调试信息到屏幕上
				-1,				// 使用 -1 不会覆盖前面的调试信息
				15.f,			// 调试信息的显示时间
				FColor::Yellow,	// 字体颜色：黄色
				FString::Printf(TEXT("Failed to create session!"))	// 打印会话创建成功消息
			);
		}

		HostButton->SetIsEnabled(true);
		
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr) {
		return;
	}

	// 遍历搜索结果并加入第一个匹配类型相同的会话（以后可以进行改进）
	for (auto Result : SearchResults) {
		FString SettingsValue;	// 保存会话匹配类型
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);	// 获取会话匹配类型
		if (SettingsValue == MatchType) {						// 如果匹配类型相同
			MultiplayerSessionsSubsystem->JoinSession(Result);	// 调用子系统的加入会话函数
			return;
		}
	}

	if(!bWasSuccessful || SearchResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}

}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();						// 获取当前的在线子系统指针
	if (OnlineSubsystem) {																// 如果当前在线子系统有效
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();	// 获取会话接口智能指针
		if (SessionInterface.IsValid()) {	// 如果获取会话接口成功
			FString Address;				// 保存会话创建源地址
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);		// 获取会话创建源地址

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();	// 获取玩家控制器
			if (PlayerController) {
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);	// 客户端传送至关卡 “Lobby”
			}
		}
	}

	if(Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
	
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	//if (GEngine) {
	//	GEngine->AddOnScreenDebugMessage(	// 添加调试信息到屏幕上
	//		-1,				// 使用 -1 不会覆盖前面的调试信息
	//		15.f,			// 调试信息的显示时间
	//		FColor::Yellow,	// 字体颜色：黄色
	//		FString::Printf(TEXT("Host Button Clicked!"))	// 打印点击事件消息
	//	);
	//}

	HostButton->SetIsEnabled(false);
	
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);	// 创建游戏会话
		//UWorld* World = GetWorld();
		//if (World) {
		//	// Uworld->ServerTravel：https://docs.unrealengine.com/5.0/en-US/API/Runtime/Engine/Engine/UWorld/ServerTravel/
		//	World->ServerTravel(FString("/Game/ThirdPerson/Maps/Lobby?listen"));	// 作为监听服务器打开 Lobby 关卡
		//}
	}
}

void UMenu::JoinButtonClicked()
{
	//if (GEngine) {
	//	GEngine->AddOnScreenDebugMessage(	// 添加调试信息到屏幕上
	//		-1,				// 使用 -1 不会覆盖前面的调试信息
	//		15.f,			// 调试信息的显示时间
	//		FColor::Blue,	// 字体颜色：黄色
	//		FString::Printf(TEXT("Join Button Clicked!"))	// 打印点击事件消息
	//	);
	//}
	JoinButton->SetIsEnabled(false);
	
	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}


void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World) {
		APlayerController* PlayerController = World->GetFirstPlayerController();	// 获取玩家控制器指针
		if (PlayerController) {
			FInputModeGameOnly InputModeData;				// 用于设置可以控制游戏的输入模式
			PlayerController->SetInputMode(InputModeData);	// 设置玩家控制器的输入模式
			PlayerController->SetShowMouseCursor(false);	// 隐藏鼠标光标
		}
	}
}

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);

	//bIsFocusable = true;
	SetIsFocusable(true);
	
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeSettings;
			InputModeSettings.SetWidgetToFocus(TakeWidget());
			InputModeSettings.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PlayerController->SetInputMode(InputModeSettings);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance) {
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();	
	}

	if (MultiplayerSessionsSubsystem) {
		MultiplayerSessionsSubsystem->SubsystemOnCreateSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->SubsystemOnFindSessionsCompleteDelegate.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->SubsystemOnJoinSessionCompleteDelegate.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->SubsystemOnDestroySessionCompleteDelegate.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->SubsystemOnStartSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnStartSession);
	}

}
