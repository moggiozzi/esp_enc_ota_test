::@echo off
if [%5]==[] goto usage

:: Init dependencies
if defined IDF_PYTHON_DIR ( goto end_init )
set "IDF_PYTHON_DIR="%LOCALAPPDATA%\Programs\Python\Python37\""
set "IDF_GIT_DIR="C:\Program Files\Git\cmd\""
set "IDF_TOOLS_JSON_PATH=%IDF_PATH%\tools\tools.json"
set "PATH=%IDF_PYTHON_DIR%;%IDF_GIT_DIR%;%PATH%"
set "IDF_TOOLS_PY_PATH=%IDF_PATH%\tools\idf_tools.py"
set "IDF_TOOLS_EXPORTS_FILE=%TEMP%\idf_export_vars.tmp"
python.exe "%IDF_TOOLS_PY_PATH%" --tools-json "%IDF_TOOLS_JSON_PATH%" export --format key-value >"%IDF_TOOLS_EXPORTS_FILE%"
if %errorlevel% neq 0 goto :end
for /f "usebackq tokens=1,2 eol=# delims==" %%a in ("%IDF_TOOLS_EXPORTS_FILE%") do (
      call set "%%a=%%b"
    )
call set PATH_ADDITIONS=%%PATH:%OLD_PATH%=%%
python.exe %IDF_PATH%\tools\check_python_dependencies.py
:end_init

python %IDF_PATH%\components\esptool_py\esptool\esptool.py --chip esp32 --port %1 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 %2 0x8000 %3 0x10000 %4 0x100000 %5
exit /B 0

:usage
echo Usage example: fw_write.bat com27 bootloader.en partition-table.en fw0_v366.en fw1_v366.en
exit /B 1