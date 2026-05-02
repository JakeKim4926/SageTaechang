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
    $used = $inputSheet.UsedRange
    $rowCount = $used.Rows.Count

    for ($rowIndex = 2; $rowIndex -le $rowCount; $rowIndex++) {
        $companyName = Get-CellText $inputSheet $rowIndex 2
        $itemName    = Get-CellText $inputSheet $rowIndex 7
        if ($companyName.Trim().Length -eq 0 -and $itemName.Trim().Length -eq 0) { continue }

        $department        = Get-CellText $inputSheet $rowIndex 3
        $orderDate         = Get-CellText $inputSheet $rowIndex 4
        $deliveryDate      = Get-CellText $inputSheet $rowIndex 5
        $deliveryTime      = Get-CellText $inputSheet $rowIndex 6
        $productType       = Get-CellText $inputSheet $rowIndex 8
        $companyCopies     = Get-CellText $inputSheet $rowIndex 9
        $corporationCopies = Get-CellText $inputSheet $rowIndex 10
        $totalCopies       = Get-CellText $inputSheet $rowIndex 11

        $rows += [ordered]@{
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
