using UnrealBuildTool;
public class DEMO_MultiGameServerTarget : TargetRules
{
	public DEMO_MultiGameServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("DEMO_MultiGame");
	}
}