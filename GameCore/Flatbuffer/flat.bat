set DST_DIR=D:\02.Trunk.Client\Assets\Share\Define\

set SRC_DIR=..\

COPY AnimationInformation.fbs %DST_DIR%AnimationInformation.fbs
COPY F4Packet.fbs %DST_DIR%F4Packet.fbs
COPY Common.fbs %DST_DIR%Common.fbs


::flatc.exe --csharp %SRC_DIR%Common.fbs
::flatc.exe --cpp -o %DST_DIR% %SRC_DIR%Common.fbs


@echo off
::for %%i in (*.fbs) do (
::		flatc.exe --csharp %%i --gen-mutable
::		flatc.exe --cpp -o ../../../../7.Common/GameCore/ %%i --gen-mutable
::	)
::flatc.exe --csharp F4Packet.fbs --gen-mutable
::flatc.exe --cpp -o ../../../../7.Common/GameCore/ F4Packet.fbs --gen-mutable

::Compile Options
:: --scoped-enums use c++ class enum.
:: --gen-mutable available modifying


flatc.exe --cpp -o ..\ AnimationInformation.fbs --scoped-enums
flatc.exe --cpp -o ..\ F4Packet.fbs --gen-mutable --scoped-enums --gen-object-api
flatc.exe --cpp -o ..\ Common.fbs --gen-mutable --scoped-enums --gen-object-api


pause