// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OBRuntimeLogViewerSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "OB Runtime Log Viewer"))
class OBRUNTIMELOGVIEWER_API UOBRuntimeLogViewerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	/** Widget Blueprint class that will be used to display Log Viewer. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowedClasses = "/Script/UMG.UserWidget"))
	FSoftClassPath LogViewerWidgetClass = FSoftClassPath(TEXT("/OBRuntimeLogViewer/WBP_LogViewer.WBP_LogViewer_C"));

	/** Automatically show Log Viewer when the game starts (useful for QA builds). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Startup")
	bool bShowLogViewerOnStartup = true;

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override { return TEXT("Plugins");}
};
