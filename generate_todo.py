#!/usr/bin/env python3
"""
Parse compiler warnings from build log and generate TODO.md
Tracks progress by comparing original warnings with current build
Files that no longer produce warnings are marked as FIXED
"""

import re
import sys
import os
from collections import defaultdict

def parse_warnings(log_file):
    """Parse warnings from build log"""
    warnings = defaultdict(list)
    
    if not os.path.exists(log_file):
        print(f"Warning: {log_file} not found")
        return warnings
    
    with open(log_file, 'r') as f:
        lines = f.readlines()
    
    for line in lines:
        # Match warning lines like: /path/to/file.cpp:123:45: warning: message [-Wflag]
        match = re.match(r'(/.*?):(\d+):\d+:\s+warning:\s+(.*?)\s+\[(-.*?)\]$', line.strip())
        if match:
            file_path = match.group(1)
            line_num = int(match.group(2))
            message = match.group(3)
            flag = match.group(4)
            
            # Extract just the filename for grouping
            filename = file_path.split('/')[-1]
            
            warnings[filename].append({
                'file_path': file_path,
                'line': line_num,
                'message': message,
                'flag': flag
            })
    
    return warnings

def generate_todo_md(original_warnings, current_warnings, output_file='TODO.md'):
    """Generate TODO.md markdown file with FIXED/PENDING status"""
    
    # Get all unique filenames from original warnings
    all_files = set(original_warnings.keys())
    
    # Determine which files are FIXED (in original but not in current)
    current_files = set(current_warnings.keys())
    fixed_files = all_files - current_files
    pending_files = all_files & current_files
    
    # Calculate totals
    total_original = sum(len(w) for w in original_warnings.values())
    total_pending = sum(len(current_warnings[f]) for f in pending_files)
    total_fixed = total_original - total_pending
    
    # Sort files: pending first (by count desc), then fixed (by count desc)
    sorted_pending = sorted(pending_files, key=lambda x: len(current_warnings[x]), reverse=True)
    sorted_fixed = sorted(fixed_files, key=lambda x: len(original_warnings[x]), reverse=True)
    
    with open(output_file, 'w') as f:
        f.write("# Compiler Warning Elimination TODO\n\n")
        f.write(f"**Original Warnings:** {total_original}\n")
        f.write(f"**Files Affected:** {len(all_files)}\n\n")
        
        f.write("## Progress Summary\n\n")
        f.write("| Status | Count |\n")
        f.write("|--------|-------|\n")
        f.write(f"| ✅ Fixed | {total_fixed} |\n")
        f.write(f"| ⏳ Pending | {total_pending} |\n")
        f.write(f"| **Original Total** | **{total_original}** |\n\n")
        
        if total_pending == 0:
            f.write("🎉 **All warnings have been eliminated!** 🎉\n\n")
        
        f.write("---\n\n")
        
        # Show pending files first
        if sorted_pending:
            f.write("## ⏳ Pending Warnings\n\n")
            
            for filename in sorted_pending:
                file_warnings = current_warnings[filename]
                file_path = file_warnings[0]['file_path']
                count = len(file_warnings)
                
                f.write(f"### {filename} ({count} warnings)\n\n")
                f.write(f"**Path:** `{file_path}`\n\n")
                
                f.write("| Status | Line | Type | Message |\n")
                f.write("|--------|------|------|---------|\n")
                
                for w in file_warnings:
                    line = w['line']
                    flag = w['flag'].replace('-W', '')
                    message = w['message'][:80] + '...' if len(w['message']) > 80 else w['message']
                    
                    f.write(f"| ⏳ PENDING | {line} | `{flag}` | {message} |\n")
                
                f.write("\n---\n\n")
        
        # Show fixed files
        if sorted_fixed:
            f.write("## ✅ Fixed Files\n\n")
            f.write(f"*{len(sorted_fixed)} files with {total_fixed} warnings eliminated*\n\n")
            
            for filename in sorted_fixed:
                file_warnings = original_warnings[filename]
                file_path = file_warnings[0]['file_path']
                count = len(file_warnings)
                
                f.write(f"### {filename} ~~({count} warnings)~~ ✅ FIXED\n\n")
                f.write(f"**Path:** `{file_path}`\n\n")
                
                # Show summary of what was fixed
                flag_counts = defaultdict(int)
                for w in file_warnings:
                    flag_counts[w['flag'].replace('-W', '')] += 1
                
                f.write("| Type | Count |\n")
                f.write("|------|-------|\n")
                for flag, count in sorted(flag_counts.items(), key=lambda x: x[1], reverse=True):
                    f.write(f"| `{flag}` | {count} |\n")
                
                f.write("\n---\n\n")
        
        f.write("## Fix Strategy\n\n")
        f.write("1. **Unused parameters** → Add `(void)param;` casts\n")
        f.write("2. **Unused variables** → Remove or add `(void)var;` if kept for debugging\n")
        f.write("3. **Missing struct initializers** → Add missing fields or use `= {{}}` syntax\n")
        f.write("4. **Sign comparisons** → Add explicit casts\n")
        f.write("5. **Switch statements** → Add `default:` cases\n")
        f.write("6. **Format specifiers** → Fix to correct types\n")
        f.write("7. **Missing braces** → Use `= {{}}` instead of `= {{ 0 }}`\n\n")
        
        f.write("**Rule:** NO pragma suppressions - fix warnings properly with code changes!\n\n")
        
        f.write("## How to Update\n\n")
        f.write("1. Build project: `cd build && make -j8 > /tmp/latest_build.log 2>&1`\n")
        f.write("2. Run script: `python3 generate_todo.py`\n")
        f.write("3. Script compares `/tmp/clean_build.log` (original) with `/tmp/latest_build.log` (current)\n")
        f.write("4. Files with no warnings in current build are automatically marked as ✅ FIXED\n")

if __name__ == '__main__':
    original_log = '/tmp/clean_build.log'
    current_log = '/tmp/final_build.log'
    output_file = '/Volumes/TB4-4Tb/Projects/emulators/github/quaesar-ng/TODO.md'
    
    print(f"Parsing original warnings from {original_log}...")
    original_warnings = parse_warnings(original_log)
    original_total = sum(len(w) for w in original_warnings.values())
    print(f"  Found {original_total} original warnings in {len(original_warnings)} files")
    
    print(f"Parsing current warnings from {current_log}...")
    current_warnings = parse_warnings(current_log)
    current_total = sum(len(w) for w in current_warnings.values())
    print(f"  Found {current_total} current warnings in {len(current_warnings)} files")
    
    print(f"\nGenerating {output_file}...")
    generate_todo_md(original_warnings, current_warnings, output_file)
    
    fixed = original_total - current_total
    print(f"\n✅ TODO.md generated successfully!")
    print(f"   Original: {original_total} warnings")
    print(f"   Current: {current_total} warnings")
    print(f"   Fixed: {fixed} warnings ({fixed/original_total*100:.1f}% eliminated)")
    
    if current_warnings:
        print(f"\nTop 10 files still with warnings:")
        sorted_files = sorted(current_warnings.items(), key=lambda x: len(x[1]), reverse=True)
        for i, (filename, file_warnings) in enumerate(sorted_files[:10], 1):
            print(f"  {i}. {filename}: {len(file_warnings)} warnings")
    else:
        print(f"\n🎉 ALL WARNINGS ELIMINATED! 🎉")
