#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightRevealComponent.generated.h"

class USpotLightComponent;


UENUM(BlueprintType)
enum class ERevealAngularMode : uint8 { SmoothCone, UEAngularFalloff };

USTRUCT(BlueprintType)
struct FLASHLIGHTGHOSTREVEAL_API FRevealLightSnapshot
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly,Category="Ghost Reveal") FVector Position = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly,Category="Ghost Reveal") FVector Direction = FVector::ForwardVector;
    UPROPERTY(BlueprintReadOnly,Category="Ghost Reveal") float InnerHalfAngle = 15;
    UPROPERTY(BlueprintReadOnly,Category="Ghost Reveal") float OuterHalfAngle = 25;
    UPROPERTY(BlueprintReadOnly,Category="Ghost Reveal") float RangeStart = 720;
    UPROPERTY(BlueprintReadOnly,Category="Ghost Reveal") float RangeEnd = 800;
    UPROPERTY(BlueprintReadOnly,Category="Ghost Reveal") bool Enabled = false;
    uint64 FrameId = 0;
    bool IsValid() const;
    float Coverage(const FVector& Surface, ERevealAngularMode Mode) const;
};

UCLASS(ClassGroup=(Reveal), meta=(BlueprintSpawnableComponent))
class FLASHLIGHTGHOSTREVEAL_API UFlashlightRevealComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFlashlightRevealComponent();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reveal") TObjectPtr<USpotLightComponent> Light;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reveal") bool bRevealEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reveal") ERevealAngularMode AngularMode = ERevealAngularMode::SmoothCone;
    UFUNCTION(BlueprintCallable,Category="Ghost Reveal") bool Initialize(USpotLightComponent* InLight);
    UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void SetRevealEnabled(bool bEnabled);
    UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void Publish();
    const FRevealLightSnapshot& GetSnapshot() const { return Snapshot; }
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Delta, ELevelTick Tick, FActorComponentTickFunction* Function) override;
private:
    FRevealLightSnapshot Snapshot;
    TWeakObjectPtr<AActor> PrerequisiteOwner;
};
