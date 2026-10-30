#include "FlashlightRevealDepthComponent.h"
#include "GhostRevealComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "TextureResource.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/IConsoleManager.h"

UFlashlightRevealDepthComponent::UFlashlightRevealDepthComponent()
{
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.TickGroup=TG_PostUpdateWork;
}
bool UFlashlightRevealDepthComponent::Initialize(USpotLightComponent* Light)
{
    InvalidateTargets();
    if(IsValid(Source)) RemoveTickPrerequisiteComponent(Source);
    if(!IsValid(Source)) {
        Source=NewObject<UFlashlightRevealComponent>(GetOwner());
        Source->Light=Light;
        GetOwner()->AddInstanceComponent(Source); Source->RegisterComponent();
    }
    const bool Result=Source->Initialize(Light);
    AddTickPrerequisiteComponent(Source); ++Generation;
    return Result;
}
void UFlashlightRevealDepthComponent::SetRevealEnabled(bool bEnabled)
{
    if(IsValid(Source)) Source->SetRevealEnabled(bEnabled);
    if(!bEnabled) { const bool WasValid=IsDepthValid(); InvalidateTargets(); if(WasValid) NotifyHistoryInvalidation(); }
}
bool UFlashlightRevealDepthComponent::RegisterTarget(UGhostRevealComponent* Target)
{
    if(!IsValid(Target) || Target->GetWorld()!=GetWorld()) return false;
    if(Target->DepthSource && Target->DepthSource!=this) Target->DepthSource->UnregisterTarget(Target);
    Target->DepthSource=this; Targets.AddUnique(Target); Target->InvalidateRevealDepth(); return true;
}
void UFlashlightRevealDepthComponent::UnregisterTarget(UGhostRevealComponent* Target)
{
    Targets.Remove(Target);
    if(IsValid(Target)) { Target->InvalidateRevealDepth(); if(Target->DepthSource==this) Target->DepthSource=nullptr; }
}
void UFlashlightRevealDepthComponent::InvalidateTargets()
{
    PublishedIndex=INDEX_NONE;
    for(const auto& Target:Targets) if(IsValid(Target.Get())) Target->InvalidateRevealDepth();
}
void UFlashlightRevealDepthComponent::BeginPlay()
{
    Super::BeginPlay();
    AddTickPrerequisiteActor(GetOwner());
    if(!Source) Source=GetOwner()->FindComponentByClass<UFlashlightRevealComponent>();
    if(IsValid(Source)) AddTickPrerequisiteComponent(Source);
    else Initialize(GetOwner()->FindComponentByClass<USpotLightComponent>());
    Capture=NewObject<USceneCaptureComponent2D>(GetOwner(),TEXT("RevealLightDepthCapture"));
    Capture->SetupAttachment(GetOwner()->GetRootComponent());
    Capture->bCaptureEveryFrame=false; Capture->bCaptureOnMovement=false;
    Capture->bAlwaysPersistRenderingState=false;
    Capture->CaptureSource=SCS_SceneDepth;
    Capture->bOverride_CustomNearClippingPlane=true;
    Capture->ShowFlags.SetLighting(false); Capture->ShowFlags.SetPostProcessing(false);
    Capture->ShowFlags.SetFog(false); Capture->ShowFlags.SetAtmosphere(false);
    Capture->ShowFlags.SetVolumetricFog(false); Capture->ShowFlags.SetDynamicShadows(false);
    Capture->ShowFlags.SetTranslucency(false); Capture->ShowFlags.SetMotionBlur(false);
    Capture->ShowFlags.SetAntiAliasing(false); Capture->ShowFlags.SetTemporalAA(false);
    GetOwner()->AddInstanceComponent(Capture); Capture->RegisterComponent();
    InvalidateTargets();
}
bool UFlashlightRevealDepthComponent::PrepareResources()
{
    if(AllocatedResolution==Resolution && Buffers.Num()==2) return true;
    InvalidateTargets();
    Capture->TextureTarget=nullptr;
    Buffers.Empty();
    for(int32 I=0;I<2;++I)
    {
        auto* RT=NewObject<UTextureRenderTarget2D>(this);
        RT->ClearColor=FLinearColor::Black; RT->bAutoGenerateMips=false;
        RT->Filter=TF_Nearest; RT->AddressX=TA_Clamp; RT->AddressY=TA_Clamp;
        RT->InitCustomFormat(Resolution,Resolution,PF_R32_FLOAT,true);
        RT->UpdateResourceImmediate(true); Buffers.Add(RT);
    }
    AllocatedResolution=Resolution; ++Generation;
    UE_LOG(LogTemp,Display,TEXT("REVEAL_DEPTH_RESOURCES generation=%u size=%d format=R32_FLOAT buffers=2"),Generation,Resolution);
    return true;
}
void UFlashlightRevealDepthComponent::PublishPending()
{
    Published=Pending; PublishedIndex=PendingIndex; PublishedAtFrame=GFrameCounter;
    for(const auto& Target:Targets) if(IsValid(Target.Get())) Target->ApplyRevealDepth(Published,Buffers[PublishedIndex]);
}
void UFlashlightRevealDepthComponent::NotifyHistoryInvalidation()
{
    if(HistoryPolicy!=ERevealDepthHistoryPolicy::ResetMainViewWhenDepthInvalidates || !GetWorld()) return;
    const auto* AA=IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod"));
    if(!AA || !(AA->GetInt()==2 || AA->GetInt()==4)) return;
    auto* PC=GetWorld()->GetFirstPlayerController();
    if(PC && PC->IsLocalController() && PC->PlayerCameraManager)
    {
        // Explicit opt-in: this resets the whole main view's temporal history,
        // not a private TSR texture or an effect-local history region.
        PC->PlayerCameraManager->SetGameCameraCutThisFrame();
        UE_LOG(LogTemp,Display,TEXT("REVEAL_DEPTH_MAIN_VIEW_HISTORY_RESET frame=%llu"),GFrameCounter);
    }
}
void UFlashlightRevealDepthComponent::TickComponent(float Delta,ELevelTick Tick,FActorComponentTickFunction* Function)
{
    Super::TickComponent(Delta,Tick,Function);
    const bool WasValid=IsDepthValid();
    const bool ValidConfig=(Resolution==512 || Resolution==1024 || Resolution==2048)
        && (FilterSamples==1 || FilterSamples==4 || FilterSamples==9)
        && FMath::IsFinite(BiasCm) && BiasCm>=0 && FMath::IsFinite(NearClipCm) && NearClipCm>0;
    const bool Enabled=ValidConfig && GetWorld() && GetWorld()->Scene && IsValid(Source)
        && Source->GetWorld()==GetWorld() && Source->GetSnapshot().Enabled && IsValid(Source->Light);
    if(Enabled!=LastEnabled || (IsValid(Source) && LastLight.Get()!=Source->Light.Get()))
    {
        ++Generation; InvalidateTargets(); LastEnabled=Enabled;
        LastLight=IsValid(Source)?Source->Light.Get():nullptr;
    }
    if(!Enabled)
    {
        InvalidateTargets();
        if(WasValid) NotifyHistoryInvalidation();
        return;
    }
    if(!PrepareResources()) { if(WasValid && !IsDepthValid()) NotifyHistoryInvalidation(); return; }
    Pending.Light=Source->GetSnapshot();
    Pending.AngularMode=Source->AngularMode;
    const FQuat Rotation=Source->Light->GetComponentQuat();
    Pending.Right=Rotation.GetRightVector(); Pending.Up=Rotation.GetUpVector();
    Pending.TanHalfFOV=FMath::Tan(FMath::DegreesToRadians(Pending.Light.OuterHalfAngle));
    Pending.NearClip=NearClipCm; Pending.Bias=BiasCm; Pending.Resolution=Resolution;
    Pending.FilterSamples=FilterSamples; Pending.Generation=Generation; Pending.CaptureRequestFrame=GFrameCounter;
    PendingIndex=PublishedIndex==0?1:0;
    Capture->SetWorldLocationAndRotation(Pending.Light.Position,Rotation);
    Capture->FOVAngle=2*Pending.Light.OuterHalfAngle;
    Capture->CustomNearClippingPlane=NearClipCm;
    Capture->HiddenComponents.Reset();
    for(const auto& Target:Targets) if(IsValid(Target.Get()))
        for(const auto& Mesh:Target->GetBoundMeshes()) if(IsValid(Mesh.Get())) Capture->HiddenComponents.AddUnique(Mesh.Get());
    for(const auto& Primitive:AdditionalHiddenComponents) if(IsValid(Primitive.Get())) Capture->HiddenComponents.AddUnique(Primitive.Get());
    Capture->TextureTarget=Buffers[PendingIndex];
    Capture->CaptureScene();
    // Capture submission precedes matching MID updates and main-view rendering.
    // Alternating external RTs retain ordered RHI work without a per-frame CPU wait.
    PublishPending();
}
void UFlashlightRevealDepthComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    const bool WasValid=IsDepthValid();
    ++Generation; InvalidateTargets();
    if(WasValid && Reason==EEndPlayReason::Destroyed) NotifyHistoryInvalidation();
    CaptureFence.BeginFence(); CaptureFence.Wait();
    if(Capture) { Capture->TextureTarget=nullptr; Capture->DestroyComponent(); }
    for(auto& Target:Targets) if(IsValid(Target) && Target->DepthSource==this) Target->DepthSource=nullptr;
    Targets.Empty();
    Super::EndPlay(Reason);
}
