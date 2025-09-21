// Fill out your copyright notice in the Description page of Project Settings.


#include "OBRuntimeLogCaptureSubsystem.h"

void UOBRuntimeLogCaptureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GLog)
	{
		LogOutputDevice = MakeUnique<FOBRuntimeLogOutputDevice>(this);
		GLog->AddOutputDevice(LogOutputDevice.Get());
	}

	UE_LOG(LogTemp, Log, TEXT("RuntimeLogCaptureSubsystem Initialized."));
}

void UOBRuntimeLogCaptureSubsystem::Deinitialize()
{
	if (GLog && LogOutputDevice.IsValid())
	{
		GLog->RemoveOutputDevice(LogOutputDevice.Get());
	}
	LogOutputDevice.Reset();

	Super::Deinitialize();
}

void UOBRuntimeLogCaptureSubsystem::GetCapturedLogs(TArray<FOBLogMessage>& OutLogs) const
{
	FScopeLock Lock(&LogMutex);
	OutLogs = CapturedLogs;
}

void UOBRuntimeLogCaptureSubsystem::CaptureLog(const TCHAR* Message, ELogVerbosity::Type Verbosity,
                                               const FName& Category)
{
	if (FString(Message).IsEmpty())
	{
		return;
	}

	FScopeLock Lock(&LogMutex);

	if (CapturedLogs.Num() >= MaxLogCount)
	{
		CapturedLogs.RemoveAt(0);
	}

	FOBLogMessage& NewLog = CapturedLogs.AddDefaulted_GetRef();
	NewLog.Message = Message;
	NewLog.Category = Category;
	NewLog.Verbosity = ConvertEngineVerbosity(Verbosity);
	NewLog.Timestamp = FDateTime::UtcNow();
}

EOBRuntimeLogVerbosity UOBRuntimeLogCaptureSubsystem::ConvertEngineVerbosity(ELogVerbosity::Type EngineVerbosity)
{
	switch (EngineVerbosity)
	{
	case ELogVerbosity::Fatal: return EOBRuntimeLogVerbosity::Fatal;
	case ELogVerbosity::Error: return EOBRuntimeLogVerbosity::Error;
	case ELogVerbosity::Warning: return EOBRuntimeLogVerbosity::Warning;
	case ELogVerbosity::Display: return EOBRuntimeLogVerbosity::Display;
	case ELogVerbosity::Log: return EOBRuntimeLogVerbosity::Log;
	case ELogVerbosity::Verbose: return EOBRuntimeLogVerbosity::Verbose;
	case ELogVerbosity::VeryVerbose: return EOBRuntimeLogVerbosity::VeryVerbose;
	default: return EOBRuntimeLogVerbosity::Log; // Mặc định an toàn
	}
}
