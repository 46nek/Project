$files = Get-ChildItem -Path . -Include *.cpp, *.h -Recurse
foreach ($file in $files) {
    if ($file.FullName -match "utf8") { continue }
    $content = Get-Content -Path $file.FullName -Raw
    $content | Out-File -FilePath $file.FullName -Encoding utf8
    Write-Host "Converted $($file.Name) to UTF-8 with BOM"
}
