// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OBLogMessageObject.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OBRuntimeLogViewerSubsystem.generated.h"

/**
 * 
 */
UCLASS(config = Engine, defaultconfig)
class OBRUNTIMELOGVIEWER_API UOBRuntimeLogViewerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Show Log Viewer widget. */
	UFUNCTION(BlueprintCallable, Category = "Runtime Log Viewer")
	void ShowLogViewer();

	/** Hide Log Viewer widget. */
	UFUNCTION(BlueprintCallable, Category = "Runtime Log Viewer")
	void HideLogViewer();

	/** Toggle Log Viewer widget. */
	UFUNCTION(BlueprintCallable, Category = "Runtime Log Viewer")
	void ToggleLogViewer();

	UFUNCTION(BlueprintCallable, Category = "Runtime Log Viewer")
	TArray<UOBLogMessageObject*> GetFilteredLogObjects(bool bShowErrors, bool bShowWarnings, bool bShowLogs,
	                                                 const FString& FilterText);

	/**
	 * Convert an FDateTime object to FString with custom formatting.
	 * This function is static and pure, so it can be called from anywhere in Blueprint.
	 * Format: HH:MM:SS:ms, DD/MM/YYYY (time is assumed to be UTC)
	 * @param InDateTime UTC time to convert.
	 * @param UtcOffset
	 * @return Formatted string.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Runtime Log Viewer|Utilities")
	static FString FormatDateTimeToString(const FDateTime& InDateTime, const int32 UtcOffset = 7);

protected:


private:
	void OnWorldChanged(UWorld* World);
	bool bIsLogViewerVisible;
	FDelegateHandle OnPostLoadMapDelegateHandle;

	// Pointer to the log capture Subsystem.
	UPROPERTY()
	TObjectPtr<UOBRuntimeLogCaptureSubsystem> CaptureSubsystem;

	// Pointer to the created widget instance.
	UPROPERTY()
	TObjectPtr<UUserWidget> LogViewerWidgetInstance;

	// Console command object that can be called from the PC console.
	TUniquePtr<FAutoConsoleCommand> ToggleLogViewerCommand;

	UPROPERTY()
	TArray<TObjectPtr<UOBLogMessageObject>> LogMessageObjects;
};
