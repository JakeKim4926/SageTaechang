param(
    [Parameter(Mandatory=$true)][string]$InputPath,
    [Parameter(Mandatory=$true)][string]$TemplatePath,
    [Parameter(Mandatory=$false)][string]$RowNums = '',
    [Parameter(Mandatory=$true)][string]$OutputFolder,
    [Parameter(Mandatory=$true)][string]$ResultPath,
    [Parameter(Mandatory=$false)][switch]$OnePageMode
)

$ErrorActionPreference = 'Stop'

$rowNumList = @()
if ($RowNums.Trim().Length -gt 0) {
    $rowNumList = @($RowNums -split ',' |
        Where-Object { $_.Trim() -match '^\d+$' } |
        ForEach-Object { [int]$_.Trim() })
}

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

function Safe-FileName($value) {
    $name = (ConvertTo-TextValue $value).Trim()
    if ($name.Length -eq 0) { $name = 'estimate' }
    $invalid = [System.IO.Path]::GetInvalidFileNameChars()
    foreach ($ch in $invalid) {
        $name = $name.Replace([string]$ch, '_')
    }
    $name = [regex]::Replace($name, '\.{2,}', '.').Trim().TrimEnd([char]'.')
    if ($name.Length -eq 0) { $name = 'estimate' }
    return $name
}

function Set-CellText($sheet, $address, $value) {
    $sheet.Range($address).Value2 = [string](ConvertTo-TextValue $value)
}

function Set-CellNumber($sheet, $address, $value) {
    if ($null -eq $value) {
        $sheet.Range($address).Value2 = ''
        return
    }
    try {
        $sheet.Range($address).Value2 = [double]$value
    } catch {
        try { $sheet.Range($address).Value2 = [string]$value } catch {}
    }
}

function New-EstimateItem($values, $rowNum) {
    $companyName = ConvertTo-TextValue (Get-MatrixValue $values $rowNum 2)
    $dateSerial  = Get-DateSerial (Get-MatrixValue $values $rowNum 3)
    $itemName    = ConvertTo-TextValue (Get-MatrixValue $values $rowNum 4)
    $copies      = Get-MatrixValue $values $rowNum 5
    $pages       = Get-MatrixValue $values $rowNum 6
    $unitPrice   = Get-MatrixValue $values $rowNum 7
    $reportCost  = Get-MatrixValue $values $rowNum 8
    $coverCost   = Get-MatrixValue $values $rowNum 9
    $freight     = Get-MatrixValue $values $rowNum 10

    return [ordered]@{
        rowNum      = $rowNum
        companyName = $companyName
        dateSerial  = $dateSerial
        itemName    = $itemName
        copies      = $copies
        pages       = $pages
        unitPrice   = $unitPrice
        reportCost  = $reportCost
        coverCost   = $coverCost
        freight     = $freight
    }
}

function New-EstimateOutputPath($outputFolder, $item) {
    $dateStr = ''
    if ($item.dateSerial -gt 0) {
        $dateStr = [datetime]::FromOADate($item.dateSerial).ToString("yyyyMMdd")
    }
    $timeStr = (Get-Date).ToString("HHmmssfff")
    if ($dateStr.Length -gt 0) {
        $baseName = (Safe-FileName $item.companyName) + '_견적서_' + $dateStr + '_' + $timeStr
    } else {
        $baseName = (Safe-FileName $item.companyName) + '_견적서_' + $timeStr
    }
    $outputPath = [System.IO.Path]::Combine($outputFolder, $baseName + '.xlsx')
    $suffix = 1
    while ([System.IO.File]::Exists($outputPath)) {
        $outputPath = [System.IO.Path]::Combine($outputFolder, $baseName + '_' + $suffix + '.xlsx')
        $suffix++
    }
    return $outputPath
}

function Set-PrimaryEstimateRow($sheet, $item) {
    $kika = [char]0x8CB4 + [char]0x4E0B
    Set-CellText $sheet 'A4' ($item.companyName + $kika)

    if ($item.dateSerial -gt 0) {
        $sheet.Range('A2').Value2 = $item.dateSerial
    } else {
        $sheet.Range('A2').Value2 = ''
    }

    Set-CellText   $sheet 'A9'  $item.itemName
    Set-CellNumber $sheet 'B9'  $item.pages
    Set-CellNumber $sheet 'C9'  $item.copies
    Set-CellNumber $sheet 'E9'  $item.unitPrice
    Set-CellNumber $sheet 'F10' $item.coverCost
    $freightText = ConvertTo-TextValue $item.freight
    if ($freightText.Trim().Length -gt 0) {
        Set-CellNumber $sheet 'F11' $item.freight
    } else {
        Set-CellText   $sheet 'F11' ''
    }

    Set-CellText $sheet 'G9'  ([char]0xB0B4+[char]0xC6A9+' '+[char]0xC778+[char]0xC1C4+' '+[char]0xBC0F+' '+[char]0xC7AC+[char]0xB2E8)
    Set-CellText $sheet 'G10' ([char]0xD45C+[char]0xC9C0+' '+[char]0xC778+[char]0xC1C4+' '+[char]0xBC0F+' '+[char]0xC81C+[char]0xBCF8)
    if ($freightText.Trim().Length -gt 0) {
        Set-CellText $sheet 'G11' ([char]0xC6B4+[char]0xC784)
    } else {
        Set-CellText $sheet 'G11' ''
    }
}

function Set-AdditionalEstimateRow($sheet, $item, $startRow) {
    $endRow = $startRow + 2
    $mergeRange = $sheet.Range(('A{0}:A{1}' -f $startRow, $endRow))
    $mergeRange.Merge() | Out-Null
    $mergeRange.HorizontalAlignment = -4108
    $mergeRange.VerticalAlignment = -4108

    Set-CellText   $sheet ('A{0}' -f $startRow) $item.itemName
    Set-CellNumber $sheet ('B{0}' -f $startRow) $item.pages
    Set-CellNumber $sheet ('C{0}' -f $startRow) $item.copies
    Set-CellNumber $sheet ('E{0}' -f $startRow) $item.unitPrice
    Set-CellNumber $sheet ('F{0}' -f $startRow) $item.reportCost
    Set-CellNumber $sheet ('F{0}' -f ($startRow + 1)) $item.coverCost
    $freightText = ConvertTo-TextValue $item.freight
    if ($freightText.Trim().Length -gt 0) {
        Set-CellNumber $sheet ('F{0}' -f ($startRow + 2)) $item.freight
    } else {
        Set-CellText $sheet ('F{0}' -f ($startRow + 2)) ''
    }

    Set-CellText $sheet ('G{0}' -f $startRow) ([char]0xB0B4+[char]0xC6A9+' '+[char]0xC778+[char]0xC1C4+' '+[char]0xBC0F+' '+[char]0xC7AC+[char]0xB2E8)
    Set-CellText $sheet ('G{0}' -f ($startRow + 1)) ([char]0xD45C+[char]0xC9C0+' '+[char]0xC778+[char]0xC1C4+' '+[char]0xBC0F+' '+[char]0xC81C+[char]0xBCF8)
    if ($freightText.Trim().Length -gt 0) {
        Set-CellText $sheet ('G{0}' -f ($startRow + 2)) ([char]0xC6B4+[char]0xC784)
    } else {
        Set-CellText $sheet ('G{0}' -f ($startRow + 2)) ''
    }
}

$excel            = $null
$inputWorkbook    = $null
$templateWorkbook = $null
$items            = New-Object System.Collections.ArrayList

try {
    if (-not [System.IO.File]::Exists($InputPath))    { throw "Input file was not found: $InputPath" }
    if (-not [System.IO.File]::Exists($TemplatePath)) { throw "Template file was not found: $TemplatePath" }
    if (-not [System.IO.Directory]::Exists($OutputFolder)) {
        [System.IO.Directory]::CreateDirectory($OutputFolder) | Out-Null
    }

    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false
    try { $excel.ScreenUpdating = $false } catch {}
    try { $excel.EnableEvents = $false } catch {}
    try { $excel.Calculation = -4135 } catch {}

    $inputWorkbook = $excel.Workbooks.Open($InputPath)
    $inputSheet    = $inputWorkbook.Worksheets.Item(1)
    if ($rowNumList.Count -eq 0) {
        $xlUp = -4162
        $lastRow = $inputSheet.Cells($inputSheet.Rows.Count, 2).End($xlUp).Row
        if ($lastRow -ge 6) {
            $rowNumList = 6..$lastRow
        }
    }
    $lastInputRow = 5
    if ($rowNumList.Count -gt 0) {
        $lastInputRow = ($rowNumList | Measure-Object -Maximum).Maximum
    }
    $inputValues = $inputSheet.Range(('A1:J{0}' -f $lastInputRow)).Value2
    $templateWorkbook = $excel.Workbooks.Open($TemplatePath)
    $sheet = $templateWorkbook.Worksheets.Item(1)

    if ($OnePageMode -and $rowNumList.Count -gt 6) {
        throw 'One page mode supports up to 6 rows.'
    }

    if ($OnePageMode) {
        if ($rowNumList.Count -gt 0) {
            $sheet.Range('A13:G27').ClearContents() | Out-Null

            $firstItem = New-EstimateItem $inputValues ($rowNumList[0])
            Set-PrimaryEstimateRow $sheet $firstItem
            for ($i = 1; $i -lt $rowNumList.Count; $i++) {
                $item = New-EstimateItem $inputValues ($rowNumList[$i])
                $startRow = 13 + (($i - 1) * 3)
                Set-AdditionalEstimateRow $sheet $item $startRow
            }

            $outputPath = New-EstimateOutputPath $OutputFolder $firstItem
            $templateWorkbook.SaveCopyAs($outputPath)
            [void]$items.Add([ordered]@{
                rowNum      = $RowNums
                filePath    = $outputPath
                fileName    = [System.IO.Path]::GetFileName($outputPath)
                companyName = $firstItem.companyName
                itemName    = $firstItem.itemName
                status      = 'success'
            })
        }
    } else {
        foreach ($rowNum in $rowNumList) {
            $item = New-EstimateItem $inputValues $rowNum
            $outputPath = New-EstimateOutputPath $OutputFolder $item

            Set-PrimaryEstimateRow $sheet $item
            $templateWorkbook.SaveCopyAs($outputPath)

            [void]$items.Add([ordered]@{
                rowNum      = $rowNum
                filePath    = $outputPath
                fileName    = [System.IO.Path]::GetFileName($outputPath)
                companyName = $item.companyName
                itemName    = $item.itemName
                status      = 'success'
            })
        }
    }

    $templateWorkbook.Close($false)
    $templateWorkbook = $null

    $inputWorkbook.Close($false)
    $inputWorkbook = $null

    $result = [ordered]@{
        status         = 'success'
        generatedCount = $items.Count
        outputFolder   = $OutputFolder
        files          = $items
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
}
catch {
    if ($templateWorkbook -ne $null) { try { $templateWorkbook.Close($false) } catch {} }
    if ($inputWorkbook    -ne $null) { try { $inputWorkbook.Close($false) } catch {} }
    $result = [ordered]@{
        status         = 'error'
        message        = $_.Exception.Message
        generatedCount = $items.Count
        outputFolder   = $OutputFolder
        files          = $items
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
    exit 1
}
finally {
    if ($excel -ne $null) { try { $excel.Quit() } catch {} }
}
