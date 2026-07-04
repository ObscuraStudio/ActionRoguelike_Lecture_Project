// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


class FRocketTraceHelper
{
public:
    // Static method to determine which actor the material is dropped onto
    static AActor* GetActorUnderMousePosition(const FVector2D& ScreenPosition);
};
