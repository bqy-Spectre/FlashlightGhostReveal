#include "FlashlightExample.h"
#include "FlashlightRevealActor.h"
#include "FlashlightRevealDepthComponent.h"
#include "GhostRevealComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/Canvas.h"
#include "Engine/StaticMesh.h"
#include "Components/SpotLightComponent.h"

AFlashlightExamplePlayer::AFlashlightExamplePlayer() {
 GetCapsuleComponent()->InitCapsuleSize(34,90);
 GetCharacterMovement()->MaxWalkSpeed=300;
 Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
 Camera->SetupAttachment(GetCapsuleComponent()); Camera->SetRelativeLocation(FVector(0,0,65));
 Camera->bUsePawnControlRotation=true; Camera->FieldOfView=80;
 bUseControllerRotationYaw=true;
}
void AFlashlightExamplePlayer::BeginPlay() {
 Super::BeginPlay();
 Camera->PostProcessSettings.bOverride_BloomIntensity=true;
 Camera->PostProcessSettings.BloomIntensity=0;
 FActorSpawnParameters P; P.Owner=this; P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 Flashlight=GetWorld()->SpawnActor<AFlashlightRevealActor>(AFlashlightRevealActor::StaticClass(),FTransform::Identity,P);
 Flashlight->AttachToComponent(Camera,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
 // The actual emitter shares the view origin/axis so one screen-space beam
 // covers foreground targets and background surfaces consistently.
 Flashlight->SetActorRelativeTransform(FTransform::Identity);
 auto* Body=NewObject<UStaticMeshComponent>(Flashlight,TEXT("FlashlightBody"));
 Body->SetupAttachment(Flashlight->GetRootComponent());
 Body->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
 Body->SetRelativeScale3D(FVector(.055,.055,.18)); Body->SetRelativeRotation(FRotator(90,0,0));
 // Keep the visible prop in the hand without offsetting the light emitter.
 Body->SetRelativeLocation(FVector(46,24,-14)); Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Body->SetCastShadow(false); Flashlight->AddInstanceComponent(Body); Body->RegisterComponent();
 Flashlight->Depth->AdditionalHiddenComponents.Add(Body);
 for(TActorIterator<AFlashlightExampleGhost> It(GetWorld());It;++It) Flashlight->Depth->RegisterTarget(It->Reveal);
 CaptureMouse();
}
void AFlashlightExamplePlayer::SetupPlayerInputComponent(UInputComponent* I) {
 Super::SetupPlayerInputComponent(I);
 I->BindAxis("Forward",this,&AFlashlightExamplePlayer::Forward);
 I->BindAxis("Right",this,&AFlashlightExamplePlayer::Right);
 I->BindAxis("Turn",this,&AFlashlightExamplePlayer::Turn);
 I->BindAxis("Look",this,&AFlashlightExamplePlayer::Look);
 I->BindAction("Flashlight",IE_Pressed,this,&AFlashlightExamplePlayer::ToggleLight);
 I->BindAction("ReleaseMouse",IE_Pressed,this,&AFlashlightExamplePlayer::ReleaseMouse);
 I->BindAction("CaptureMouse",IE_Pressed,this,&AFlashlightExamplePlayer::CaptureMouse);
}
void AFlashlightExamplePlayer::Forward(float V) { AddMovementInput(GetActorForwardVector(),V); }
void AFlashlightExamplePlayer::Right(float V) { AddMovementInput(GetActorRightVector(),V); }
void AFlashlightExamplePlayer::Turn(float V) { if(auto* PC=Cast<APlayerController>(Controller);PC && !PC->bShowMouseCursor) AddControllerYawInput(V); }
void AFlashlightExamplePlayer::Look(float V) { if(auto* PC=Cast<APlayerController>(Controller);PC && !PC->bShowMouseCursor) AddControllerPitchInput(V); }
void AFlashlightExamplePlayer::ToggleLight() { bLightEnabled=!bLightEnabled; if(Flashlight) Flashlight->SetFlashlightEnabled(bLightEnabled); }
void AFlashlightExamplePlayer::ReleaseMouse() {
 if(auto* PC=Cast<APlayerController>(Controller)) { PC->bShowMouseCursor=true; PC->SetInputMode(FInputModeGameAndUI()); }
}
void AFlashlightExamplePlayer::CaptureMouse() {
 if(auto* PC=Cast<APlayerController>(Controller)) { PC->bShowMouseCursor=false; PC->SetInputMode(FInputModeGameOnly()); }
}
AFlashlightExampleGhost::AFlashlightExampleGhost() {
 Mesh=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Manny")); RootComponent=Mesh;
 Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Mesh->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
 Reveal=CreateDefaultSubobject<UGhostRevealComponent>(TEXT("Reveal"));
}
void AFlashlightExampleGhost::BeginPlay() {
 Super::BeginPlay();
 if(auto* PC=GetWorld()->GetFirstPlayerController())
  if(auto* Player=Cast<AFlashlightExamplePlayer>(PC->GetPawn());Player && Player->Flashlight)
   Player->Flashlight->Depth->RegisterTarget(Reveal);
}
void AFlashlightExampleHUD::DrawHUD() {
 Super::DrawHUD();
 DrawText(TEXT("WASD Move   Mouse Aim   F Flashlight   Esc Release cursor"),FLinearColor::White,24,24,nullptr,1.1f);
 DrawText(TEXT("Sweep the light over Manny. Walk around the wall to see light occlusion."),FLinearColor(.7f,.85f,.9f),24,48);
 const float X=Canvas->ClipX*.5f,Y=Canvas->ClipY*.5f;
 DrawLine(X-4,Y,X+4,Y,FLinearColor::White); DrawLine(X,Y-4,X,Y+4,FLinearColor::White);
}
AFlashlightExampleGameMode::AFlashlightExampleGameMode() {
 DefaultPawnClass=AFlashlightExamplePlayer::StaticClass(); HUDClass=AFlashlightExampleHUD::StaticClass();
}
