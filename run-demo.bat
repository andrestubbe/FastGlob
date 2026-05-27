@echo off
echo ⚡ Building Main Project...
call mvn -q clean package -DskipTests
if %ERRORLEVEL% NEQ 0 ( pause & exit /b )
echo 🚀 Running Demo...
call mvn -q -f examples/Demo/pom.xml compile
call java --enable-native-access=ALL-UNNAMED -cp "target\fastglob-0.1.0.jar;examples\Demo\target\classes" fastglob.Demo
pause
