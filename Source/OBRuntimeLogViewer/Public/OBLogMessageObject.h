// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OBRuntimeLogCaptureSubsystem.h"
#include "UObject/Object.h"
#include "OBLogMessageObject.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class OBRUNTIMELOGVIEWER_API UOBLogMessageObject : public UObject
{
	GENERATED_BODY()

public:
	// Dữ liệu log thực tế được chứa bên trong.
	UPROPERTY(BlueprintReadOnly, Category="Log")
	FOBLogMessage LogData;
};
