@echo off
for /f "tokens=2" %%a in ('tasklist ^| findstr /C:"testsuit"') do (
    echo "%%a"
    taskkill /F /pid %%a /t
)
pause