param(
    [string]$PackageRoot = (Join-Path $PSScriptRoot '../build/desktop-test'),
    [string]$BuildRoot = (Join-Path $PSScriptRoot '../build/native/native/Release')
)

$ErrorActionPreference = 'Stop'
$packagePath = (Resolve-Path -LiteralPath $PackageRoot).Path
$buildPath = (Resolve-Path -LiteralPath $BuildRoot).Path
$verificationPath = Join-Path $packagePath 'verification'
New-Item -ItemType Directory -Path $verificationPath -Force | Out-Null
$reportPath = Join-Path $verificationPath 'startup-results.json'
if (Test-Path -LiteralPath $reportPath) { Remove-Item -LiteralPath $reportPath }

# Check that the installed executables are the ones just built, before launch.
$binaryHashes = @{}
foreach ($name in @('smartflow.exe', 'smartflow-data.exe')) {
    $installedHash = (Get-FileHash -LiteralPath (Join-Path $packagePath "bin/$name") -Algorithm SHA256).Hash
    $builtHash = (Get-FileHash -LiteralPath (Join-Path $buildPath $name) -Algorithm SHA256).Hash
    if ($installedHash -ne $builtHash) { throw "Installed $name differs from the build. Reinstall before testing." }
    $binaryHashes[$name] = $installedHash
}
foreach ($name in @('Qt6Core.dll', 'Qt6Gui.dll', 'Qt6Widgets.dll', 'vcruntime140.dll', 'msvcp140.dll', 'qt.conf')) {
    if (-not (Test-Path -LiteralPath (Join-Path $packagePath "bin/$name"))) { throw "Missing packaged runtime file: $name" }
}
if (-not (Test-Path -LiteralPath (Join-Path $packagePath 'plugins/platforms/qwindows.dll'))) {
    throw 'Missing packaged Windows platform plugin.'
}

$cases = @(
    @{ Name = 'scene'; App = 'smartflow.exe'; Options = @() },
    @{ Name = 'numeric'; App = 'smartflow.exe'; Options = @('--numeric') },
    @{ Name = 'data'; App = 'smartflow.exe'; Options = @('--data') },
    @{ Name = 'data-independent'; App = 'smartflow-data.exe'; Options = @() },
    @{ Name = 'scene-example'; App = 'smartflow.exe'; Options = @('--project', (Join-Path $packagePath 'examples/scene.smartflow')) },
    @{ Name = 'data-example'; App = 'smartflow-data.exe'; Options = @('--project', (Join-Path $packagePath 'examples/data.smartflow')) },
    @{ Name = 'component-example'; App = 'smartflow-data.exe'; Options = @('--project', (Join-Path $packagePath 'examples/components.smartflow')) },
    @{ Name = 'scene-tool'; App = 'smartflow.exe'; Options = @('--project', (Join-Path $packagePath 'examples/scene-tool.smartflow')) },
    @{ Name = 'data-tool'; App = 'smartflow-data.exe'; Options = @('--use', '--project', (Join-Path $packagePath 'examples/components.smartflow')) },
    @{ Name = 'scene-parallel'; App = 'smartflow.exe'; Options = @('--parallel', '--threads', '4', '--project', (Join-Path $packagePath 'examples/scene.smartflow')) },
    @{ Name = 'data-parallel'; App = 'smartflow-data.exe'; Options = @('--parallel', '--threads', '4', '--project', (Join-Path $packagePath 'examples/data.smartflow')) },
    @{ Name = 'component-parallel'; App = 'smartflow-data.exe'; Options = @('--parallel', '--threads', '4', '--project', (Join-Path $packagePath 'examples/components.smartflow')) }
)

$originalEnvironment = @{}
$isolatedEnvironment = @('PATH', 'QT_PLUGIN_PATH', 'QT_QPA_PLATFORM_PLUGIN_PATH', 'QT_QPA_PLATFORM', 'QML2_IMPORT_PATH', 'QTDIR')
$results = @()
try {
    foreach ($name in $isolatedEnvironment) {
        $originalEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
        [Environment]::SetEnvironmentVariable($name, $null, 'Process')
    }
    $env:PATH = "$env:SystemRoot/System32;$env:SystemRoot"
    foreach ($case in $cases) {
        $screenshot = Join-Path $verificationPath ($case.Name + '.png')
        $arguments = @('--smoke-test', '--screenshot', $screenshot) + $case.Options
        # Start-Process accepts a command-line string; quote each filesystem path
        # so test folders containing spaces work without invoking another shell.
        $quotedArguments = ($arguments | ForEach-Object { '"' + $_ + '"' }) -join ' '
        $process = Start-Process -FilePath (Join-Path $packagePath ('bin/' + $case.App)) `
            -ArgumentList $quotedArguments -WorkingDirectory $packagePath -WindowStyle Hidden -PassThru
        $null = $process.Handle
        if (-not $process.WaitForExit(15000)) {
            $process.Kill()
            throw "Packaged $($case.Name) startup timed out."
        }
        $process.Refresh()
        if ($process.ExitCode -ne 0) { throw "Packaged $($case.Name) startup failed with exit code $($process.ExitCode)." }
        if (-not (Test-Path -LiteralPath $screenshot)) { throw "Packaged $($case.Name) did not write its rendering check." }
        $results += @{ name = $case.Name; exitCode = $process.ExitCode; screenshot = "verification/$($case.Name).png" }
        Write-Output "PASS packaged $($case.Name)"
    }
} finally {
    foreach ($name in $originalEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $originalEnvironment[$name], 'Process')
    }
}

$sourceCommit = git -C (Join-Path $PSScriptRoot '..') rev-parse HEAD
if ($LASTEXITCODE -ne 0) { throw 'Could not record the source commit.' }
@{
    sourceCommit = $sourceCommit
    verifiedAtUtc = [DateTime]::UtcNow.ToString('o')
    binarySha256 = $binaryHashes
    platform = 'Windows native; developer Qt environment removed'
    startupChecks = $results
    manualInteractionAcceptance = 'pending user results'
} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $reportPath -Encoding UTF8
Write-Output "Verified desktop test folder: $packagePath"
