# space_build.ps1 - compilación por fichero con estimador y diagnósticos
Param()

if (-not $env:SRC) { Write-Host "[BUILD] ERROR: SRC environment variable not set"; exit 1 }
$srcList = $env:SRC -split ' '

$compiler = 'C:\mingw64\bin\g++.exe'
$buildFlags = $env:BUILD_FLAGS
if (-not $buildFlags) { $buildFlags = '-O2' }
$includes = $env:INCLUDES_ENV
$libs = $env:LIBS_ENV
$out = $env:OUT_ENV

Write-Host "[BUILD] Working dir: $(Get-Location)"
Write-Host "[BUILD] Compiler: $compiler"
if (!(Test-Path $compiler)) {
    Write-Host "[BUILD] ERROR: Compiler not found at $compiler"
    Write-Host "Please install MinGW or adjust the compiler path in tools\space_build.ps1"
    exit 2
}

if (!(Test-Path 'build')) { New-Item -ItemType Directory -Path 'build' | Out-Null }

$total = $srcList.Count
$times = @()
$start = Get-Date
Write-Host "[BUILD] Started: $start"
for ($i=0; $i -lt $total; $i++) {
    $src = $srcList[$i].Trim()
    if ($src -eq '') { continue }
    if (!(Test-Path $src)) {
        Write-Host "[BUILD] ERROR: Fuente no encontrada: $src"
        exit 3
    }
    $name = [System.IO.Path]::GetFileNameWithoutExtension($src)
    $obj = Join-Path 'build' ($name + '.o')
    Write-Host "[BUILD] Compiling ($([int]($i+1))/$total): $src"
    $t0 = Get-Date
    $argList = @('-std=c++17') + ($buildFlags -split ' ') + @('-c', $src) + (($includes -split ' ') | Where-Object { $_ -ne '' }) + @('-o', $obj)
    Write-Host "[BUILD] CMD: $compiler $($argList -join ' ')"
    & $compiler @argList
    $exit = $LASTEXITCODE
    if ($exit -ne 0) { Write-Host "[BUILD] Error compiling $src (exit $exit)"; exit $exit }
    $t1 = Get-Date
    $dur = ($t1 - $t0).TotalSeconds
    $times += $dur
    $avg = ($times | Measure-Object -Sum).Sum / $times.Count
    $remaining = $avg * ($total - ($i+1))
    Write-Host ([string]::Format('[BUILD] Time: {0:N2}s | Avg: {1:N2}s | Est remaining: {2:N2}s', $dur, $avg, $remaining))
}

Write-Host '[BUILD] Linking...'
 $objsList = (Get-ChildItem -Path build -Filter *.o | ForEach-Object { $_.FullName })
 $argList = @('-std=c++17') + ($buildFlags -split ' ') + $objsList + (($libs -split ' ') | Where-Object { $_ -ne '' }) + @('-o', $out)
 Write-Host "[BUILD] LINK CMD: $compiler $($argList -join ' ')"
 & $compiler @argList
 $exit = $LASTEXITCODE
 if ($exit -ne 0) { Write-Host '[BUILD] Link error'; exit $exit }
$end = Get-Date
$totalSec = ($end - $start).TotalSeconds
Write-Host ([string]::Format('[BUILD] Finished: {0} | Total time: {1:N2}s', $end, $totalSec))
exit 0
