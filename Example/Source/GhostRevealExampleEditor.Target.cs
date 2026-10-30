using UnrealBuildTool;
public class GhostRevealExampleEditorTarget : TargetRules { public GhostRevealExampleEditorTarget(TargetInfo Target) : base(Target) {
 Type=TargetType.Editor; DefaultBuildSettings=BuildSettingsVersion.V7; IncludeOrderVersion=EngineIncludeOrderVersion.Unreal5_8;
 ExtraModuleNames.AddRange(new[]{"GhostRevealExample"}); } }
