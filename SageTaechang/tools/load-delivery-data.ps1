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

function ConvertTo-DateTextValue($value) {
    if ($null -ne $value) {
        try {
            $serial = [double]$value
            if ($serial -gt 0) { return [datetime]::FromOADate($serial).ToString('yyyy-MM-dd') }
        } catch {}
    }
    return ConvertTo-TextValue $value
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
    $used = $inputSheet.UsedRange
    $rowCount = $used.Row + $used.Rows.Count - 1
    $colCount = $used.Columns.Count
    if ($colCount -lt 11) { $colCount = 11 }
    $inputValues = $inputSheet.Range(('A1:K{0}' -f $rowCount)).Value2

    for ($rowIndex = 2; $rowIndex -le $rowCount; $rowIndex++) {
        $companyName = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 2)
        $itemName    = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 7)
        if ($companyName.Trim().Length -eq 0 -and $itemName.Trim().Length -eq 0) { continue }

        $department        = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 3)
        $orderDate         = ConvertTo-DateTextValue (Get-MatrixValue $inputValues $rowIndex 4)
        $deliveryDate      = ConvertTo-DateTextValue (Get-MatrixValue $inputValues $rowIndex 5)
        $deliveryTime      = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 6)
        $productType       = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 8)
        $companyCopies     = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 9)
        $corporationCopies = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 10)
        $totalCopies       = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 11)

        [void]$rows.Add([ordered]@{
            rowIndex           = $rowIndex
            companyName        = $companyName
            department         = $department
            orderDate          = $orderDate
            deliveryDate       = $deliveryDate
            deliveryTime       = $deliveryTime
            itemName           = $itemName
            productType        = $productType
            companyCopies      = $companyCopies
            corporationCopies  = $corporationCopies
            totalCopies        = $totalCopies
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
