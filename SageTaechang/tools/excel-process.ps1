function Get-ExcelProcessIds {
    return @(Get-Process -Name EXCEL -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id)
}

function Find-NewExcelProcessId($excelProcessIdsBefore) {
    $spawned = Get-Process -Name EXCEL -ErrorAction SilentlyContinue |
        Where-Object { $excelProcessIdsBefore -notcontains $_.Id } |
        Select-Object -First 1
    if ($null -eq $spawned) { return 0 }
    return $spawned.Id
}

function Stop-OwnedExcelProcess($excelProcessId) {
    if ($null -eq $excelProcessId -or $excelProcessId -le 0) { return }
    if ($null -eq (Get-Process -Id $excelProcessId -ErrorAction SilentlyContinue)) { return }
    try { Stop-Process -Id $excelProcessId -Force -ErrorAction Stop } catch {}
}
