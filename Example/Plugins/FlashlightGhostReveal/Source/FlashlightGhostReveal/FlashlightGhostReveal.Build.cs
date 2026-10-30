using UnrealBuildTool;
public class FlashlightGhostReveal : ModuleRules {
 public FlashlightGhostReveal(ReadOnlyTargetRules Target) : base(Target) {
  PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","RenderCore"});
  PrivateDependencyModuleNames.AddRange(new[]{"RHI"});
 }
}
