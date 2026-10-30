#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "FlashlightExample.generated.h"
class UCameraComponent;
class AFlashlightRevealActor;
class USkeletalMeshComponent;
class UGhostRevealComponent;

UCLASS(Blueprintable)
class GHOSTREVEALEXAMPLE_API AFlashlightExamplePlayer : public ACharacter {
 GENERATED_BODY()
public:
 AFlashlightExamplePlayer();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UCameraComponent> Camera;
 UPROPERTY(BlueprintReadOnly) TObjectPtr<AFlashlightRevealActor> Flashlight;
protected:
 virtual void BeginPlay() override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
private:
 bool bLightEnabled=true;
 void Forward(float Value);
 void Right(float Value);
 void Turn(float Value);
 void Look(float Value);
 void ToggleLight();
 void ReleaseMouse();
 void CaptureMouse();
};
UCLASS(Blueprintable)
class GHOSTREVEALEXAMPLE_API AFlashlightExampleGhost : public AActor {
 GENERATED_BODY()
public:
 AFlashlightExampleGhost();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<USkeletalMeshComponent> Mesh;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UGhostRevealComponent> Reveal;
protected:
 virtual void BeginPlay() override;
};
UCLASS()
class AFlashlightExampleHUD : public AHUD {
 GENERATED_BODY()
 virtual void DrawHUD() override;
};
UCLASS()
class AFlashlightExampleGameMode : public AGameModeBase {
 GENERATED_BODY()
public:
 AFlashlightExampleGameMode();
};
