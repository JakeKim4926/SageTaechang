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

function Get-DateText($sheet, $row, $col) {
    $value = $sheet.Cells.Item($row, $col).Value2
    if ($null -ne $value) {
        if ($value -is [datetime]) { return $value.ToString('yyyy-MM-dd') }
        try {
            $serial = [double]$value
            if ($serial -gt 0) { return [datetime]::FromOADate($serial).ToString('yyyy-MM-dd') }
        } catch {}
    }
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

$excel = $null
$inputWorkbook = $null
$rows = @()
$missingCompanies = @{}

try {
    if (-not [System.IO.File]::Exists($InputPath)) {
        throw 'Input file was not found.'
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
    if ($lastRow -lt 3) {
        $lastRow = 2
    }

    for ($rowNum = 3; $rowNum -le $lastRow; $rowNum++) {
        $companyName = (Get-CellText $inputSheet $rowNum 2).Trim()
        $manager = Get-CellText $inputSheet $rowNum 3
        $issueDate = Get-DateText $inputSheet $rowNum 4
        $itemName = Get-CellText $inputSheet $rowNum 5
        $issueType = Get-CellText $inputSheet $rowNum 6
        $totalAmount = ConvertTo-TextValue $inputSheet.Cells.Item($rowNum, 7).Value2
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
            issueDate = $issueDate
            manager = $manager
            companyName = $companyName
            totalAmount = $totalAmount
            issueType = $issueType
            itemName = $itemName
            bankName = $bankName
        }
    }

    $rows = @($rows | Sort-Object @{ Expression = { $_.priority }; Ascending = $true }, @{ Expression = { $_.manager }; Ascending = $true }, @{ Expression = { $_.companyName }; Ascending = $true })

    $inputWorkbook.Close($false)
    $inputWorkbook = $null

    $result = [ordered]@{
        status = 'success'
        rowCount = $rows.Count
        missingCompanies = @($missingCompanies.Keys | Sort-Object)
        rows = $rows
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
}
catch {
    if ($inputWorkbook -ne $null) { try { $inputWorkbook.Close($false) } catch {} }
    $result = [ordered]@{
        status = 'error'
        message = $_.Exception.Message
        rows = @()
    }
    $result | ConvertTo-Json -Depth 5 -Compress | Set-Content -LiteralPath $ResultPath -Encoding UTF8
    exit 1
}
finally {
    if ($excel -ne $null) { try { $excel.Quit() } catch {} }
}
