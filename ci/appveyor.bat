setlocal

echo Preparing environment
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

set ROOT=%~dp0..\
echo Downloading PHP SDK
cd %ROOT%
curl -L  -k -o php-sdk.zip https://github.com/OSTC/php-sdk-binary-tools/archive/php-sdk-2.0.9.zip || goto :error
7z x php-sdk.zip
del /Q php-sdk.zip
ren php-sdk-binary-tools-php-sdk-2.0.9 php-sdk
cd php-sdk

echo Setting Environment
REM php_setvars and phpsdk_buildtree will break the batch.
set PHP_SDK_VC=vc14
set PHP_SDK_BIN_PATH=%ROOT%php-sdk\bin
for %%a in ("%PHP_SDK_BIN_PATH%") do set PHP_SDK_ROOT_PATH=%%~dpa
set PHP_SDK_ROOT_PATH=%PHP_SDK_ROOT_PATH:~0,-1%
set PHP_SDK_MSYS2_PATH=%PHP_SDK_ROOT_PATH%\msys2\usr\bin
set PHP_SDK_PHP_CMD=%PHP_SDK_BIN_PATH%\php\do_php.bat
set PATH=%PHP_SDK_BIN_PATH%;%PHP_SDK_MSYS2_PATH%;%PATH%
cmd /c bin\phpsdk_buildtree.bat phpdev

echo Downloading php-src and deps
curl -L -k -o php-src.zip https://github.com/php/php-src/archive/php-7.1.8.zip || goto :error
7z x php-src.zip
del /Q php-src.zip
ren php-src-php-7.1.8 php-7.1.8
move php-7.1.8 phpdev\vc14\x64\php-7.1.8
cd phpdev\vc14\x64\php-7.1.8
cmd /c phpsdk_deps --update --branch 7.1 || goto :error

echo Building Extension for PHP
mkdir ..\pecl
mklink /D ..\pecl\php-fval ..\..\..\..\..\
cmd /c buildconf


cmd /c configure --disable-all --enable-cli --enable-fval=shared --disable-zts || goto :error
nmake || goto :error

REM echo Testing PHP
REM set PHP_RELEASE_PATH=%ROOT%php-sdk\phpdev\vc14\x64\php-7.1.8\x64\Release
REM curl -L -k -o %PHP_RELEASE_PATH%\phpunit.phar https://phar.phpunit.de/phpunit-5.7.phar || goto :error
REM cd %ROOT%\tests
REM echo PHP Test in Windows is unavailable yet.
REM Don't ``goto :error`` here before the extension can run on Windows.
REM cmd /c test.bat

goto :EOF


:error
echo Failed!
EXIT /b %ERRORLEVEL%
