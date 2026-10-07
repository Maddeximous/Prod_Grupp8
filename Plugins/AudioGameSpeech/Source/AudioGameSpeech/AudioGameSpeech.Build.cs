using UnrealBuildTool;

public class AudioGameSpeech : ModuleRules
{
	public AudioGameSpeech(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.Add("Projects"); // IPluginManager (finds ThirdParty/Piper)

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Piper (built-in voice): ship piper.exe, its DLLs, espeak data and voices with packaged builds.
			// Committed to git. Missing? Run ThirdParty/Piper/Setup-Piper.ps1
			string PiperDir = System.IO.Path.Combine(PluginDirectory, "ThirdParty", "Piper");
			if (System.IO.File.Exists(System.IO.Path.Combine(PiperDir, "piper.exe")))
			{
				RuntimeDependencies.Add(System.IO.Path.Combine(PiperDir, "*.exe"), StagedFileType.NonUFS);
				RuntimeDependencies.Add(System.IO.Path.Combine(PiperDir, "*.dll"), StagedFileType.NonUFS);
				RuntimeDependencies.Add(System.IO.Path.Combine(PiperDir, "espeak-ng-data", "..."), StagedFileType.NonUFS);
				RuntimeDependencies.Add(System.IO.Path.Combine(PiperDir, "voices", "*.onnx"), StagedFileType.NonUFS);
				RuntimeDependencies.Add(System.IO.Path.Combine(PiperDir, "voices", "*.onnx.json"), StagedFileType.NonUFS);
			}

			// C++/WinRT reports errors with exceptions
			bEnableExceptions = true;

			// Windows Runtime (speech API)
			PublicSystemLibraries.Add("windowsapp.lib");

			PublicDefinitions.Add("WITH_WINRT_SPEECH=1");

			// If <winrt/...> headers are not found, add the SDK's cppwinrt folder, e.g.:
			// PublicSystemIncludePaths.Add(@"C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\cppwinrt");
			// UE 5.8 doesn't add it by default, so use the SDK this build picked.
			string SdkDir = Target.WindowsPlatform.WindowsSdkDir;
			string SdkVersion = Target.WindowsPlatform.WindowsSdkVersion;
			if (!string.IsNullOrEmpty(SdkDir) && !string.IsNullOrEmpty(SdkVersion))
			{
				PublicSystemIncludePaths.Add(System.IO.Path.Combine(SdkDir, "Include", SdkVersion, "cppwinrt"));
			}
		}
		else
		{
			PublicDefinitions.Add("WITH_WINRT_SPEECH=0");
		}
	}
}
