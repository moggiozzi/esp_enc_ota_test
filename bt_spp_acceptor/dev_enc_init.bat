::@echo off
if [%2]==[] goto usage

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

:: Burning Encryption Key
python %IDF_PATH%\components\esptool_py\esptool\espefuse.py --port %1 burn_key flash_encryption %2
:: Enabling Flash Encryption mechanism
python %IDF_PATH%\components\esptool_py\esptool\espefuse.py --port %1 burn_efuse FLASH_CRYPT_CNT
:: Configuring Flash Encryption to use all address bits together with Encryption key (max value 0x0F)
python %IDF_PATH%\components\esptool_py\esptool\espefuse.py --port %1 burn_efuse FLASH_CRYPT_CONFIG 0x0F

python %IDF_PATH%\components\esptool_py\esptool\espefuse.py --port %1 burn_efuse DISABLE_DL_DECRYPT
python %IDF_PATH%\components\esptool_py\esptool\espefuse.py --port %1 burn_efuse DISABLE_DL_CACHE

:: for Release mode
exit /B 0
python %IDF_PATH%\components\esptool_py\esptool\espefuse.py --port %1 burn_efuse DISABLE_DL_ENCRYPT
python %IDF_PATH%\components\esptool_py\esptool\espefuse.py --port %1 write_protect_efuse DISABLE_DL_ENCRYPT

exit /B 0

:usage
echo Usage example: dev_enc_init.bat com27 ".\my_key.bin"
exit /B 1