param(
    [Parameter(Mandatory=$true)][string]$TemplatePath,
    [Parameter(Mandatory=$true)][string]$OutputFolder,
    [Parameter(Mandatory=$true)][string]$ResultPath,
    [Parameter(Mandatory=$true)][string]$CompanyName,
    [Parameter(Mandatory=$true)][string]$DateText,
    [Parameter(Mandatory=$true)][string]$ItemName,
    [Parameter(Mandatory=$true)][int]$Copies,
    [Parameter(Mandatory=$true)][int]$Pages,
    [Parameter(Mandatory=$true)][int]$UnitPrice,
    [Parameter(Mandatory=$true)][int]$CoverCost,
    [Parameter(Mandatory=$false)][int]$Freight = 0
)

$ErrorActionPreference = 'Stop'

# Korean string constants (Unicode escapes to avoid encoding issues)
$KR_ESTIMATE   = '_' + [char]0xACAC + [char]0xC801 + [char]0xC11C + '_'
$KR_KIKA       = [char]0x8CB4 + [char]0x4E0B
$KR_BODY_PRINT = [char]0xB0B4 + [char]0xC6A9 + ' ' + [char]0xC778 + [char]0xC1C4 + ' ' + [char]0xBC0F + ' ' + [char]0xC7AC + [char]0xB2E8
$KR_COVER      = [char]0xD45C + [char]0xC9C0 + ' ' + [char]0xC778 + [char]0xC1C4 + ' ' + [char]0xBC0F + ' ' + [char]0xC81C + [char]0xBCF8
$KR_FREIGHT    = [char]0xC6B4 + [char]0xC784

function ConvertTo-TextValue($value) {
    if ($null -eq $value) { return '' }
    return [string]$value
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

function Safe-FileName($value) {
    $name = (ConvertTo-TextValue $value).Trim()
    if ($name.Length -eq 0) { return 'estimate' }
    $name = [regex]::Replace($name, '[\\/:*?"<>|\x00-\x1F]', '_')
    $name = [regex]::Replace($name, '_+', '_')
    $name = $name.Trim('_').Trim()
    if ($name.Length -eq 0) { return 'estimate' }
    return $name
}

$excel    = $null
$workbook = $null

try {
    if (-not [System.IO.File]::Exists($TemplatePath)) {
        throw "Template file was not found: $TemplatePath"
    }
    if (-not [System.IO.Directory]::Exists($OutputFolder)) {
        [System.IO.Directory]::CreateDirectory($OutputFolder) | Out-Null
    }

    $dateSerial = 0
    if ($DateText.Trim().Length -gt 0) {
        $parsedDate = [datetime]::ParseExact(
            $DateText.Trim(), 'yyyy-MM-dd',
            [System.Globalization.CultureInfo]::InvariantCulture)
        $dateSerial = [int][math]::Floor($parsedDate.ToOADate())
    }

    $safeName = Safe-FileName $CompanyName
    $timeStr  = (Get-Date).ToString('HHmmssfff')
    if ($dateSerial -gt 0) {
        $dateStr  = [datetime]::FromOADate($dateSerial).ToString('yyyyMMdd')
        $baseName = $safeName + $KR_ESTIMATE + $dateStr + '_' + $timeStr
    } else {
        $baseName = $safeName + $KR_ESTIMATE + $timeStr
    }

    $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '.xlsx')
    $suffix = 1
    while ([System.IO.File]::Exists($outputPath)) {
        $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '_' + $suffix + '.xlsx')
        $suffix++
    }

    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false
    try { $excel.ScreenUpdating = $false } catch {}
    try { $excel.EnableEvents = $false } catch {}
    try { $excel.Calculation = -4135 } catch {}

    $workbook = $excel.Workbooks.Open($TemplatePath)
    $sheet    = $workbook.Worksheets.Item(1)

    Set-CellText $sheet 'A4' ($CompanyName + $KR_KIKA)

    if ($dateSerial -gt 0) {
        $sheet.Range('A2').Value2 = $dateSerial
    } else {
        $sheet.Range('A2').Value2 = ''
    }

    Set-CellText   $sheet 'A9'  $ItemName
    Set-CellNumber $sheet 'B9'  $Pages
    Set-CellNumber $sheet 'C9'  $Copies
    Set-CellNumber $sheet 'E9'  $UnitPrice
    Set-CellNumber $sheet 'F10' $CoverCost

    if ($Freight -gt 0) {
        Set-CellNumber $sheet 'F11' $Freight
        Set-CellText   $sheet 'G11' $KR_FREIGHT
    } else {
        Set-CellText $sheet 'F11' ''
        Set-CellText $sheet 'G11' ''
    }

    Set-CellText $sheet 'G9'  $KR_BODY_PRINT
    Set-CellText $sheet 'G10' $KR_COVER

    $workbook.SaveCopyAs($outputPath)
    $workbook.Close($false)
    $workbook = $null

    $result = [ordered]@{
        status      = 'success'
        filePath    = $outputPath
        fileName    = [System.IO.Path]::GetFileName($outputPath)
        companyName = $CompanyName
        itemName    = $ItemName
    }
    $result | ConvertTo-Json -Depth 3 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
}
catch {
    if ($workbook -ne $null) { try { $workbook.Close($false) } catch {} }
    $result = [ordered]@{
        status  = 'error'
        message = $_.Exception.Message
    }
    $result | ConvertTo-Json -Depth 3 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
    exit 1
}
finally {
    if ($excel -ne $null) { try { $excel.Quit() } catch {} }
}
