// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class MyProject : ModuleRules
{
    public MyProject(ReadOnlyTargetRules Target) : base(Target)
    {
        bEnableExceptions = true;

        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "InfraworldRuntime", "UnrealYAML" , "Sockets", "HTTP", "Json", "JsonUtilities", "UMG", "GeoReferencing", "ChaosVehicles", "MqttUtilities", "EnhancedInput", "AIModule", "NavigationSystem", "LevelSequence", "MovieScene", "CinematicCamera" });

        PrivateDependencyModuleNames.AddRange(new string[] { "InfraworldRuntime", "UnrealYAML", "OpenSSL", "zlib", "Slate", "SlateCore", "MqttUtilities" });


        PublicIncludePaths.AddRange(new string[] { "VirtualWorld" });
        PublicIncludePaths.AddRange(new string[] { "UnrealYAML/Public" });
        PublicIncludePaths.AddRange(new string[] { "MqttUtilities./Public" });
        PublicIncludePaths.AddRange(new string[] { "Engine/Plugins/EnhancedInput/Source/EnhancedInput/Public" });

        PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI");
        PublicDefinitions.Add("GPR_FORBID_UNREACHABLE_CODE");
        PublicDefinitions.Add("GRPC_ALLOW_EXCEPTIONS=0");
        PublicDefinitions.Add("GOOGLE_PROTOBUF_USE_UNALIGNED");

        string grpcIncludePath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Plugins/InfraworldRuntime/GrpcIncludes/include"));
        string grpcThirdPartiesPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Plugins/InfraworldRuntime/GrpcIncludes/third_party"));
        string UnrealYAMLCPPPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Plugins/unreal-yaml-main/Source"));

        PublicIncludePaths.Add(Path.Combine(grpcIncludePath, "grpc"));
        PublicIncludePaths.Add(Path.Combine(grpcIncludePath, "grpc++"));
        PublicIncludePaths.Add(Path.Combine(grpcIncludePath, "grpcpp"));
        PublicIncludePaths.Add(Path.Combine(grpcThirdPartiesPath, "protobuf"));
        PublicIncludePaths.Add(UnrealYAMLCPPPath);

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
