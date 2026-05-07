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
$QuickSuffix = ([char]0xD035).ToString()
$MiddleQuick = ([char]0xC911).ToString() + ' ' + ([char]0xD035).ToString()
$CourierSuffix = ([char]0xD0DD).ToString() + ([char]0xBC30).ToString()
$MiddleSuffix = ([char]0xC911).ToString()

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

function Get-MatrixValue($values, $row, $col) {
    try { return $values[$row, $col] } catch { return $null }
}

function Split-DateParts($value) {
    if ($null -ne $value) {
        try {
            $serial = [double]$value
            if ($serial -gt 0) {
                $dateValue = [datetime]::FromOADate($serial)
                return @{ Year = $dateValue.Year; Month = $dateValue.Month; Day = $dateValue.Day }
            }
        } catch {}
    }
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
$items = New-Object System.Collections.ArrayList

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
    try { $excel.ScreenUpdating = $false } catch {}
    try { $excel.EnableEvents = $false } catch {}
    try { $excel.Calculation = -4135 } catch {}

    $inputWorkbook = $excel.Workbooks.Open($InputPath)
    $inputSheet = $inputWorkbook.Worksheets.Item(1)
    $used = $inputSheet.UsedRange
    $rowCount = $used.Row + $used.Rows.Count - 1
    $colCount = $used.Columns.Count
    if ($colCount -lt 17) { $colCount = 17 }
    $inputValues = $inputSheet.Range(('A1:Q{0}' -f $rowCount)).Value2
    $templateWorkbook = $excel.Workbooks.Open($TemplatePath)
    $sheet = $templateWorkbook.Worksheets.Item(1)

    for ($rowIndex = 2; $rowIndex -le $rowCount; $rowIndex++) {
        if ($filterByRows -and -not $allowedRows.ContainsKey($rowIndex)) { continue }

        $hasValue = $false
        for ($col = 1; $col -le $colCount; $col++) {
            $value = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex $col)
            if ($value.Trim().Length -gt 0) {
                $hasValue = $true
            }
        }
        if (-not $hasValue) { continue }

        $companyName = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 2)
        $department = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 3)
        $orderDate = Split-DateParts (Get-MatrixValue $inputValues $rowIndex 4)
        $deliveryDate = Split-DateParts (Get-MatrixValue $inputValues $rowIndex 5)
        $deliveryTime = (ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 6)).Replace(' ', '')
        $itemName = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 7)
        $productType = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 8)
        $companyCopies = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 9)
        $corporationCopies = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 10)
        $totalCopies = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 11)
        $destination = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 12)
        $manager = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 13)
        $phone = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 14)
        $invoice = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 15)
        $memo = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 16)
        $numberValue = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowIndex 17)
        if ($numberValue.Trim().Length -eq 0) {
            $numberValue = ConvertTo-TextValue ($rowIndex - 1)
        }

        $generatedAt = Get-Date
        if ($deliveryDate.Year -ne '' -and $deliveryDate.Month -ne '' -and $deliveryDate.Day -ne '') {
            $datePart = '{0:D4}{1:D2}{2:D2}' -f [int]$deliveryDate.Year, [int]$deliveryDate.Month, [int]$deliveryDate.Day
        } else {
            $datePart = $generatedAt.ToString('yyyyMMdd')
        }
        $timePart = $generatedAt.ToString('HHmmss')
        $msPart = $generatedAt.ToString('fff')
        $baseName = (Safe-FileName $companyName) + '_' + (Safe-FileName $productType) + '_' + $datePart + '_' + $timePart + '_' + $msPart
        $outputPath = Join-Path $OutputFolder ($baseName + '.xls')
        $suffix = 1
        while (Test-Path -LiteralPath $outputPath) {
            $outputPath = Join-Path $OutputFolder ($baseName + '_' + $suffix + '.xls')
            $suffix++
        }

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
        Set-CellValue $sheet 'K2' ''
        if ($deliveryTime.EndsWith($QuickSuffix)) {
            if ($deliveryTime.StartsWith($MorningPrefix)) {
                Set-CellValue $sheet 'H6' $MiddleQuick
            } elseif ($deliveryTime.StartsWith($AfternoonPrefix)) {
                Set-CellValue $sheet 'M6' $MiddleQuick
            }
        } elseif ($deliveryTime.EndsWith($CourierSuffix)) {
            if ($deliveryTime.StartsWith($MorningPrefix)) {
                Set-CellValue $sheet 'H6' $MiddleDelivery
            } elseif ($deliveryTime.StartsWith($AfternoonPrefix)) {
                Set-CellValue $sheet 'M6' $MiddleDelivery
            }
            Set-CellValue $sheet 'K2' $CourierSuffix
        } elseif ($deliveryTime.EndsWith($MiddleSuffix)) {
            if ($deliveryTime.StartsWith($MorningPrefix)) {
                Set-CellValue $sheet 'H6' $MiddleSuffix
            } elseif ($deliveryTime.StartsWith($AfternoonPrefix)) {
                Set-CellValue $sheet 'M6' $MiddleSuffix
            }
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
        $templateWorkbook.SaveCopyAs($outputPath)

        [void]$items.Add([ordered]@{
            rowIndex = $rowIndex
            filePath = $outputPath
            fileName = [System.IO.Path]::GetFileName($outputPath)
            companyName = $companyName
            itemName = $itemName
            status = 'success'
        })
    }

    $templateWorkbook.Close($false)
    $templateWorkbook = $null

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
