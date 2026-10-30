using UnrealBuildTool;
public class GhostRevealExample : ModuleRules { public GhostRevealExample(ReadOnlyTargetRules Target) : base(Target) {
 PCHUsage=PCHUsageMode.UseExplicitOrSharedPCHs; PublicIncludePaths.Add(ModuleDirectory); PublicDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","InputCore","FlashlightGhostReveal"}); } }
