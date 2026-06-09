#!/usr/bin/env pwsh
# Build script for fastGHD utilities
# Usage: .\build.ps1 [clean|all|tests|hypertree_test|fec_test|tree_decomp_brute]

param(
    [ValidateSet("clean", "all", "tests", "hypertree_test", "fec_test", "tree_decomp_brute")]
    [string]$Target = "all"
)

$CXX = "g++"
$CXXFLAGS = @("-std=c++17", "-Wall", "-Wextra", "-I../includes", "-g", "-O3")

$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $scriptPath

# Define executables and their sources
$targets = @{
    "hypertree_test" = @{
        exe = "hypertree_test.exe"
        sources = @("../test/hypergraphs/hypertree_check_test.cpp", "../src/hypergraphs/hypertree_check.cpp")
    }
    "fec_test" = @{
        exe = "fec_test.exe"
        sources = @("../test/fractional_edge_cover/fractional_edge_cover_test.cpp", "../src/fractional_edge_cover/fractional_edge_cover_solver.cpp")
    }
    "tree_decomp_brute" = @{
        exe = "tree_decomp_brute.exe"
        sources = @("../src/tree_decomp_brute/tree_decomp_brute.cpp", "../src/hypergraphs/hypertree_check.cpp", "../src/fractional_edge_cover/fractional_edge_cover_solver.cpp")
    }
}

function Build-Target {
    param([string]$targetName)

    $target = $targets[$targetName]
    if (-not $target) {
        Write-Host "Unknown target: $targetName" -ForegroundColor Red
        return $false
    }

    $exe = $target.exe
    $sources = $target.sources

    Write-Host "Building $targetName -> $exe..." -ForegroundColor Cyan

    $compileCmd = @($CXX) + $CXXFLAGS + $sources + @("-o", $exe)

    & $CXX @CXXFLAGS @sources -o $exe

    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ $exe built successfully" -ForegroundColor Green
        return $true
    } else {
        Write-Host "✗ Failed to build $exe" -ForegroundColor Red
        return $false
    }
}

function Clean {
    Write-Host "Cleaning..." -ForegroundColor Cyan
    $targets.Keys | ForEach-Object {
        $exe = $targets[$_].exe
        if (Test-Path $exe) {
            Remove-Item $exe -Force
            Write-Host "✓ Removed $exe" -ForegroundColor Yellow
        }
    }
}

# Execute target
switch ($Target) {
    "clean" {
        Clean
    }
    "all" {
        foreach ($t in $targets.Keys) {
            Build-Target $t
            if ($LASTEXITCODE -ne 0) { break }
        }
    }
    "tests" {
        Build-Target "hypertree_test"
        Build-Target "fec_test"
    }
    default {
        Build-Target $Target
    }
}

Pop-Location

