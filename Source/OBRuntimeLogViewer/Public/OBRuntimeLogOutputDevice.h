// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UOBRuntimeLogCaptureSubsystem;

/**
 * 
 */
class OBRUNTIMELOGVIEWER_API FOBRuntimeLogOutputDevice : public FOutputDevice
{
public:
	// Constructor nhận vào subsystem sở hữu nó.
	FOBRuntimeLogOutputDevice(UOBRuntimeLogCaptureSubsystem* InOwner);
	virtual ~FOBRuntimeLogOutputDevice() override;

protected:
	// Ghi đè hàm Serialize để nhận các thông điệp log từ engine.
	// Hàm này có thể được gọi từ bất kỳ thread nào.
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;

private:
	// Con trỏ yếu đến subsystem sở hữu để tránh circular dependency gây memory leak.
	TWeakObjectPtr<UOBRuntimeLogCaptureSubsystem> OwnerSubsystem;
};