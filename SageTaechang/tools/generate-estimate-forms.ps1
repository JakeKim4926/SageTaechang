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
    if ($null -eq $value) { return }
    try {
        $sheet.Range($address).Value2 = [double]$value
    } catch {
        try { $sheet.Range($address).Value2 = [string]$value } catch {}
    }
}

$excel            = $null
$inputWorkbook    = $null
$templateWorkbook = $null
$items            = @()

try {
    if (-not [System.IO.File]::Exists($InputPath))    { throw "Input file was not found: $InputPath" }
    if (-not [System.IO.File]::Exists($TemplatePath)) { throw "Template file was not found: $TemplatePath" }
    if (-not [System.IO.Directory]::Exists($OutputFolder)) {
        [System.IO.Directory]::CreateDirectory($OutputFolder) | Out-Null
    }

    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false

    $inputWorkbook = $excel.Workbooks.Open($InputPath)
    $inputSheet    = $inputWorkbook.Worksheets.Item(1)
    if ($rowNumList.Count -eq 0) {
        $xlUp = -4162
        $lastRow = $inputSheet.Cells($inputSheet.Rows.Count, 2).End($xlUp).Row
        if ($lastRow -ge 6) {
            $rowNumList = 6..$lastRow
        }
    }

    foreach ($rowNum in $rowNumList) {
        $companyName = Get-CellText $inputSheet $rowNum 2
        $dateSerial  = Get-DateSerial ($inputSheet.Cells.Item($rowNum, 3).Value2)
        $itemName    = Get-CellText $inputSheet $rowNum 4
        $copies      = $inputSheet.Cells.Item($rowNum, 5).Value2
        $pages       = $inputSheet.Cells.Item($rowNum, 6).Value2
        $unitPrice   = $inputSheet.Cells.Item($rowNum, 7).Value2
        $coverCost   = $inputSheet.Cells.Item($rowNum, 9).Value2
        $freight     = $inputSheet.Cells.Item($rowNum, 10).Value2

        $dateStr = ''
        if ($dateSerial -gt 0) {
            $dateStr = [datetime]::FromOADate($dateSerial).ToString("yyyyMMdd")
        }

        $baseName   = (Safe-FileName $companyName) + '_estimate_' + $dateStr
        $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '.xlsx')
        $suffix = 1
        while ([System.IO.File]::Exists($outputPath)) {
            $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '_' + $suffix + '.xlsx')
            $suffix++
        }

        [System.IO.File]::Copy($TemplatePath, $outputPath, $true)

        $templateWorkbook = $excel.Workbooks.Open($outputPath)
        $sheet = $templateWorkbook.Worksheets.Item(1)

        $kika = [char]0x8CB4 + [char]0x4E0B
        Set-CellText $sheet 'A4' ($companyName + $kika)

        if ($dateSerial -gt 0) {
            $sheet.Range('A2').Value2 = $dateSerial
        }

        Set-CellText   $sheet 'A9'  $itemName
        Set-CellNumber $sheet 'B9'  $pages
        Set-CellNumber $sheet 'C9'  $copies
        Set-CellNumber $sheet 'E9'  $unitPrice
        Set-CellNumber $sheet 'F10' $coverCost
        Set-CellNumber $sheet 'F11' $freight

        Set-CellText $sheet 'G9'  ([char]0xB0B4+[char]0xC6A9+' '+[char]0xC778+[char]0xC1C4+' '+[char]0xBC0F+' '+[char]0xC7AC+[char]0xB2E8)
        Set-CellText $sheet 'G10' ([char]0xD45C+[char]0xC9C0+' '+[char]0xC778+[char]0xC1C4+' '+[char]0xBC0F+' '+[char]0xC81C+[char]0xBCF8)
        Set-CellText $sheet 'G11' ([char]0xC6B4+[char]0xC784)

        $templateWorkbook.Save()
        $templateWorkbook.Close($false)
        $templateWorkbook = $null

        $items += [ordered]@{
            rowNum      = $rowNum
            filePath    = $outputPath
            fileName    = [System.IO.Path]::GetFileName($outputPath)
            companyName = $companyName
            itemName    = $itemName
            status      = 'success'
        }
    }

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
