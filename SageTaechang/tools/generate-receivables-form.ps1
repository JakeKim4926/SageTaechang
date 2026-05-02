param(
    [Parameter(Mandatory=$true)][string]$InputPath,
    [Parameter(Mandatory=$true)][string]$TemplatePath,
    [Parameter(Mandatory=$true)][string]$OutputFolder,
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

function Get-DateValue($sheet, $row, $col) {
    $value = $sheet.Cells.Item($row, $col).Value2
    if ($null -ne $value) {
        if ($value -is [datetime]) { return $value }
        try {
            $serial = [double]$value
            if ($serial -gt 0) { return [datetime]::FromOADate($serial) }
        } catch {}
    }

    $text = Get-CellText $sheet $row $col
    $parsed = [datetime]::MinValue
    if ([datetime]::TryParse($text, [ref]$parsed)) { return $parsed }
    return $null
}

function Get-DateText($sheet, $row, $col) {
    $dateValue = Get-DateValue $sheet $row $col
    if ($null -ne $dateValue) { return $dateValue.ToString('yyyy-MM-dd') }
    return Get-CellText $sheet $row $col
}

function Get-NumberValue($value) {
    if ($null -eq $value) { return 0 }
    try { return [int][double]$value } catch { return 0 }
}

function Normalize-CompanyName($value) {
    $text = (ConvertTo-TextValue $value).Trim()
    $text = [regex]::Replace($text, '^\s*\d+\s*[\.\)\-\:_]?\s*', '')
    return $text.Trim()
}

function Find-Worksheet($workbook, $name) {
    foreach ($sheet in $workbook.Worksheets) {
        if ($sheet.Name -eq $name) { return $sheet }
    }
    return $null
}

function Build-PriorityMap($sheet) {
    $map = @{}
    if ($null -eq $sheet) { return $map }

    $used = $sheet.UsedRange
    $rowCount = $used.Rows.Count
    $colCount = $used.Columns.Count

    for ($row = 1; $row -le $rowCount; $row++) {
        $priority = 0
        $companyName = ''

        for ($col = 1; $col -le $colCount; $col++) {
            $text = Get-CellText $sheet $row $col
            if ($text.Trim().Length -eq 0) { continue }

            if ($priority -le 0) {
                $match = [regex]::Match($text, '^\s*(\d+)')
                if ($match.Success) {
                    $priority = [int]$match.Groups[1].Value
                    $candidate = Normalize-CompanyName $text
                    if ($candidate.Length -gt 0 -and $candidate -ne $text.Trim()) {
                        $companyName = $candidate
                    }
                    continue
                }
            }

            if ($companyName.Length -eq 0) {
                $candidate = Normalize-CompanyName $text
                if ($candidate.Length -gt 0) {
                    $companyName = $candidate
                }
            }
        }

        if ($priority -gt 0 -and $companyName.Length -gt 0 -and -not $map.ContainsKey($companyName)) {
            $map[$companyName] = $priority
        }
    }

    return $map
}

function Safe-FileName($value) {
    $name = (ConvertTo-TextValue $value).Trim()
    if ($name.Length -eq 0) { $name = 'receivables' }
    $invalid = [System.IO.Path]::GetInvalidFileNameChars()
    foreach ($ch in $invalid) {
        $name = $name.Replace([string]$ch, '_')
    }
    $name = [regex]::Replace($name, '\.{2,}', '.').Trim().TrimEnd([char]'.')
    if ($name.Length -eq 0) { $name = 'receivables' }
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

function Copy-RowFormat($sheet, $sourceRow, $targetRow) {
    $sheet.Rows.Item($sourceRow).Copy() | Out-Null
    $sheet.Rows.Item($targetRow).PasteSpecial(-4122) | Out-Null
    $sheet.Application.CutCopyMode = $false
}

function Clear-OutputRow($sheet, $row) {
    $sheet.Range(('A{0}:K{0}' -f $row)).ClearContents() | Out-Null
}

$excel = $null
$inputWorkbook = $null
$outputWorkbook = $null
$items = @()
$missingCompanies = @{}

try {
    if (-not [System.IO.File]::Exists($InputPath)) { throw "Input file was not found: $InputPath" }
    if (-not [System.IO.File]::Exists($TemplatePath)) { throw "Template file was not found: $TemplatePath" }
    if (-not [System.IO.Directory]::Exists($OutputFolder)) {
        [System.IO.Directory]::CreateDirectory($OutputFolder) | Out-Null
    }

    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false

    $inputWorkbook = $excel.Workbooks.Open($InputPath)
    $taxSheetName = [string][char]0xC138 + [string][char]0xAE08 + [string][char]0xACC4 + [string][char]0xC0B0 + [string][char]0xC11C
    $prioritySheetName = [string][char]0xBC88 + [string][char]0xD638
    $etcName = [string][char]0xAE30 + [string][char]0xD0C0
    $inputSheet = Find-Worksheet $inputWorkbook $taxSheetName
    if ($null -eq $inputSheet) {
        $inputSheet = $inputWorkbook.Worksheets.Item(1)
    }
    $prioritySheet = Find-Worksheet $inputWorkbook $prioritySheetName
    $priorityMap = Build-PriorityMap $prioritySheet

    $xlUp = -4162
    $lastRow = $inputSheet.Cells($inputSheet.Rows.Count, 2).End($xlUp).Row
    if ($lastRow -lt 3) { $lastRow = 2 }

    $rows = @()
    for ($rowNum = 3; $rowNum -le $lastRow; $rowNum++) {
        $companyName = (Get-CellText $inputSheet $rowNum 2).Trim()
        $manager = Get-CellText $inputSheet $rowNum 3
        $issueDateValue = Get-DateValue $inputSheet $rowNum 4
        $issueDateText = Get-DateText $inputSheet $rowNum 4
        $itemName = Get-CellText $inputSheet $rowNum 5
        $issueType = Get-CellText $inputSheet $rowNum 6
        $totalAmount = $inputSheet.Cells.Item($rowNum, 7).Value2
        $totalAmountText = ConvertTo-TextValue $totalAmount
        $bankName = Get-CellText $inputSheet $rowNum 10

        if ($companyName.Trim().Length -eq 0 -and $itemName.Trim().Length -eq 0) { continue }

        $priority = 39
        $priorityName = $etcName
        if ($priorityMap.ContainsKey($companyName)) {
            $priority = [int]$priorityMap[$companyName]
            $priorityName = $companyName
        } elseif ($companyName.Length -gt 0) {
            $missingCompanies[$companyName] = $true
        }

        $rows += [ordered]@{
            rowNum = $rowNum
            priority = $priority
            priorityName = $priorityName
            issueDateValue = $issueDateValue
            issueDateText = $issueDateText
            manager = $manager
            companyName = $companyName
            totalAmount = $totalAmount
            totalAmountText = $totalAmountText
            issueType = $issueType
            itemName = $itemName
            bankName = $bankName
        }
    }

    $rows = @($rows | Sort-Object @{ Expression = { $_.priority }; Ascending = $true }, @{ Expression = { $_.manager }; Ascending = $true }, @{ Expression = { $_.companyName }; Ascending = $true })
    if ($rows.Count -eq 0) {
        throw 'Input file has no data rows.'
    }

    $titleDate = $rows[0].issueDateValue
    if ($null -eq $titleDate) {
        throw 'Issue date was not found.'
    }

    $baseName = 'receivables_' + $titleDate.ToString('yyyyMM')
    $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '.xls')
    $suffix = 1
    while ([System.IO.File]::Exists($outputPath)) {
        $outputPath = [System.IO.Path]::Combine($OutputFolder, $baseName + '_' + $suffix + '.xls')
        $suffix++
    }

    [System.IO.File]::Copy($TemplatePath, $outputPath, $true)

    $outputWorkbook = $excel.Workbooks.Open($outputPath)
    $sheet = $outputWorkbook.Worksheets.Item(1)

    $yearMarker = [string][char]0xB144
    $monthMarker = [string][char]0xC6D4
    $titleSuffix = [string][char]0xBBF8 + [string][char]0xC218 + [string][char]0xAE08 + ' ' + [string][char]0xB0B4 + [string][char]0xC5ED + [string][char]0xC11C
    Set-CellText $sheet 'A1' ($titleDate.ToString('yyyy') + $yearMarker + ' ' + $titleDate.Month + $monthMarker + ' ' + $titleSuffix)

    $targetRow = 5
    $previousCompanyName = $null
    $writtenDataRows = 0
    $writtenSeparatorRows = 0

    foreach ($row in $rows) {
        if ($null -ne $previousCompanyName -and $row.companyName -ne $previousCompanyName) {
            Copy-RowFormat $sheet 8 $targetRow
            Clear-OutputRow $sheet $targetRow
            foreach ($col in 2..11) {
                $sheet.Cells.Item($targetRow, $col).Value2 = '-'
            }
            $targetRow++
            $writtenSeparatorRows++
        }

        Copy-RowFormat $sheet 5 $targetRow
        Clear-OutputRow $sheet $targetRow

        Set-CellText $sheet "B$targetRow" $row.companyName
        Set-CellText $sheet "C$targetRow" $row.manager
        Set-CellText $sheet "D$targetRow" $row.issueDateText
        Set-CellText $sheet "E$targetRow" $row.itemName
        Set-CellText $sheet "F$targetRow" $row.issueType
        Set-CellNumber $sheet "G$targetRow" $row.totalAmount
        Set-CellText $sheet "J$targetRow" $row.bankName

        $previousCompanyName = $row.companyName
        $targetRow++
        $writtenDataRows++
    }

    $outputWorkbook.SaveAs($outputPath, 56)
    $outputWorkbook.Close($false)
    $outputWorkbook = $null

    $inputWorkbook.Close($false)
    $inputWorkbook = $null

    $items += [ordered]@{
        filePath = $outputPath
        fileName = [System.IO.Path]::GetFileName($outputPath)
        status = 'success'
    }

    $result = [ordered]@{
        status = 'success'
        generatedCount = 1
        rowCount = $writtenDataRows
        separatorCount = $writtenSeparatorRows
        missingCompanies = @($missingCompanies.Keys | Sort-Object)
        outputFolder = $OutputFolder
        files = $items
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
}
catch {
    if ($outputWorkbook -ne $null) { try { $outputWorkbook.Close($false) } catch {} }
    if ($inputWorkbook -ne $null) { try { $inputWorkbook.Close($false) } catch {} }
    $result = [ordered]@{
        status = 'error'
        message = $_.Exception.Message
        generatedCount = 0
        outputFolder = $OutputFolder
        files = $items
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
    exit 1
}
finally {
    if ($excel -ne $null) { try { $excel.Quit() } catch {} }
}
