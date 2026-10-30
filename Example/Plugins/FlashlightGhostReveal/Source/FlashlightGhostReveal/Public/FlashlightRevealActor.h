#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlashlightRevealActor.generated.h"
class USpotLightComponent;
class UFlashlightRevealComponent;
class UFlashlightRevealDepthComponent;
UCLASS(Blueprintable)
class FLASHLIGHTGHOSTREVEAL_API AFlashlightRevealActor : public AActor {
 GENERATED_BODY()
public:
 AFlashlightRevealActor();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Ghost Reveal") TObjectPtr<USpotLightComponent> Light;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Ghost Reveal") TObjectPtr<UFlashlightRevealComponent> Source;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Ghost Reveal") TObjectPtr<UFlashlightRevealDepthComponent> Depth;
 UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void SetFlashlightEnabled(bool Enabled);
};
