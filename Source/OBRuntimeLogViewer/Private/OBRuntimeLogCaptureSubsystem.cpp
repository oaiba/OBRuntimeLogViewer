// Fill out your copyright notice in the Description page of Project Settings.


#include "OBRuntimeLogCaptureSubsystem.h"
#include "Misc/FileHelper.h" // NEW: Cần cho việc ghi file
#include "HAL/PlatformFileManager.h" // NEW: Cần cho việc quản lý file
#include "Misc/Paths.h" // NEW: Cần để lấy các đường dẫn chuẩn
#include "HAL/IConsoleManager.h" // NEW: Cần cho console command

void UOBRuntimeLogCaptureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GLog)
	{
		LogOutputDevice = MakeUnique<FOBRuntimeLogOutputDevice>(this);
		GLog->AddOutputDevice(LogOutputDevice.Get());
	}

	SaveLogsCommand = MakeUnique<FAutoConsoleCommand>(
		TEXT("Log.SaveToFile"),
		TEXT("Saves the runtime captured logs to a file in the project's Saved/Logs directory."),
		FConsoleCommandDelegate::CreateUObject(this, &UOBRuntimeLogCaptureSubsystem::SaveLogsToFile_FromConsole)
	);

	UE_LOG(LogTemp, Log, TEXT("RuntimeLogCaptureSubsystem Initialized."));
}

void UOBRuntimeLogCaptureSubsystem::Deinitialize()
{
	// NEW: Tự động lưu log khi subsystem bị hủy (khi game thoát)
	UE_LOG(LogTemp, Log, TEXT("RuntimeLogCaptureSubsystem Deinitializing. Attempting to save logs..."));
	SaveLogsToFile_FromConsole();

	if (GLog && LogOutputDevice.IsValid())
	{
		GLog->RemoveOutputDevice(LogOutputDevice.Get());
	}
	LogOutputDevice.Reset();

	// Hủy đăng ký command
	SaveLogsCommand.Reset();

	Super::Deinitialize();
}

void UOBRuntimeLogCaptureSubsystem::GetCapturedLogs(TArray<FOBLogMessage>& OutLogs) const
{
	FScopeLock Lock(&LogMutex);
	OutLogs = CapturedLogs;
}

void UOBRuntimeLogCaptureSubsystem::SaveLogsToFile_FromConsole()
{
	// Gọi hàm gốc và bỏ qua giá trị trả về của nó.
	// Console command không cần biết đường dẫn file, vì nó đã được in ra log.
	SaveLogsToFile(TEXT(""));
}

FString UOBRuntimeLogCaptureSubsystem::SaveLogsToFile(const FString& OptionalFilename)
{
	TArray<FOBLogMessage> LogsToSave;
	GetCapturedLogs(LogsToSave);

	if (LogsToSave.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveLogsToFile: No logs captured, nothing to save."));
		return FString();
	}

	// Tạo tên file nếu không được cung cấp
	FString Filename = OptionalFilename;
	if (Filename.IsEmpty())
	{
		Filename = FString::Printf(TEXT("%s-RuntimeLog-%s.txt"),
								   FApp::GetProjectName(),
								   *FDateTime::Now().ToString(TEXT("%Y.%m.%d-%H.%M.%S")));
	}
	else if (!Filename.EndsWith(TEXT(".txt")))
	{
		Filename.Append(TEXT(".txt"));
	}

	// Lấy đường dẫn thư mục Logs chuẩn của project
	const FString SaveDirectory = FPaths::ProjectLogDir();
	const FString FullPath = SaveDirectory + Filename;

	// Chuẩn bị nội dung để ghi
	TArray<FString> LinesToSave;
	LinesToSave.Reserve(LogsToSave.Num());

	for (const FOBLogMessage& Log : LogsToSave)
	{
		// Định dạng mỗi dòng log
		// [Timestamp][Category][Verbosity] Message
		FString FormattedLine = FString::Printf(TEXT("[%s][%s][%s] %s"),
												*Log.Timestamp.ToString(TEXT("%Y.%m.%d-%H:%M:%S:%l")),
												*Log.Category.ToString(),
												*VerbosityToString(Log.Verbosity),
												*Log.Message
		);
		LinesToSave.Add(FormattedLine);
	}

	// Ghi mảng chuỗi ra file. FFileHelper sẽ xử lý việc tạo file và ghi nội dung.
	if (FFileHelper::SaveStringArrayToFile(LinesToSave, *FullPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully saved %d logs to: %s"), LogsToSave.Num(), *FullPath);
		return FullPath;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save logs to: %s"), *FullPath);
		return FString();
	}
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

FString UOBRuntimeLogCaptureSubsystem::VerbosityToString(EOBRuntimeLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EOBRuntimeLogVerbosity::Fatal: return TEXT("Fatal");
	case EOBRuntimeLogVerbosity::Error: return TEXT("Error");
	case EOBRuntimeLogVerbosity::Warning: return TEXT("Warning");
	case EOBRuntimeLogVerbosity::Display: return TEXT("Display");
	case EOBRuntimeLogVerbosity::Log: return TEXT("Log");
	case EOBRuntimeLogVerbosity::Verbose: return TEXT("Verbose");
	case EOBRuntimeLogVerbosity::VeryVerbose: return TEXT("VeryVerbose");
	default: return TEXT("Unknown");
	}
}
