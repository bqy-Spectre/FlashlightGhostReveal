#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RenderCommandFence.h"
#include "FlashlightRevealComponent.h"
#include "FlashlightRevealDepthComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UGhostRevealComponent;
class UPrimitiveComponent;
class USpotLightComponent;

UENUM(BlueprintType)
enum class ERevealDepthHistoryPolicy : uint8
{
    PreserveMainView,
    ResetMainViewWhenDepthInvalidates
};

// One immutable light/projection identity travels with one completed depth image.
struct FRevealDepthFrame
{
    FRevealLightSnapshot Light;
    FVector Right=FVector::RightVector,Up=FVector::UpVector;
    float TanHalfFOV=1,NearClip=1,Bias=0.5f;
    ERevealAngularMode AngularMode=ERevealAngularMode::SmoothCone;
    int32 Resolution=2048,FilterSamples=1;
    uint32 Generation=0;
    uint64 CaptureRequestFrame=0;
};

UCLASS(ClassGroup=(Reveal),meta=(BlueprintSpawnableComponent))
class FLASHLIGHTGHOSTREVEAL_API UFlashlightRevealDepthComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFlashlightRevealDepthComponent();
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RevealDepth") TObjectPtr<UFlashlightRevealComponent> Source;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RevealDepth") int32 Resolution=2048;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RevealDepth") int32 FilterSamples=1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RevealDepth") float BiasCm=.5f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RevealDepth") float NearClipCm=1;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="RevealDepth") ERevealDepthHistoryPolicy HistoryPolicy=ERevealDepthHistoryPolicy::PreserveMainView;
    UPROPERTY(EditAnywhere,Category="RevealDepth") TArray<TObjectPtr<UPrimitiveComponent>> AdditionalHiddenComponents;
    UFUNCTION(BlueprintCallable,Category="Ghost Reveal") bool Initialize(USpotLightComponent* Light);
    UFUNCTION(BlueprintCallable,Category="Ghost Reveal") bool RegisterTarget(UGhostRevealComponent* Target);
    UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void UnregisterTarget(UGhostRevealComponent* Target);
    UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void SetRevealEnabled(bool bEnabled);
    const FRevealDepthFrame& GetPublishedFrame() const { return Published; }
    uint64 GetPublishedAtFrame() const { return PublishedAtFrame; }
    bool IsDepthValid() const { return PublishedIndex!=INDEX_NONE; }
    uint32 GetGeneration() const { return Generation; }
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Delta,ELevelTick Tick,FActorComponentTickFunction* Function) override;
private:
    void InvalidateTargets();
    bool PrepareResources();
    void PublishPending();
    void NotifyHistoryInvalidation();
    UPROPERTY(Transient) TObjectPtr<USceneCaptureComponent2D> Capture;
    UPROPERTY(Transient) TArray<TObjectPtr<UTextureRenderTarget2D>> Buffers;
    UPROPERTY(Transient) TArray<TObjectPtr<UGhostRevealComponent>> Targets;
    TWeakObjectPtr<USpotLightComponent> LastLight;
    FRenderCommandFence CaptureFence;
    FRevealDepthFrame Pending,Published;
    int32 PendingIndex=0,PublishedIndex=INDEX_NONE,AllocatedResolution=0;
    uint32 Generation=0;
    uint64 PublishedAtFrame=0;
    bool LastEnabled=false;
};
