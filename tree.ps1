function Get-SourceTree {
    param([string]$Path = ".")
    
    $excludeDirs = @("build", ".git", "devlog", "doc", "Debug", "Release", "x64", "CMakeFiles", "ALL_BUILD", "ZERO_CHECK")
    $excludeFiles = @("\.(obj|pdb|ilk|exe|lib|recipe|log|lastbuildstate|tlog|vcxproj|user|filters|sln|stamp|rule|bin)$", "^tree\.ps1$", "^lang_stats\.py$")
    
    function Show-Branch($items, $prefix = "") {
        $dirs = $items | Where-Object { $_.PSIsContainer }
        $files = $items | Where-Object { -not $_.PSIsContainer }
        $all = @($dirs) + @($files)
        
        for ($i = 0; $i -lt $all.Count; $i++) {
            $isLast = ($i -eq $all.Count - 1)
            $item = $all[$i]
            # 改用 ASCII 字符
            $line = if ($isLast) { "`-- " } else { "|-- " }
            
            Write-Host "$prefix$line" -NoNewline
            if ($item.PSIsContainer) { 
                Write-Host $item.Name -ForegroundColor Cyan
                $subPrefix = if ($isLast) { "$prefix    " } else { "$prefix|   " }
                $subItems = Get-ChildItem -LiteralPath $item.FullName -Force | Where-Object {
                    $child = $_
                    $dirMatch = $excludeDirs | Where-Object { $child.Name -like $_ }
                    $fileMatch = $excludeFiles | Where-Object { $child.Name -match $_ }
                    -not ($dirMatch -or $fileMatch)
                } | Sort-Object { -not $_.PSIsContainer }, Name
                if ($subItems) { Show-Branch $subItems $subPrefix }
            } else {
                Write-Host $item.Name
            }
        }
    }
    
    $root = Get-Item $Path
    Write-Host $root.Name -ForegroundColor Green
    $items = Get-ChildItem -LiteralPath $root.FullName -Force | Where-Object {
        $child = $_
        $dirMatch = $excludeDirs | Where-Object { $child.Name -like $_ }
        $fileMatch = $excludeFiles | Where-Object { $child.Name -match $_ }
        -not ($dirMatch -or $fileMatch)
    } | Sort-Object { -not $_.PSIsContainer }, Name
    Show-Branch $items
}

Get-SourceTree
Write-Host ""
python .\lang_stats.py