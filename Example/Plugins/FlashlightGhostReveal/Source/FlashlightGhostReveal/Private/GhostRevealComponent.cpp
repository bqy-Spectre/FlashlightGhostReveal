#include "GhostRevealComponent.h"
#include "FlashlightRevealDepthComponent.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
UGhostRevealComponent::UGhostRevealComponent() {
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/FlashlightGhostReveal/Materials/M_GhostReveal.M_GhostReveal"));
 RevealMaterial=Material.Object;
}
void UGhostRevealComponent::BeginPlay() {
 Super::BeginPlay();
 if(bBindOwnerMeshes) { TArray<UMeshComponent*> Meshes; GetOwner()->GetComponents(Meshes); for(auto* M:Meshes) BindTarget(M); }
 if(IsValid(DepthSource)) DepthSource->RegisterTarget(this);
}
bool UGhostRevealComponent::BindTarget(UMeshComponent* Mesh) {
 if(!IsValid(Mesh) || Mesh->GetWorld()!=GetWorld() || !IsValid(RevealMaterial) || Mesh->GetNumMaterials()==0) {
  UE_LOG(LogTemp,Warning,TEXT("GhostReveal: BindTarget requires a mesh with material slots and a reveal material in this World.")); return false;
 }
 if(BoundMeshes.Contains(Mesh)) return true;
 FRevealMeshBinding B; B.Mesh=Mesh; B.CastShadow=Mesh->CastShadow; B.CustomDepth=Mesh->bRenderCustomDepth;
 for(int32 Slot=0;Slot<Mesh->GetNumMaterials();++Slot) {
  B.Originals.Add(Mesh->GetMaterial(Slot));
  auto* MID=UMaterialInstanceDynamic::Create(RevealMaterial,this);
  MID->SetScalarParameterValue(TEXT("RevealDepthValid"),0);
  B.Instances.Add(MID); Mesh->SetMaterial(Slot,MID);
 }
 Mesh->SetCastShadow(false); Mesh->SetRenderCustomDepth(false);
 Bindings.Add(MoveTemp(B)); BoundMeshes.Add(Mesh); RefreshStyle(); return true;
}
void UGhostRevealComponent::UnbindTarget(UMeshComponent* Mesh) {
 for(int32 I=Bindings.Num()-1;I>=0;--I) if(Bindings[I].Mesh==Mesh) {
  const auto& B=Bindings[I];
  if(IsValid(Mesh)) {
   for(int32 S=0;S<B.Originals.Num() && S<Mesh->GetNumMaterials();++S) Mesh->SetMaterial(S,B.Originals[S]);
   Mesh->SetCastShadow(B.CastShadow); Mesh->SetRenderCustomDepth(B.CustomDepth);
  }
  Bindings.RemoveAt(I);
 }
 BoundMeshes.Remove(Mesh);
}
void UGhostRevealComponent::UnbindAll() { auto Meshes=BoundMeshes; for(const auto& M:Meshes) UnbindTarget(M.Get()); }
void UGhostRevealComponent::RefreshStyle() {
 const float Value=FMath::IsFinite(Strength)?FMath::Clamp(Strength,0.f,1.f):0.f;
 for(const auto& B:Bindings) for(const auto& MID:B.Instances) if(MID) MID->SetScalarParameterValue(TEXT("RevealStrength"),Value);
}
void UGhostRevealComponent::InvalidateRevealDepth() {
 for(const auto& B:Bindings) for(const auto& MID:B.Instances) if(MID) MID->SetScalarParameterValue(TEXT("RevealDepthValid"),0);
}
void UGhostRevealComponent::EndPlay(const EEndPlayReason::Type R) {
 if(IsValid(DepthSource)) DepthSource->UnregisterTarget(this);
 UnbindAll(); Super::EndPlay(R);
}
void UGhostRevealComponent::ApplyRevealDepth(const FRevealDepthFrame& F,UTexture* Texture)
{
    const auto& S=F.Light;
    for(const auto& Binding:Bindings) for(const auto& MID:Binding.Instances) if(MID)
    {
        MID->SetScalarParameterValue(TEXT("RevealDepthValid"),0);
        MID->SetTextureParameterValue(TEXT("RevealLightDepth"),Texture);
        MID->SetVectorParameterValue(TEXT("RevealPositionEnabled"),FLinearColor(S.Position.X,S.Position.Y,S.Position.Z,1));
        MID->SetVectorParameterValue(TEXT("RevealDirectionCosOuter"),FLinearColor(S.Direction.X,S.Direction.Y,S.Direction.Z,FMath::Cos(FMath::DegreesToRadians(S.OuterHalfAngle))));
        MID->SetVectorParameterValue(TEXT("RevealConeRange"),FLinearColor(FMath::Cos(FMath::DegreesToRadians(S.InnerHalfAngle)),S.RangeStart,S.RangeEnd,static_cast<float>(F.AngularMode)));
        MID->SetVectorParameterValue(TEXT("RevealDepthRightTan"),FLinearColor(F.Right.X,F.Right.Y,F.Right.Z,F.TanHalfFOV));
        MID->SetVectorParameterValue(TEXT("RevealDepthUpNear"),FLinearColor(F.Up.X,F.Up.Y,F.Up.Z,F.NearClip));
        MID->SetVectorParameterValue(TEXT("RevealDepthControl"),FLinearColor(1.f/F.Resolution,F.Bias,static_cast<float>(F.FilterSamples),0));
        MID->SetScalarParameterValue(TEXT("RevealDepthValid"),1);
    }
}
