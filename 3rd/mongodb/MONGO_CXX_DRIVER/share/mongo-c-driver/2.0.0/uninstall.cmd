@echo off

rem MongoDB C Driver uninstall program, generated with CMake
rem
rem Copyright 2009-present MongoDB, Inc.
rem
rem Licensed under the Apache License, Version 2.0 (the "License")
rem
rem you may not use this file except in compliance with the License.
rem You may obtain a copy of the License at
rem
rem   http://www.apache.org/licenses/LICENSE-2.0
rem
rem Unless required by applicable law or agreed to in writing, software
rem distributed under the License is distributed on an "AS IS" BASIS,
rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
rem See the License for the specific language governing permissions and
rem limitations under the License.

call :init

:print
<nul set /p_=%~1
exit /b

:rmfile
set f=%__prefix%\%~1
call :print "Remove file %f% "
if EXIST "%f%" (
    del /Q /F "%f%" || exit /b %errorlevel%
    call :print " - ok"
) else (
    call :print " - skipped: not present"
)
echo(
exit /b

:rmdir
set f=%__prefix%\%~1
call :print "Remove directory: %f% "
if EXIST "%f%" (
    rmdir /Q "%f%" 2>nul
    if ERRORLEVEL 0 (
        call :print "- ok"
    ) else (
        call :print "- skipped (non-empty?)"
    )
) else (
    call :print " - skipped: not present"
)
echo(
exit /b

:init
setlocal EnableDelayedExpansion
setlocal EnableExtensions
if /i "%~dp0" NEQ "%TEMP%\" (
    set tmpfile=%TEMP%\mongoc-%~nx0
    copy "%~f0" "!tmpfile!" >nul
    call "!tmpfile!" & del "!tmpfile!"
    exit /b
)

if "%DESTDIR%"=="" (
  set __prefix=C:\Program Files\MONGO_CXX_DRIVER
) else (
  set __prefix=!DESTDIR!\Program Files\MONGO_CXX_DRIVER
)

call :rmfile "bin\msvcp140.dll"
call :rmfile "bin\msvcp140_1.dll"
call :rmfile "bin\msvcp140_2.dll"
call :rmfile "bin\msvcp140_atomic_wait.dll"
call :rmfile "bin\msvcp140_codecvt_ids.dll"
call :rmfile "bin\vcruntime140_1.dll"
call :rmfile "bin\vcruntime140.dll"
call :rmfile "bin\concrt140.dll"
call :rmfile "bin\msvcp140.dll"
call :rmfile "bin\msvcp140_1.dll"
call :rmfile "bin\msvcp140_2.dll"
call :rmfile "bin\msvcp140_atomic_wait.dll"
call :rmfile "bin\msvcp140_codecvt_ids.dll"
call :rmfile "bin\vcruntime140_1.dll"
call :rmfile "bin\vcruntime140.dll"
call :rmfile "bin\concrt140.dll"
call :rmfile "bin\msvcp140.dll"
call :rmfile "bin\msvcp140_1.dll"
call :rmfile "bin\msvcp140_2.dll"
call :rmfile "bin\msvcp140_atomic_wait.dll"
call :rmfile "bin\msvcp140_codecvt_ids.dll"
call :rmfile "bin\vcruntime140_1.dll"
call :rmfile "bin\vcruntime140.dll"
call :rmfile "bin\concrt140.dll"
call :rmfile "lib\bson2.lib"
call :rmfile "lib\cmake\bson-2.0.0\bson_static-targets.cmake"
call :rmfile "lib\cmake\bson-2.0.0\bson_static-targets-release.cmake"
call :rmfile "lib\pkgconfig\bson2-static.pc"
call :rmfile "lib\bson2.dll.lib"
call :rmfile "bin\bson2.dll"
call :rmfile "lib\cmake\bson-2.0.0\bson_shared-targets.cmake"
call :rmfile "lib\cmake\bson-2.0.0\bson_shared-targets-release.cmake"
call :rmfile "lib\pkgconfig\bson2.pc"
call :rmfile "include\bson-2.0.0\bson\bcon.h"
call :rmfile "include\bson-2.0.0\bson\bson-clock.h"
call :rmfile "include\bson-2.0.0\bson\bson-compat.h"
call :rmfile "include\bson-2.0.0\bson\bson-context.h"
call :rmfile "include\bson-2.0.0\bson\bson-decimal128.h"
call :rmfile "include\bson-2.0.0\bson\bson-endian.h"
call :rmfile "include\bson-2.0.0\bson\bson-error.h"
call :rmfile "include\bson-2.0.0\bson\bson-iter.h"
call :rmfile "include\bson-2.0.0\bson\bson-json.h"
call :rmfile "include\bson-2.0.0\bson\bson-keys.h"
call :rmfile "include\bson-2.0.0\bson\bson-macros.h"
call :rmfile "include\bson-2.0.0\bson\bson-memory.h"
call :rmfile "include\bson-2.0.0\bson\bson-oid.h"
call :rmfile "include\bson-2.0.0\bson\bson-prelude.h"
call :rmfile "include\bson-2.0.0\bson\bson-reader.h"
call :rmfile "include\bson-2.0.0\bson\bson-string.h"
call :rmfile "include\bson-2.0.0\bson\bson-types.h"
call :rmfile "include\bson-2.0.0\bson\bson-utf8.h"
call :rmfile "include\bson-2.0.0\bson\bson-value.h"
call :rmfile "include\bson-2.0.0\bson\bson-vector.h"
call :rmfile "include\bson-2.0.0\bson\bson-version-functions.h"
call :rmfile "include\bson-2.0.0\bson\bson-writer.h"
call :rmfile "include\bson-2.0.0\bson\bson.h"
call :rmfile "include\bson-2.0.0\bson\bson-config.h"
call :rmfile "include\bson-2.0.0\bson\bson-version.h"
call :rmfile "lib\cmake\bson-2.0.0\00-mongo-platform-targets.cmake"
call :rmfile "lib\cmake\bson-2.0.0\bsonConfig.cmake"
call :rmfile "lib\cmake\bson-2.0.0\bsonConfigVersion.cmake"
call :rmfile "bin\msvcp140.dll"
call :rmfile "bin\msvcp140_1.dll"
call :rmfile "bin\msvcp140_2.dll"
call :rmfile "bin\msvcp140_atomic_wait.dll"
call :rmfile "bin\msvcp140_codecvt_ids.dll"
call :rmfile "bin\vcruntime140_1.dll"
call :rmfile "bin\vcruntime140.dll"
call :rmfile "bin\concrt140.dll"
call :rmfile "bin\msvcp140.dll"
call :rmfile "bin\msvcp140_1.dll"
call :rmfile "bin\msvcp140_2.dll"
call :rmfile "bin\msvcp140_atomic_wait.dll"
call :rmfile "bin\msvcp140_codecvt_ids.dll"
call :rmfile "bin\vcruntime140_1.dll"
call :rmfile "bin\vcruntime140.dll"
call :rmfile "bin\concrt140.dll"
call :rmfile "lib\pkgconfig\mongoc2.pc"
call :rmfile "lib\pkgconfig\mongoc2-static.pc"
call :rmfile "lib\mongoc2.lib"
call :rmfile "lib\mongoc2.dll.lib"
call :rmfile "bin\mongoc2.dll"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-config.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-version.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-apm.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-bulk-operation.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-bulkwrite.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-change-stream.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-client.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-client-pool.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-client-side-encryption.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-collection.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-cursor.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-database.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-error.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-flags.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-find-and-modify.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-gridfs.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-gridfs-bucket.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-gridfs-file.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-gridfs-file-page.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-gridfs-file-list.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-handshake.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-host-list.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-init.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-iovec.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-log.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-macros.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-opcode.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-optional.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-prelude.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-read-concern.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-read-prefs.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-server-api.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-server-description.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-client-session.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-sleep.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-socket.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-stream-tls-openssl.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-stream.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-stream-buffered.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-stream-file.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-stream-gridfs.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-stream-socket.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-structured-log.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-topology-description.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-uri.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-version-functions.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-write-concern.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-rand.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-stream-tls.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-ssl.h"
call :rmfile "include\mongoc-2.0.0\mongoc\mongoc-bulkwrite.h"
call :rmfile "lib\cmake\mongoc-2.0.0\mongoc-targets.cmake"
call :rmfile "lib\cmake\mongoc-2.0.0\mongoc-targets-release.cmake"
call :rmfile "lib\cmake\mongoc-2.0.0\mongocConfig.cmake"
call :rmfile "lib\cmake\mongoc-2.0.0\mongocConfigVersion.cmake"
call :rmfile "share\mongo-c-driver\2.0.0\COPYING"
call :rmfile "share\mongo-c-driver\2.0.0\NEWS"
call :rmfile "share\mongo-c-driver\2.0.0\README.rst"
call :rmfile "share\mongo-c-driver\2.0.0\THIRD_PARTY_NOTICES"
call :rmfile "share\mongo-c-driver\2.0.0\uninstall.cmd"
call :rmdir "share\mongo-c-driver\2.0.0"
call :rmdir "share\mongo-c-driver"
call :rmdir "lib\pkgconfig"
call :rmdir "lib\cmake\mongoc-2.0.0"
call :rmdir "lib\cmake\bson-2.0.0"
call :rmdir "lib\cmake"
call :rmdir "include\mongoc-2.0.0\mongoc"
call :rmdir "include\mongoc-2.0.0"
call :rmdir "include\bson-2.0.0\bson"
call :rmdir "include\bson-2.0.0"
