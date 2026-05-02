param(
    [Parameter(Mandatory=$true)][string]$InputPath,
    [Parameter(Mandatory=$true)][string]$TemplatePath,
    [Parameter(Mandatory=$true)][string]$OutputFolder,
    [Parameter(Mandatory=$true)][string]$ResultPath,
    [Parameter(Mandatory=$false)][string]$RowNums = ''
)

$ErrorActionPreference = 'Stop'

$InputPath = [System.IO.Path]::GetFullPath($InputPath)
$TemplatePath = [System.IO.Path]::GetFullPath($TemplatePath)
$OutputFolder = [System.IO.Path]::GetFullPath($OutputFolder)
$ResultPath = [System.IO.Path]::GetFullPath($ResultPath)
$MorningPrefix = ([char]0xC624).ToString() + ([char]0xC804).ToString()
$AfternoonPrefix = ([char]0xC624).ToString() + ([char]0xD6C4).ToString()
$MiddleDelivery = ([char]0xC911).ToString() + ' ' + ([char]0xD0DD).ToString() + ([char]0xBC30).ToString()

$allowedRows = @{}
if ($RowNums.Trim().Length -gt 0) {
    foreach ($part in $RowNums.Split(',')) {
        $trimmed = $part.Trim()
        if ($trimmed.Length -gt 0) {
            $num = 0
            if ([int]::TryParse($trimmed, [ref]$num)) {
                $allowedRows[$num] = $true
            }
        }
    }
}
$filterByRows = $allowedRows.Count -gt 0

function ConvertTo-TextValue($value) {
    if ($null -eq $value) { return '' }
    return [string]$value
}

function Get-CellText($sheet, $row, $col) {
    return ConvertTo-TextValue $sheet.Cells.Item($row, $col).Text
}

function Split-DateParts($value) {
    $text = (ConvertTo-TextValue $value).Trim()
    $result = @{ Year = ''; Month = ''; Day = '' }
    if ($text -match '(\d{4})[-./](\d{1,2})[-./](\d{1,2})') {
        $result.Year = $matches[1]
        $result.Month = [int]$matches[2]
        $result.Day = [int]$matches[3]
    }
    return $result
}

function Safe-FileName($value) {
    $name = (ConvertTo-TextValue $value).Trim()
    if ($name.Length -eq 0) { $name = 'delivery' }
    foreach ($ch in [System.IO.Path]::GetInvalidFileNameChars()) {
        $name = $name.Replace([string]$ch, '_')
    }
    $name = [regex]::Replace($name, '\.{2,}', '.').Trim().TrimEnd([char]'.')
    if ($name.Length -eq 0) { $name = 'delivery' }
    return $name
}

function Set-CellValue($sheet, $address, $value) {
    $range = $sheet.Range($address)
    $range.Value2 = [string](ConvertTo-TextValue $value)
}

$excel = $null
$inputWorkbook = $null
$templateWorkbook = $null
$items = @()

try {
    if (-not (Test-Path -LiteralPath $InputPath)) {
        throw 'Input file was not found.'
    }
    if (-not (Test-Path -LiteralPath $TemplatePath)) {
        throw 'Template file was not found.'
    }
    if (-not (Test-Path -LiteralPath $OutputFolder)) {
        New-Item -ItemType Directory -Path $OutputFolder | Out-Null
    }

    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false

    $inputWorkbook = $excel.Workbooks.Open($InputPath)
    $inputSheet = $inputWorkbook.Worksheets.Item(1)
    $used = $inputSheet.UsedRange
    $rowCount = $used.Rows.Count
    $colCount = $used.Columns.Count

    for ($rowIndex = 2; $rowIndex -le $rowCount; $rowIndex++) {
        if ($filterByRows -and -not $allowedRows.ContainsKey($rowIndex)) { continue }

        $hasValue = $false
        for ($col = 1; $col -le $colCount; $col++) {
            $value = Get-CellText $inputSheet $rowIndex $col
            if ($value.Trim().Length -gt 0) {
                $hasValue = $true
            }
        }
        if (-not $hasValue) { continue }

        $companyName = Get-CellText $inputSheet $rowIndex 2
        $department = Get-CellText $inputSheet $rowIndex 3
        $orderDate = Split-DateParts (Get-CellText $inputSheet $rowIndex 4)
        $deliveryDate = Split-DateParts (Get-CellText $inputSheet $rowIndex 5)
        $deliveryTime = (Get-CellText $inputSheet $rowIndex 6).Replace(' ', '')
        $itemName = Get-CellText $inputSheet $rowIndex 7
        $productType = Get-CellText $inputSheet $rowIndex 8
        $companyCopies = Get-CellText $inputSheet $rowIndex 9
        $corporationCopies = Get-CellText $inputSheet $rowIndex 10
        $totalCopies = Get-CellText $inputSheet $rowIndex 11
        $destination = Get-CellText $inputSheet $rowIndex 12
        $manager = Get-CellText $inputSheet $rowIndex 13
        $phone = Get-CellText $inputSheet $rowIndex 14
        $invoice = Get-CellText $inputSheet $rowIndex 15
        $memo = Get-CellText $inputSheet $rowIndex 16
        $numberValue = Get-CellText $inputSheet $rowIndex 17
        if ($numberValue.Trim().Length -eq 0) {
            $numberValue = ConvertTo-TextValue ($rowIndex - 1)
        }

        $baseName = (Safe-FileName $numberValue) + '_' + (Safe-FileName $companyName) + '_' + (Safe-FileName $itemName)
        $outputPath = Join-Path $OutputFolder ($baseName + '.xls')
        $suffix = 1
        while (Test-Path -LiteralPath $outputPath) {
            $outputPath = Join-Path $OutputFolder ($baseName + '_' + $suffix + '.xls')
            $suffix++
        }

        $templateWorkbook = $excel.Workbooks.Open($TemplatePath)
        $sheet = $templateWorkbook.Worksheets.Item(1)
        Set-CellValue $sheet 'C4' $companyName
        Set-CellValue $sheet 'C5' $department
        Set-CellValue $sheet 'H4' $orderDate.Year
        Set-CellValue $sheet 'K4' $orderDate.Month
        Set-CellValue $sheet 'M4' $orderDate.Day
        Set-CellValue $sheet 'H5' $deliveryDate.Year
        Set-CellValue $sheet 'K5' $deliveryDate.Month
        Set-CellValue $sheet 'M5' $deliveryDate.Day
        Set-CellValue $sheet 'H6' ''
        Set-CellValue $sheet 'M6' ''
        if ($deliveryTime.StartsWith($MorningPrefix)) {
            Set-CellValue $sheet 'H6' $MiddleDelivery
        } elseif ($deliveryTime.StartsWith($AfternoonPrefix)) {
            Set-CellValue $sheet 'M6' $MiddleDelivery
        }
        Set-CellValue $sheet 'C9' $itemName
        Set-CellValue $sheet 'C12' $productType
        Set-CellValue $sheet 'D14' $companyCopies
        Set-CellValue $sheet 'G14' $corporationCopies
        Set-CellValue $sheet 'K14' $totalCopies
        Set-CellValue $sheet 'C15' $destination
        Set-CellValue $sheet 'C19' $manager
        Set-CellValue $sheet 'C22' $phone
        Set-CellValue $sheet 'C25' $invoice
        Set-CellValue $sheet 'C28' $memo
        Set-CellValue $sheet 'O38' $numberValue
        $templateWorkbook.SaveAs($outputPath, 56)
        $templateWorkbook.Close($false)
        $templateWorkbook = $null

        $items += [ordered]@{
            rowIndex = $rowIndex
            filePath = $outputPath
            fileName = [System.IO.Path]::GetFileName($outputPath)
            companyName = $companyName
            itemName = $itemName
            status = 'success'
        }
    }

    if ($inputWorkbook -ne $null) {
        $inputWorkbook.Close($false)
        $inputWorkbook = $null
    }

    $result = [ordered]@{
        status = 'success'
        generatedCount = $items.Count
        outputFolder = $OutputFolder
        files = $items
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
}
catch {
    if ($templateWorkbook -ne $null) {
        $templateWorkbook.Close($false)
    }
    if ($inputWorkbook -ne $null) {
        $inputWorkbook.Close($false)
    }
    $result = [ordered]@{
        status = 'error'
        message = $_.Exception.Message
        generatedCount = $items.Count
        outputFolder = $OutputFolder
        files = $items
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
    exit 1
}
finally {
    if ($excel -ne $null) {
        $excel.Quit()
    }
}
