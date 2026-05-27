@echo off
echo Building main project...
call mvn clean package -DskipTests
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ❌ Maven build failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Running Benchmark...
call mvn -q -f examples/Benchmark/pom.xml compile
call java -cp "target\fastglob-1.0.0.jar;examples\Benchmark\target\classes" fastglob.Benchmark
pause
