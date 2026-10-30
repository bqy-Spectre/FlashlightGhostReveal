#include "FlashlightRevealComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
bool FRevealLightSnapshot::IsValid() const
{
    return !Position.ContainsNaN() && !Direction.ContainsNaN() && Direction.IsNormalized()
        && FMath::IsFinite(InnerHalfAngle) && FMath::IsFinite(OuterHalfAngle)
        && FMath::IsFinite(RangeStart) && FMath::IsFinite(RangeEnd)
        && InnerHalfAngle >= 0 && InnerHalfAngle < OuterHalfAngle && OuterHalfAngle < 90
        && RangeStart >= 0 && RangeStart < RangeEnd;
}

float FRevealLightSnapshot::Coverage(const FVector& Surface, ERevealAngularMode Mode) const
{
    if (!Enabled || !IsValid() || Surface.ContainsNaN()) return 0;
    const FVector Q = Surface - Position;
    const double R = Q.Length();
    if (R < 0.1) return 0;
    const double OuterCos = FMath::Cos(FMath::DegreesToRadians(OuterHalfAngle));
    const double InnerCos = FMath::Cos(FMath::DegreesToRadians(InnerHalfAngle));
    // Match MF_RevealConeCoverage when float half-angle cosines collapse.
    if (InnerCos <= OuterCos) return 0;
    const double T = FMath::Clamp((FVector::DotProduct(Q / R, Direction)-OuterCos)/(InnerCos-OuterCos),0.0,1.0);
    const double C = Mode == ERevealAngularMode::SmoothCone ? T*T*(3-2*T) : T*T;
    const double V = FMath::Clamp((R-RangeStart)/(RangeEnd-RangeStart),0.0,1.0);
    return C*(1-V*V*(3-2*V));
}


UFlashlightRevealComponent::UFlashlightRevealComponent() {
 PrimaryComponentTick.bCanEverTick=true;
 PrimaryComponentTick.TickGroup=TG_PostUpdateWork;
}
bool UFlashlightRevealComponent::Initialize(USpotLightComponent* InLight) {
 Snapshot.Enabled=false;
 if(PrerequisiteOwner.IsValid()) RemoveTickPrerequisiteActor(PrerequisiteOwner.Get());
 Light=InLight;
 if(!IsValid(Light) || Light->GetWorld()!=GetWorld()) {
  UE_LOG(LogTemp,Error,TEXT("GhostReveal: Initialize requires a Spot Light in the same World."));
  return false;
 }
 PrerequisiteOwner=Light->GetOwner();
 AddTickPrerequisiteActor(GetOwner());
 if(PrerequisiteOwner.IsValid()) AddTickPrerequisiteActor(PrerequisiteOwner.Get());
 Publish(); return true;
}
void UFlashlightRevealComponent::SetRevealEnabled(bool bEnabled) { bRevealEnabled=bEnabled; Publish(); }
void UFlashlightRevealComponent::BeginPlay() {
 Super::BeginPlay();
 if(!Light) Light=GetOwner()->FindComponentByClass<USpotLightComponent>();
 Initialize(Light);
}
void UFlashlightRevealComponent::Publish() {
 Snapshot.Enabled=false; Snapshot.FrameId=GFrameCounter;
 if(!IsValid(Light) || Light->GetWorld()!=GetWorld()) return;
 Snapshot.Position=Light->GetComponentLocation();
 Snapshot.Direction=Light->GetForwardVector().GetSafeNormal();
 const FVector2f Angles=Light->GetClampedConeAngles();
 Snapshot.InnerHalfAngle=FMath::RadiansToDegrees(Angles.X);
 Snapshot.OuterHalfAngle=FMath::RadiansToDegrees(Angles.Y);
 Snapshot.RangeEnd=Light->AttenuationRadius; Snapshot.RangeStart=.9f*Snapshot.RangeEnd;
 const bool AnglesValid=FMath::IsFinite(Light->InnerConeAngle) && FMath::IsFinite(Light->OuterConeAngle)
  && Light->InnerConeAngle>=0 && Light->InnerConeAngle<Light->OuterConeAngle && Light->OuterConeAngle<90;
 Snapshot.Enabled=bRevealEnabled && Light->IsVisible() && AnglesValid && Snapshot.IsValid();
}
void UFlashlightRevealComponent::TickComponent(float D,ELevelTick T,FActorComponentTickFunction* F) {
 Super::TickComponent(D,T,F); Publish();
}
void UFlashlightRevealComponent::EndPlay(const EEndPlayReason::Type R) {
 Snapshot.Enabled=false; Super::EndPlay(R);
}
