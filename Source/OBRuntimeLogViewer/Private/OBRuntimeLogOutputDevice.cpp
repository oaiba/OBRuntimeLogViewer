// Fill out your copyright notice in the Description page of Project Settings.


#include "OBRuntimeLogOutputDevice.h"
#include "OBRuntimeLogCaptureSubsystem.h"

FOBRuntimeLogOutputDevice::FOBRuntimeLogOutputDevice(UOBRuntimeLogCaptureSubsystem* InOwner)
{
	OwnerSubsystem = InOwner;
}

FOBRuntimeLogOutputDevice::~FOBRuntimeLogOutputDevice() = default;

void FOBRuntimeLogOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	if (OwnerSubsystem.IsValid())
	{
		OwnerSubsystem->CaptureLog(V, Verbosity, Category);
	}
}
