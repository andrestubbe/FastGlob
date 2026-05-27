@echo off
echo ⚡ Building Main Project...
call mvn -q clean package -DskipTests
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ Maven build failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo 🚀 Running Benchmark...
call mvn -q -f examples/Benchmark/pom.xml compile
call java --enable-native-access=ALL-UNNAMED -cp "target\fastglob-0.1.0.jar;examples\Benchmark\target\classes" fastglob.Benchmark
pause
