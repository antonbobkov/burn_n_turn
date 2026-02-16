# Run simulation_test 100 times with up to 10 parallel jobs.
$Total = 100
$MaxParallel = 10
$BinDir = "e:\code\dragongame\bin"
$Exe = "$BinDir\simulation_test.exe"

$procs = @()
foreach ($i in 1..$Total) {
  while (($procs | Where-Object { -not $_.HasExited }).Count -ge $MaxParallel) {
    Start-Sleep -Milliseconds 50
  }
  $procs += Start-Process -FilePath $Exe -WorkingDirectory $BinDir -PassThru -NoNewWindow
}

$failed = 0
foreach ($p in $procs) {
  $p.WaitForExit()
  $ec = $p.ExitCode
  if ($null -ne $ec -and $ec -ne 0) { $failed++ }
}
Write-Host "Failed: $failed / $Total"
exit $(if ($failed -gt 0) { 1 } else { 0 })
