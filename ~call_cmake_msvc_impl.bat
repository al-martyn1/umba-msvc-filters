@set TOOLSET=%1
@set ARCH=%2
@set DO_BUILD=%3

:CHECK_GENERATOR_MSVC2019
@if not "%TOOLSET%"=="msvc2019" goto CHECK_GENERATOR_MSVC2022
@set GENERATOR="Visual Studio 16 2019"
@goto GENERATOR_GOOD

:CHECK_GENERATOR_MSVC2022
@if not "%TOOLSET%"=="msvc2022" goto CHECK_GENERATOR_MSVC2025
@set GENERATOR="Visual Studio 17 2022"
@goto GENERATOR_GOOD

:CHECK_GENERATOR_MSVC2025
@echo "Unknown MSVC version taken"
@goto ERR

:GENERATOR_GOOD

:CHECK_ARCH_X64
@if not "%ARCH%"=="x86" goto CHECK_ARCH_X64
@set ARCH=Win32
@set ARCH_PATH=x86
@goto ARCH_GOOD

:CHECK_ARCH_X64
@if not "%ARCH%"=="x64" goto CHECK_ARCH_UNKNOWN
@set ARCH=x64
@set ARCH_PATH=x64
@goto ARCH_GOOD

:CHECK_ARCH_UNKNOWN
@echo "Unknown Arch taken"
@goto ERR

:ARCH_GOOD
@if "%DO_BUILD%"=="BUILD" goto PERFORM_BUILD
@if "%DO_BUILD%"=="GENERATE" goto PERFORM_GENERATE
@echo "Unknown command taken, not BUILD nor GENERATE"
@goto ERR

:PERFORM_GENERATE
@cmake -G %GENERATOR%  -A %ARCH% -B .out\%TOOLSET%\%ARCH_PATH%
@goto END

:PERFORM_BUILD
@cmake --build .out\%TOOLSET%\%ARCH_PATH% --config Debug   --target ALL_BUILD -j 8 --
@cmake --build .out\%TOOLSET%\%ARCH_PATH% --config Release --target ALL_BUILD -j 8 --
@goto END


:ERR
exit /b 1

:END
exit /b 0

