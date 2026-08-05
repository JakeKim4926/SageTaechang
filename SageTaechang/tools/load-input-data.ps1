param(
    [Parameter(Mandatory=$true)][string]$InputPath,
    [Parameter(Mandatory=$true)][string]$ResultPath
)

$ErrorActionPreference = 'Stop'


function ConvertTo-TextValue($value) {
    if ($null -eq $value) { return '' }
    return [string]$value
}

function Get-CellText($sheet, $row, $col) {
    return ConvertTo-TextValue $sheet.Cells.Item($row, $col).Text
}

function Get-MatrixValue($values, $row, $col) {
    try { return $values[$row, $col] } catch { return $null }
}

function Get-DateSerial($value) {
    if ($null -eq $value) { return 0 }
    if ($value -is [datetime]) { return [int][math]::Floor($value.ToOADate()) }
    try { return [int][math]::Floor([double]$value) } catch { return 0 }
}

$excel = $null
$excelProcessId = 0
$inputWorkbook = $null
$rows = New-Object System.Collections.ArrayList

. (Join-Path $PSScriptRoot 'excel-process.ps1')

try {
    if (-not [System.IO.File]::Exists($InputPath)) {
        throw 'Input file was not found.'
    }

    $excelProcessIdsBefore = Get-ExcelProcessIds
    $excel = New-Object -ComObject Excel.Application
    $excelProcessId = Find-NewExcelProcessId $excelProcessIdsBefore
    $excel.Visible = $false
    $excel.DisplayAlerts = $false

    $inputWorkbook = $excel.Workbooks.Open($InputPath)
    $inputSheet = $inputWorkbook.Worksheets.Item(1)
    $xlUp = -4162
    $lastRow = $inputSheet.Cells($inputSheet.Rows.Count, 2).End($xlUp).Row
    if ($lastRow -lt 6) {
        $lastRow = 5
    }
    $inputValues = $inputSheet.Range(('A1:J{0}' -f $lastRow)).Value2

    for ($rowNum = 6; $rowNum -le $lastRow; $rowNum++) {
        $companyName = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 2)
        $itemName    = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 4)
        if ($companyName.Trim().Length -eq 0 -and $itemName.Trim().Length -eq 0) { continue }

        $dateSerial = Get-DateSerial (Get-MatrixValue $inputValues $rowNum 3)
        $dateText = ''
        if ($dateSerial -gt 0) {
            $dateText = [datetime]::FromOADate($dateSerial).ToString("yyyy-MM-dd")
        }

        $copies    = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 5)
        $pages     = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 6)
        $unitPrice = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 7)
        $coverCost = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 9)
        $freight   = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 10)

        [void]$rows.Add([ordered]@{
            rowNum      = $rowNum
            companyName = $companyName
            dateText    = $dateText
            dateSerial  = $dateSerial
            itemName    = $itemName
            copies      = $copies
            pages       = $pages
            unitPrice   = $unitPrice
            coverCost   = $coverCost
            freight     = $freight
        })
    }

    $inputWorkbook.Close($false)
    $inputWorkbook = $null

    $result = [ordered]@{
        status   = 'success'
        rowCount = $rows.Count
        rows     = $rows
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
}
catch {
    if ($inputWorkbook -ne $null) { try { $inputWorkbook.Close($false) } catch {} }
    $result = [ordered]@{
        status  = 'error'
        message = $_.Exception.Message
        rows    = @()
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
    exit 1
}
finally {
    if ($excel -ne $null) { try { $excel.Quit() } catch {} }
    Stop-OwnedExcelProcess $excelProcessId
}
