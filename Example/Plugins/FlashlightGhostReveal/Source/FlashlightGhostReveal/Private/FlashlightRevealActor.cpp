#include "FlashlightRevealActor.h"
#include "FlashlightRevealComponent.h"
#include "FlashlightRevealDepthComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/Scene.h"
AFlashlightRevealActor::AFlashlightRevealActor() {
 Light=CreateDefaultSubobject<USpotLightComponent>(TEXT("Light")); RootComponent=Light;
 Light->InnerConeAngle=12; Light->OuterConeAngle=22; Light->AttenuationRadius=1500;
 Light->SetIntensityUnits(ELightUnits::Lumens); Light->Intensity=2500; Light->SetMobility(EComponentMobility::Movable);
 Source=CreateDefaultSubobject<UFlashlightRevealComponent>(TEXT("Source")); Source->Light=Light;
 Depth=CreateDefaultSubobject<UFlashlightRevealDepthComponent>(TEXT("Depth")); Depth->Source=Source;
}
void AFlashlightRevealActor::SetFlashlightEnabled(bool Enabled) {
 Light->SetVisibility(Enabled); Depth->SetRevealEnabled(Enabled);
}
