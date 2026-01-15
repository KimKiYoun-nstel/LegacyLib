param(
    [string]$AgentHost = "127.0.0.1",
    [int]$AgentPort = 25001,
    [int]$CliPort = 23000,
    [int]$LogPort = 24000,
    [switch]$NoBuild,
    [int]$ReadyTimeoutSec = 10,
    [int]$LogCollectSec = 3
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Stage {
    param([string]$Message)
    Write-Host ""
    Write-Host "=== $Message ==="
}

function Wait-TcpPort {
    param(
        [string]$TargetHost,
        [int]$Port,
        [int]$TimeoutSec
    )

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec) {
        try {
            $client = New-Object System.Net.Sockets.TcpClient
            $async = $client.BeginConnect($TargetHost, $Port, $null, $null)
            if ($async.AsyncWaitHandle.WaitOne(500)) {
                $client.EndConnect($async)
                $client.Close()
                return $true
            }
            $client.Close()
        } catch {
            # ignore and retry
        }
        Start-Sleep -Milliseconds 200
    }
    return $false
}

function Read-Available {
    param(
        [System.Net.Sockets.NetworkStream]$Stream,
        [int]$MaxMillis = 500
    )

    $buffer = New-Object byte[] 4096
    $sb = New-Object System.Text.StringBuilder
    $sw = [System.Diagnostics.Stopwatch]::StartNew()

    while ($sw.ElapsedMilliseconds -lt $MaxMillis) {
        if ($Stream.DataAvailable) {
            $read = $Stream.Read($buffer, 0, $buffer.Length)
            if ($read -gt 0) {
                $text = [System.Text.Encoding]::ASCII.GetString($buffer, 0, $read)
                $null = $sb.Append($text)
                continue
            }
            break
        }
        Start-Sleep -Milliseconds 50
    }

    return $sb.ToString()
}

function Send-CliCommand {
    param(
        [System.Net.Sockets.NetworkStream]$Stream,
        [string]$Command
    )

    $bytes = [System.Text.Encoding]::ASCII.GetBytes($Command + "`n")
    $Stream.Write($bytes, 0, $bytes.Length)
    $Stream.Flush()
    Start-Sleep -Milliseconds 100
    return Read-Available -Stream $Stream -MaxMillis 1000
}

function Read-LogWindow {
    param(
        [System.Net.Sockets.NetworkStream]$Stream,
        [int]$Seconds
    )

    $buffer = New-Object byte[] 4096
    $sb = New-Object System.Text.StringBuilder
    $sw = [System.Diagnostics.Stopwatch]::StartNew()

    while ($sw.Elapsed.TotalSeconds -lt $Seconds) {
        if ($Stream.DataAvailable) {
            $read = $Stream.Read($buffer, 0, $buffer.Length)
            if ($read -gt 0) {
                $text = [System.Text.Encoding]::ASCII.GetString($buffer, 0, $read)
                $null = $sb.Append($text)
                Write-Host $text -NoNewline
                continue
            }
        }
        Start-Sleep -Milliseconds 50
    }

    return $sb.ToString()
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$demoRoot = Resolve-Path (Join-Path $scriptDir "..")
$repoRoot = Resolve-Path (Join-Path $demoRoot "..")
$exePath = Join-Path $demoRoot "build_win\\demo_app.exe"

$proc = $null
$cliClient = $null
$logClient = $null

try {
    if (-not $NoBuild) {
        Write-Stage "Build DemoApp (Windows)"
        Push-Location $demoRoot
        & mingw32-make -f Makefile.windows
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed (mingw32-make exit $LASTEXITCODE)."
        }
        Pop-Location
    }

    if (-not (Test-Path $exePath)) {
        throw "demo_app.exe not found at $exePath"
    }

    Write-Stage "Start DemoApp"
    $args = "-cli_port $CliPort -log_port $LogPort -agent_host $AgentHost -agent_port $AgentPort"
    $proc = Start-Process -FilePath $exePath -ArgumentList $args -PassThru

    Write-Stage "Wait for CLI/Log ports"
    if (-not (Wait-TcpPort -TargetHost "127.0.0.1" -Port $CliPort -TimeoutSec $ReadyTimeoutSec)) {
        throw "CLI port $CliPort did not open in time."
    }
    if (-not (Wait-TcpPort -TargetHost "127.0.0.1" -Port $LogPort -TimeoutSec $ReadyTimeoutSec)) {
        throw "Log port $LogPort did not open in time."
    }

    Write-Stage "Connect CLI/Log"
    $cliClient = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $CliPort)
    $logClient = New-Object System.Net.Sockets.TcpClient("127.0.0.1", $LogPort)
    $cliStream = $cliClient.GetStream()
    $logStream = $logClient.GetStream()
    $cliStream.ReadTimeout = 500
    $logStream.ReadTimeout = 500

    $banner = Read-Available -Stream $cliStream -MaxMillis 1000
    if ($banner) { Write-Host $banner -NoNewline }

    Write-Stage "Run CLI Commands (struct mode)"
    $cliOutputs = @()
    $cliOutputs += Send-CliCommand -Stream $cliStream -Command "log_level debug"
    $cliOutputs += Send-CliCommand -Stream $cliStream -Command "codec struct"
    $cliOutputs += Send-CliCommand -Stream $cliStream -Command "connect $AgentHost $AgentPort"
    $cliOutputs += Send-CliCommand -Stream $cliStream -Command "create_entities"
    $cliOutputs += Send-CliCommand -Stream $cliStream -Command "test_write all"

    foreach ($resp in $cliOutputs) {
        if ($resp) {
            Write-Host $resp -NoNewline
        }
    }

    Write-Stage "Collect Log Output"
    $logText = Read-LogWindow -Stream $logStream -Seconds $LogCollectSec

    Write-Stage "Result Check"
    $allCli = ($cliOutputs -join "`n")
    $okConnect = $allCli -match "OK: Connected to Agent"
    $okEntities = $allCli -match "OK: DDS entities created"
    $okTest = $allCli -match "OK: Test message\(s\) sent"
    $hasHex = $logText -match "\[HEX\]"

    Write-Host ("Connected:     " + ($(if ($okConnect) { "OK" } else { "FAIL" })))
    Write-Host ("Entities:      " + ($(if ($okEntities) { "OK" } else { "FAIL" })))
    Write-Host ("Test Write:    " + ($(if ($okTest) { "OK" } else { "FAIL" })))
    Write-Host ("HEX Log Found: " + ($(if ($hasHex) { "OK" } else { "FAIL" })))

    if (-not ($okConnect -and $okEntities -and $okTest -and $hasHex)) {
        throw "One or more checks failed. Review output above."
    }
} finally {
    Write-Stage "Cleanup"
    if ($cliClient) { $cliClient.Close() }
    if ($logClient) { $logClient.Close() }
    if ($proc -and -not $proc.HasExited) {
        Stop-Process -Id $proc.Id -Force
    }
}
