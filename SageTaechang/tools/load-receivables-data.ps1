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

function Get-MatrixValue($values, $row, $col) {
    try { return $values[$row, $col] } catch { return $null }
}

function ConvertTo-DateTextValue($value) {
    if ($null -ne $value) {
        if ($value -is [datetime]) { return $value.ToString('yyyy-MM-dd') }
        try {
            $serial = [double]$value
            if ($serial -gt 0) { return [datetime]::FromOADate($serial).ToString('yyyy-MM-dd') }
        } catch {}
    }
    return ConvertTo-TextValue $value
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

function Get-CompanyKey($value) {
    $text = Normalize-CompanyName $value
    return [regex]::Replace($text, '\s+', '')
}

function Get-ManagerSortKey($value) {
    $text = (ConvertTo-TextValue $value).Trim()
    $prefix = '1'
    if ($text -match '^\d+') {
        $prefix = '0'
    }
    $key = [regex]::Replace($text, '\d+', {
        param($match)
        return ('{0:D10}' -f [int64]$match.Value)
    })
    return $prefix + '|' + $key
}

function Add-PriorityItem($map, $priority, $companyName) {
    $key = Get-CompanyKey $companyName
    if ($key.Length -gt 0 -and -not $map.ContainsKey($key)) {
        $map[$key] = [ordered]@{
            priority = $priority
            companyName = $companyName
        }
    }
}

function Build-DefaultPriorityMap() {
    $map = @{}
    $items = '[{"p":1,"n":"\uc0c8\ube5b\ud68c\uacc4\ubc95\uc778"},{"p":2,"n":"\uc81c\uc6d0\ud68c\uacc4\ubc95\uc778"},{"p":3,"n":"\uc11c\ub9b0\ud68c\uacc4\ubc95\uc778"},{"p":4,"n":"\ud68c\uacc4\ubc95\uc778 \uc624\ud604"},{"p":5,"n":"\ud0dc\uc77c\ud68c\uacc4\ubc95\uc778"},{"p":6,"n":"\uc2e0\uc815\ud68c\uacc4\ubc95\uc778"},{"p":7,"n":"\ub3c4\uc6d0\ud68c\uacc4\ubc95\uc778"},{"p":8,"n":"\uc778\uc138\ud68c\uacc4\ubc95\uc778"},{"p":9,"n":"\uc138\ub355\ud68c\uacc4\ubc95\uc778"},{"p":10,"n":"\ud55c\uc131\ud68c\uacc4\ubc95\uc778"},{"p":12,"n":"\ucc9c\uc77c\uacf5\uc778\ud68c\uacc4\uc0ac\uac10\uc0ac\ubc18"},{"p":13,"n":"\uc774\uc0b0\ud68c\uacc4\ubc95\uc778"},{"p":14,"n":"\ud0dc\uc728\ud68c\uacc4\ubc95\uc778"},{"p":15,"n":"\ud68c\uacc4\ubc95\uc778 \uc608\uc6d0"},{"p":16,"n":"\ud638\uc5f0\ud68c\uacc4\ubc95\uc778"},{"p":17,"n":"\uc624\ub298\ud68c\uacc4\ubc95\uc778"},{"p":18,"n":"\ub300\ub95c\ud68c\uacc4\ubc95\uc778"},{"p":19,"n":"\ub3d9\uc5f0\ud68c\uacc4\ubc95\uc778"},{"p":20,"n":"\uc601\uc564\uc9c4\uc138\ubb34\ubc95\uc778"},{"p":21,"n":"\ub2e4\uc6b8\uacf5\uc778\ud68c\uacc4\uc0ac\uac10\uc0ac\ubc18"},{"p":22,"n":"\ud68c\uacc4\ubc95\uc778 \ucc3d\ucc9c"},{"p":23,"n":"\ud68c\uacc4\ubc95\uc778 \uc138\uc9c4"},{"p":24,"n":"\uc218\uc778\ud68c\uacc4\ubc95\uc778"},{"p":25,"n":"\ub098\uc6b0\ud68c\uacc4\ubc95\uc778"},{"p":26,"n":"\uc9c0\uc6b0\ud68c\uacc4\ubc95\uc778"},{"p":27,"n":"\ud68c\uacc4\ubc95\uc778 \ub3d9\ud589"},{"p":28,"n":"\ud0dc\uc778\ud68c\uacc4\ubc95\uc778"},{"p":29,"n":"\ud558\ub298\ud68c\uacc4\ubc95\uc778"},{"p":30,"n":"\ub3c4\uc601\ud68c\uacc4\ubc95\uc778"},{"p":31,"n":"\uc2a4\ud0c0\ub9ac\uce58 \uc138\ubb34\uadf8\ub8f9"},{"p":32,"n":"\uc911\uc815\ud68c\uacc4\ubc95\uc778"},{"p":33,"n":"\uc2e0\ud55c\ud68c\uacc4\ubc95\uc778"},{"p":34,"n":"\uc9c4\uc0b0\ud68c\uacc4\ubc95\uc778"},{"p":35,"n":"\uc544\uc131\ud68c\uacc4\ubc95\uc778"},{"p":36,"n":"\uc6b0\ub9ac\ud68c\uacc4\ubc95\uc778"},{"p":37,"n":"(\uc8fc)\ub9dd\uace0\ubd80\uc2a4\ud2b8"},{"p":38,"n":"(\uc7ac)\ud55c\uad6d\ub85c\ub0a0\ub4dc\ub9e5\ub3c4\ub0a0\ub4dc\ud558\uc6b0\uc2a4"},{"p":39,"n":"\uae30\ud0c0"},{"p":40,"n":"\uc0bc\ub355\ud68c\uacc4\ubc95\uc778"},{"p":41,"n":"\ub2e4\uc0b0\ud68c\uacc4\ubc95\uc778"},{"p":42,"n":"\ub3d9\uc131\ud68c\uacc4\ubc95\uc778"},{"p":43,"n":"\ub300\ud604\ud68c\uacc4\ubc95\uc778"},{"p":44,"n":"\uc9c0\uc778\ud68c\uacc4\ubc95\uc778"},{"p":45,"n":"\uc131\ud604\ud68c\uacc4\ubc95\uc778"},{"p":46,"n":"\ubbfc\uc6b0\uc138\ubb34\ubc95\uc778"},{"p":47,"n":"\uad11\uad50\uc138\ubb34\ubc95\uc778"},{"p":48,"n":"\uc138\ubb34\ubc95\uc778 \ub2e4\uc6b0"},{"p":49,"n":"\uc11c\ud604\ud68c\uacc4\ubc95\uc778"},{"p":50,"n":"\uc138\ubb34\ubc95\uc778 \uc13c\ud2b8\ub9ad"},{"p":51,"n":"\uc774\uc815\ud68c\uacc4\ubc95\uc778"},{"p":52,"n":"\ud55c\uae38\ud68c\uacc4\ubc95\uc778"},{"p":53,"n":"\uc6b0\ub355\ud68c\uacc4\ubc95\uc778"},{"p":54,"n":"\uc608\uc6d0\uc138\ubb34\ubc95\uc778"},{"p":55,"n":"\uc138\ubb34\ubc95\uc778 \ud654\uc6b0"},{"p":56,"n":"\ubc95\ubb34\ubc95\uc778 \ub450\ud604"},{"p":57,"n":"\uc138\ubb34\ubc95\uc778 \uc774\ub2f4"},{"p":58,"n":"\uc138\ubb34\ud68c\uacc4 \uc778\uc131"},{"p":59,"n":"\ud55c\uacbd\ud68c\uacc4\ubc95\uc778"},{"p":60,"n":"\uc2e0\ud654\ud68c\uacc4\ubc95\uc778"},{"p":61,"n":"\uc774\uc9c0\ud68c\uacc4\ubc95\uc778"},{"p":62,"n":"\uc0bc\uc6b0\ud68c\uacc4\ubc95\uc778"},{"p":63,"n":"\uc2e0\uc601\ud68c\uacc4\ubc95\uc778"},{"p":64,"n":"\ud55c\uc2e0\ud68c\uacc4\ubc95\uc778"},{"p":65,"n":"\uc77c\uc2e0\ud68c\uacc4\ubc95\uc778"},{"p":66,"n":"\uc77c\uc2e0\uc138\ubb34\ud68c\uacc4\uc0ac\ubb34\uc18c"}]' | ConvertFrom-Json
    foreach ($item in $items) {
        Add-PriorityItem $map ([int]$item.p) $item.n
    }
    return $map
}

function Find-Worksheet($workbook, $name) {
    foreach ($sheet in $workbook.Worksheets) {
        if ($sheet.Name -eq $name) { return $sheet }
    }
    return $null
}

function Build-PriorityMap($sheet) {
    $map = Build-DefaultPriorityMap
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

        $companyKey = Get-CompanyKey $companyName
        if ($priority -gt 0 -and $companyKey.Length -gt 0) {
            $map[$companyKey] = [ordered]@{
                priority = $priority
                companyName = $companyName
            }
        }
    }

    return $map
}

$excel = $null
$inputWorkbook = $null
$rows = New-Object System.Collections.ArrayList
$missingCompanies = @{}

try {
    if (-not [System.IO.File]::Exists($InputPath)) {
        throw 'Input file was not found.'
    }

    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false
    try { $excel.ScreenUpdating = $false } catch {}
    try { $excel.EnableEvents = $false } catch {}
    try { $excel.Calculation = -4135 } catch {}

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
    $inputValues = $inputSheet.Range(('A1:K{0}' -f $lastRow)).Value2

    for ($rowNum = 3; $rowNum -le $lastRow; $rowNum++) {
        $companyName = (ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 2)).Trim()
        $manager = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 3)
        $issueDate = ConvertTo-DateTextValue (Get-MatrixValue $inputValues $rowNum 4)
        $itemName = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 5)
        $issueType = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 6)
        $totalAmount = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 7)
        $bankName = ConvertTo-TextValue (Get-MatrixValue $inputValues $rowNum 10)

        if ($companyName.Trim().Length -eq 0 -and $itemName.Trim().Length -eq 0) { continue }

        $companyKey = Get-CompanyKey $companyName
        $etcKey = Get-CompanyKey $etcName
        $priority = [int]$priorityMap[$etcKey].priority
        $priorityName = $etcName
        $displayCompanyName = $etcName
        if ($priorityMap.ContainsKey($companyKey)) {
            $priority = [int]$priorityMap[$companyKey].priority
            $priorityName = $priorityMap[$companyKey].companyName
            $displayCompanyName = $priorityName
        } elseif ($companyName.Length -gt 0) {
            $missingCompanies[$companyName] = $true
        }

        [void]$rows.Add([ordered]@{
            rowNum = $rowNum
            priority = $priority
            priorityName = $priorityName
            issueDate = $issueDate
            manager = $manager
            managerSortKey = Get-ManagerSortKey $manager
            companyName = $displayCompanyName
            totalAmount = $totalAmount
            issueType = $issueType
            itemName = $itemName
            bankName = $bankName
        })
    }

    $rows = @($rows | Sort-Object @{ Expression = { $_.priority }; Ascending = $true }, @{ Expression = { $_.managerSortKey }; Ascending = $true }, @{ Expression = { $_.companyName }; Ascending = $true })
    foreach ($row in $rows) {
        if ($row.Contains('managerSortKey')) {
            $row.Remove('managerSortKey')
        }
    }

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
