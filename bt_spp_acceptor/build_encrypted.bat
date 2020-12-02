@echo off

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

set /p ver=< version.txt
set binaryFileName=".\build\bt_spp_acceptor_demo_esp32.bin"
set enFileName0=".\build\bt_spp_acceptor_demo0_v%ver%.en"
set enFileName1=".\build\bt_spp_acceptor_demo1_v%ver%.en"

python.exe %IDF_PATH%\components\esptool_py\esptool\espsecure.py encrypt_flash_data --keyfile my_key.bin --address 0x1000 -o .\build\bootloader\bootloader.en .\build\bootloader\bootloader.bin
echo .\build\bootloader\bootloader.en

python.exe %IDF_PATH%\components\esptool_py\esptool\espsecure.py encrypt_flash_data --keyfile my_key.bin --address 0x8000 -o .\build\partition_table\partition-table.en .\build\partition_table\partition-table.bin
echo .\build\partition_table\partition-table.en

python.exe %IDF_PATH%\components\esptool_py\esptool\espsecure.py encrypt_flash_data --keyfile my_key.bin --address 0x10000 -o %enFileName0% %binaryFileName%
echo %enFileName0%
python.exe %IDF_PATH%\components\esptool_py\esptool\espsecure.py encrypt_flash_data --keyfile my_key.bin --address 0x100000 -o %enFileName1% %binaryFileName%
echo %enFileName1%

pause