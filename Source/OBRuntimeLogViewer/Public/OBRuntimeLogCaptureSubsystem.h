// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OBRuntimeLogOutputDevice.h"
#include "Logging/LogVerbosity.h"
#include "OBRuntimeLogCaptureSubsystem.generated.h"

/**
 * Our enum, registered with the Reflection System for Blueprint safety.
 * It reflects the important log levels that we want to filter.
 */
UENUM(BlueprintType)
enum class EOBRuntimeLogVerbosity : uint8
{
	Fatal,
	Error,
	Warning,
	Display,
	Log,
	Verbose,
	VeryVerbose
};

// Struct to store information of a log line.
USTRUCT(BlueprintType)
struct FOBLogMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Log")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Log")
	FName Category;

	UPROPERTY(BlueprintReadOnly, Category = "Log")
	EOBRuntimeLogVerbosity Verbosity;

	// Add a timestamp for tracking purposes
	UPROPERTY(BlueprintReadOnly, Category = "Log")
	FDateTime Timestamp;

	FOBLogMessage() : Verbosity(EOBRuntimeLogVerbosity::Log)
	{
	}
};

/**
 * 
 */
UCLASS()
class OBRUNTIMELOGVIEWER_API UOBRuntimeLogCaptureSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// Allow FRuntimeLogOutputDevice class to access the private CaptureLog function.
	friend class FOBRuntimeLogOutputDevice;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Get a copy of the captured log list.
	 * This function is thread-safe.
	 * @param OutLogs - Array that will be filled with log data.
	 */
	void GetCapturedLogs(TArray<FOBLogMessage>& OutLogs) const;

private:
	/**
	 * Internal function to add a new log to the storage array.
	 * Must be called within a critical section (mutex lock).
	 * @param Message - Log content.
	 * @param Verbosity - Log level (Error, Warning, etc.).
	 * @param Category - Log category.
	 */
	void CaptureLog(const TCHAR* Message, ELogVerbosity::Type Verbosity, const FName& Category);

	static EOBRuntimeLogVerbosity ConvertEngineVerbosity(ELogVerbosity::Type EngineVerbosity);

	// Custom output device to listen to logs from the engine.
	TUniquePtr<FOBRuntimeLogOutputDevice> LogOutputDevice;

	// List of captured logs.
	TArray<FOBLogMessage> CapturedLogs;

	// Mutex to ensure thread-safe read/write operations on the CapturedLogs array.
	// Using mutable to allow locking it in a const function (GetCapturedLogs).
	mutable FCriticalSection LogMutex;

	// Maximum log count limit to avoid excessive memory usage.
	const int32 MaxLogCount = 1000;
};