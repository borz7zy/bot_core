$os = $PSVersionTable.PSPlatform

$scriptDir = Get-Location

$prebuildedLibsDir = Join-Path -Path $scriptDir -ChildPath "prebuilded-libs"

if (Test-Path -Path $prebuildedLibsDir) {
    Remove-Item -Recurse -Force -Path $prebuildedLibsDir
}

$wolfsslLibDir = Join-Path -Path $prebuildedLibsDir -ChildPath "wolfssl"
New-Item -Path $wolfsslLibDir -ItemType Directory -Force

$buildDir = Join-Path -Path $wolfsslLibDir -ChildPath "build"
New-Item -Path $buildDir -ItemType Directory -Force

Set-Location -Path $buildDir

$cmakeCommand = 'cmake -DCMAKE_INSTALL_PREFIX=../ -DBUILD_SHARED_LIBS=OFF ../../../libs/third_party/wolfssl'

Invoke-Expression $cmakeCommand
Invoke-Expression 'cmake --build .'
Invoke-Expression 'cmake --install . --prefix ../'

Set-Location -Path $scriptDir
