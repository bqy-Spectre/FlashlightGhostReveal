#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GhostRevealComponent.generated.h"
class UMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UFlashlightRevealDepthComponent;
class UTexture;
struct FRevealDepthFrame;
USTRUCT()
struct FRevealMeshBinding {
 GENERATED_BODY()
 UPROPERTY() TObjectPtr<UMeshComponent> Mesh;
 UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> Originals;
 UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> Instances;
 bool CastShadow=false, CustomDepth=false;
};
UCLASS(ClassGroup=(Rendering),meta=(BlueprintSpawnableComponent))
class FLASHLIGHTGHOSTREVEAL_API UGhostRevealComponent : public UActorComponent {
 GENERATED_BODY()
public:
 UGhostRevealComponent();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Ghost Reveal") TObjectPtr<UMaterialInterface> RevealMaterial;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Ghost Reveal") TObjectPtr<UFlashlightRevealDepthComponent> DepthSource;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Ghost Reveal") float Strength=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Ghost Reveal") bool bBindOwnerMeshes=true;
 UFUNCTION(BlueprintCallable,Category="Ghost Reveal") bool BindTarget(UMeshComponent* Mesh);
 UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void UnbindTarget(UMeshComponent* Mesh);
 UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void UnbindAll();
 UFUNCTION(BlueprintCallable,Category="Ghost Reveal") void RefreshStyle();
 const TArray<TObjectPtr<UMeshComponent>>& GetBoundMeshes() const { return BoundMeshes; }
 void InvalidateRevealDepth();
 void ApplyRevealDepth(const FRevealDepthFrame& Frame,UTexture* Texture);
protected:
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
 UPROPERTY(Transient) TArray<FRevealMeshBinding> Bindings;
 UPROPERTY(Transient) TArray<TObjectPtr<UMeshComponent>> BoundMeshes;
};
