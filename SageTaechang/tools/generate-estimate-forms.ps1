param(
    [Parameter(Mandatory=$true)][string]$InputPath,
    [Parameter(Mandatory=$true)][string]$TemplatePath,
    [Parameter(Mandatory=$false)][string]$RowNums = '',
    [Parameter(Mandatory=$true)][string]$OutputFolder,
    [Parameter(Mandatory=$true)][string]$ResultPath
)

$ErrorActionPreference = 'Stop'

$rowNumList = @()
if ($RowNums.Trim().Length -gt 0) {
    $rowNumList = $RowNums -split ',' |
        Where-Object { $_.Trim() -match '^\d+$' } |
        ForEach-Object { [int]$_.Trim() }
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

    foreach ($rowNum in $rowNumList) {
        $companyName = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 2)
        $dateSerial  = Get-DateSerial (Get-MatrixValue $inputValues $rowNum 3)
        $itemName    = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 4)
        $copies      = Get-MatrixValue $inputValues $rowNum 5
        $pages       = Get-MatrixValue $inputValues $rowNum 6
        $unitPrice   = Get-MatrixValue $inputValues $rowNum 7
        $coverCost   = Get-MatrixValue $inputValues $rowNum 9
        $freight     = Get-MatrixValue $inputValues $rowNum 10

        $dateStr = ''
        if ($dateSerial -gt 0) {
            $dateStr = [datetime]::FromOADate($dateSerial).ToString("yyyyMMdd")
        }
        $timeStr = (Get-Date).ToString("HHmmssfff")
        if ($dateStr.Length -gt 0) {
            $baseName = (Safe-FileName $companyName) + '_견적서_' + $dateStr + '_' + $timeStr
        } else {
            $baseName = (Safe-FileName $companyName) + '_견적서_' + $timeStr
        }
        $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '.xlsx')
        $suffix = 1
        while ([System.IO.File]::Exists($outputPath)) {
            $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '_' + $suffix + '.xlsx')
            $suffix++
        }

        $kika = [char]0x8CB4 + [char]0x4E0B
        Set-CellText $sheet 'A4' ($companyName + $kika)

        if ($dateSerial -gt 0) {
            $sheet.Range('A2').Value2 = $dateSerial
        } else {
            $sheet.Range('A2').Value2 = ''
        }

        Set-CellText   $sheet 'A9'  $itemName
        Set-CellNumber $sheet 'B9'  $pages
        Set-CellNumber $sheet 'C9'  $copies
        Set-CellNumber $sheet 'E9'  $unitPrice
        Set-CellNumber $sheet 'F10' $coverCost
        $freightText = ConvertTo-TextValue $freight
        if ($freightText.Trim().Length -gt 0) {
            Set-CellNumber $sheet 'F11' $freight
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

        $templateWorkbook.SaveCopyAs($outputPath)

        [void]$items.Add([ordered]@{
            rowNum      = $rowNum
            filePath    = $outputPath
            fileName    = [System.IO.Path]::GetFileName($outputPath)
            companyName = $companyName
            itemName    = $itemName
            status      = 'success'
        })
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
