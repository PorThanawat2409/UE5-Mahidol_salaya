# TorchTrack
Real-time 3D Map Synchronization using GPS tracking

This is a digital twin develop by Unreal Engine 5 map on Mahidol University Salaya Campus for exploration and GPS sync with our [mobile application](https://github.com/6487055yuttapichai/mahidol-GPS-app-.git).

## Features
+ Exploration on Mahidol University Salaya Campus.
+ GPS sync with mobile application.

## Requirements
+ Unreal Engine 5.11^ (install form Epic Game) 
+ Visual Studio (not vscode)
##### For use sync GPS with mobie application (can skip if use offline)
+ [TorchTrack Mobile application](https://github.com/6487055yuttapichai/mahidol-GPS-app-.git)
+ [MongoDB database API](https://github.com/PorThanawat2409/mongoDBAPI.git) 

## PreInstallation Setup

- Visual Studio need install 
   - Desktop development with c++
   - Game development with c++
   - Individual components
      - MSVC v143 - VS 2022 C++ ARM64 build tools (v14.33-17.3)
      - MSVC v143 - VS 2022 C++ ARM64/ARM64EC build tools (Latest)
      - MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
      - MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.33-17.3)
   
- Unreal Engine 5 (Recommend V5.1.1)
   - Optional for esay find error - install "Editor symbols for debugging"

## Installation

1. Clone this repository
```bash
   git clone <repository-url>
```
2. Navigate to the project directory:
```bash
   \MyProjectUE5-Mahidol_salaya\MyProject\
```
3. Rigth Click on "MyProject.uproject" file Select "Generate Visual Studio project files" you will get MyProject.sln file

4. Open MyProject.sln with Visual Studio 

##### In Visual Studio 

5. on the top toolbar "Solution Configulations"(Need mouse hover to show name) select Development Editor and "Solution Platforms" select Win64 

6. Select Build -> Build Solution

7. Select Build -> Build MyProject

8. Select Local Windows Debugger to Open Unreal Editor

##### In Unreal editor

9. Waiting for shader complie

10. Select "Play Icon" or press Alt+P to start



#### If want to use GPS sync with mobile app but have error 

- Config API URL in Config.ini (MyProjectUE5-Mahidol_salaya\MyProject\Content\VirtualWorld\Config.ini) to Your new API
- Mobile application recommend to use fake GPS & set location in Mahidol Salaya to test