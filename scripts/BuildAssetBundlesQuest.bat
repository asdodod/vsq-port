@echo off
REM Adapted from the original VainSabers BuildAssetBundles.bat for Android.
setlocal
if not defined UNITY_PATH set "UNITY_PATH=C:\Program Files\Unity\Hub\Editor\2021.3.16f1\Editor\Unity.exe"
set "PROJECT_PATH=%~dp0..\unity"
if not exist "%UNITY_PATH%" (
    echo Unity not found. Set UNITY_PATH to Unity 2021.3.16f1 Editor Unity.exe.
    exit /b 1
)
if not exist "%PROJECT_PATH%\Logs" mkdir "%PROJECT_PATH%\Logs"
"%UNITY_PATH%" -projectPath "%PROJECT_PATH%" -buildTarget Android -executeMethod BuildQuestAssets.Build -quit -batchmode -nographics -logFile "%PROJECT_PATH%\Logs\BuildLog.txt"
exit /b %ERRORLEVEL%
