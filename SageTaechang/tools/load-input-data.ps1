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

function Get-DateSerial($value) {
    if ($null -eq $value) { return 0 }
    if ($value -is [datetime]) { return [int][math]::Floor($value.ToOADate()) }
    try { return [int][math]::Floor([double]$value) } catch { return 0 }
}

$excel = $null
$inputWorkbook = $null
$rows = @()

try {
    if (-not [System.IO.File]::Exists($InputPath)) {
        throw 'Input file was not found.'
    }

    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false

    $inputWorkbook = $excel.Workbooks.Open($InputPath)
    $inputSheet = $inputWorkbook.Worksheets.Item(1)
    $xlUp = -4162
    $lastRow = $inputSheet.Cells($inputSheet.Rows.Count, 2).End($xlUp).Row
    if ($lastRow -lt 6) {
        $lastRow = 5
    }

    for ($rowNum = 6; $rowNum -le $lastRow; $rowNum++) {
        $companyName = Get-CellText $inputSheet $rowNum 2
        $itemName    = Get-CellText $inputSheet $rowNum 4
        if ($companyName.Trim().Length -eq 0 -and $itemName.Trim().Length -eq 0) { continue }

        $dateSerial = Get-DateSerial ($inputSheet.Cells.Item($rowNum, 3).Value2)
        $dateText = ''
        if ($dateSerial -gt 0) {
            $dateText = [datetime]::FromOADate($dateSerial).ToString("yyyy-MM-dd")
        }

        $copies    = ConvertTo-TextValue $inputSheet.Cells.Item($rowNum, 5).Value2
        $pages     = ConvertTo-TextValue $inputSheet.Cells.Item($rowNum, 6).Value2
        $unitPrice = ConvertTo-TextValue $inputSheet.Cells.Item($rowNum, 7).Value2
        $coverCost = ConvertTo-TextValue $inputSheet.Cells.Item($rowNum, 9).Value2
        $freight   = ConvertTo-TextValue $inputSheet.Cells.Item($rowNum, 10).Value2

        $rows += [ordered]@{
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
        }
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
}
