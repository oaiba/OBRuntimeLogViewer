// Fill out your copyright notice in the Description page of Project Settings.

#include "OBRuntimeLogViewerSubsystem.h"
#include "OBRuntimeLogViewerSettings.h"
#include "OBRuntimeLogCaptureSubsystem.h"
#include "Blueprint/UserWidget.h"

void UOBRuntimeLogViewerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    bIsLogViewerVisible = false; 

    OnPostLoadMapDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UOBRuntimeLogViewerSubsystem::OnWorldChanged);

    const UOBRuntimeLogViewerSettings* Settings = GetDefault<UOBRuntimeLogViewerSettings>();
    check(Settings); 

    CaptureSubsystem = GetGameInstance()->GetSubsystem<UOBRuntimeLogCaptureSubsystem>();
    check(CaptureSubsystem != nullptr);

    if (Settings->bShowLogViewerOnStartup)
    {
        FTimerHandle DummyHandle;
        GetWorld()->GetTimerManager().SetTimer(DummyHandle, [this]()
        {
            ShowLogViewer();
        }, 1.0f, false);
    }
    
    ToggleLogViewerCommand = MakeUnique<FAutoConsoleCommand>(
        TEXT("LogViewer.Toggle"),
        TEXT("Toggles the runtime log viewer UI."),
        FConsoleCommandDelegate::CreateUObject(this, &UOBRuntimeLogViewerSubsystem::ToggleLogViewer)
    );

    UE_LOG(LogTemp, Log, TEXT("RuntimeLogViewerSubsystem Initialized."));
}

void UOBRuntimeLogViewerSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(OnPostLoadMapDelegateHandle);

    HideLogViewer();
    ToggleLogViewerCommand.Reset();

    HideLogViewer();
    Super::Deinitialize();
}

void UOBRuntimeLogViewerSubsystem::ShowLogViewer()
{
    if (!LogViewerWidgetInstance)
    {
        const UOBRuntimeLogViewerSettings* Settings = GetDefault<UOBRuntimeLogViewerSettings>();
        const TSubclassOf<UUserWidget> WidgetClass = Settings->LogViewerWidgetClass.TryLoadClass<UUserWidget>();
        if (WidgetClass == nullptr)
        {
            UE_LOG(LogTemp, Error, TEXT("LogViewerWidgetClass is not set in Project Settings -> Plugins -> Runtime Log Viewer!"));
            return;
        }

        LogViewerWidgetInstance = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
        if (!LogViewerWidgetInstance)
        {
            return;
        }
    }

    if (!LogViewerWidgetInstance->IsInViewport())
    {
        LogViewerWidgetInstance->AddToViewport(100);
    }
	
    bIsLogViewerVisible = true;
}

void UOBRuntimeLogViewerSubsystem::HideLogViewer()
{
    if (LogViewerWidgetInstance != nullptr)
    {
        LogViewerWidgetInstance->RemoveFromParent();
        LogViewerWidgetInstance = nullptr;
    }

    bIsLogViewerVisible = false;

}

void UOBRuntimeLogViewerSubsystem::ToggleLogViewer()
{
    if (bIsLogViewerVisible)
    {
        HideLogViewer();
    }
    else
    {
        ShowLogViewer();
    }
}

TArray<UOBLogMessageObject*> UOBRuntimeLogViewerSubsystem::GetFilteredLogObjects(bool bShowErrors, bool bShowWarnings,
    bool bShowLogs, const FString& FilterText)
{
    TArray<FOBLogMessage> AllLogs;
    if (CaptureSubsystem)
    {
        CaptureSubsystem->GetCapturedLogs(AllLogs);
    }

    LogMessageObjects.Empty();
    TArray<UOBLogMessageObject*> FilteredObjects;

    if (!bShowErrors && !bShowWarnings && !bShowLogs && FilterText.IsEmpty())
    {
        return FilteredObjects; 
    }

    for (const FOBLogMessage& Log : AllLogs)
    {
        bool bVerbosityMatch = false;
        if (bShowErrors && Log.Verbosity <= EOBRuntimeLogVerbosity::Error) bVerbosityMatch = true;
        if (bShowWarnings && Log.Verbosity == EOBRuntimeLogVerbosity::Warning) bVerbosityMatch = true;
        if (bShowLogs && (Log.Verbosity == EOBRuntimeLogVerbosity::Log || Log.Verbosity == EOBRuntimeLogVerbosity::Display)) bVerbosityMatch = true;

        if (bVerbosityMatch)
        {
            if (FilterText.IsEmpty() || Log.Message.Contains(FilterText))
            {
                UOBLogMessageObject* NewLogObject = NewObject<UOBLogMessageObject>(this);
                NewLogObject->LogData = Log;
                
                LogMessageObjects.Add(NewLogObject); 
                FilteredObjects.Add(NewLogObject); 
            }
        }
    }

    return FilteredObjects;
}

FString UOBRuntimeLogViewerSubsystem::FormatDateTimeToString(const FDateTime& InDateTime, const int32 UtcOffset)
{
    // Notes: FDateTime captured with FDateTime::UtcNow() is already in UTC.
    // We apply the desired offset before formatting it to a string.
    // By default, a +7 hour offset is applied.

    // Calculate the offset as an FTimespan
    const FTimespan TimezoneOffset = FTimespan::FromHours(UtcOffset);

    // Apply the offset to the UTC DateTime
    const FDateTime LocalizedDateTime = InDateTime + TimezoneOffset;

    // Format codes:
    // %h - Hour (00-23)
    // %m - Minute (00-59)
    // %s - Second (00-59)
    // %l - Milliseconds (000-999)
    // %d - Day (01-31)
    // %m - Month (01-12)
    // %Y - Year (4-digit)
    const FString FormatString = TEXT("%h:%m:%s:%l, %d/%m/%Y");

    return LocalizedDateTime.ToString(*FormatString);
}

void UOBRuntimeLogViewerSubsystem::OnWorldChanged(UWorld* World)
{
    if (World && World->IsGameWorld())
    {
        LogViewerWidgetInstance = nullptr;

        if (bIsLogViewerVisible)
        {
            FTimerHandle DummyHandle;
            World->GetTimerManager().SetTimer(DummyHandle, this, &UOBRuntimeLogViewerSubsystem::ShowLogViewer, 0.1f, false);
        }
    }
}