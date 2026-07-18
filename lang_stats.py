import os
import re
from pathlib import Path
from collections import defaultdict

REPO_PATH = os.path.dirname(os.path.abspath(__file__))

LANGUAGE_MAP = {
    '.cpp': 'C++',
    '.c': 'C',
    '.h': 'C++',
    '.hpp': 'C++',
    '.py': 'Python',
    '.js': 'JavaScript',
    '.java': 'Java',
    '.go': 'Go',
    '.rs': 'Rust',
    '.swift': 'Swift',
    '.cs': 'C#',
    '.ts': 'TypeScript',
}


IGNORED_DIRS = {'.git', 'build', 'out', 'bin', 'obj', 'doc', 'devlog', 'node_modules', '.vs', '3rdparty'}
IGNORED_FILES = {'.gitignore', 'CMakeLists.txt', 'Makefile', 'LICENSE', 'README.md', 'General.md', 'lang_stats.py', 'tree.ps1'}
IGNORED_EXTS = {'.md', '.txt', '.json', '.xml', '.yaml', '.yml', '.ini', '.cfg', '.bat', '.sh'}


def is_code_file(filepath: Path) -> tuple[bool, str]:
    name = filepath.name
    suffix = filepath.suffix.lower()
    
    if name in IGNORED_FILES:
        return False, ""
    
    if suffix in IGNORED_EXTS:
        return False, ""
    
    if name.startswith('.'):
        return False, ""
    
    if suffix in LANGUAGE_MAP:
        return True, LANGUAGE_MAP[suffix]
    
    return False, ""

def count_effective_lines(filepath: Path) -> int:
    effective = 0
    in_block_comment = False
    
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                original = line
                stripped = line.strip()
                
                if not stripped:
                    continue
                
                if in_block_comment:
                    if '*/' in stripped:
                        in_block_comment = False
                        after = stripped.split('*/', 1)[1].strip()
                        if after and not after.startswith('//'):
                            effective += 1
                    continue
                
                if stripped.startswith('//'):
                    continue

                if stripped.startswith('/*'):
                    if '*/' in stripped:
                        after = stripped.split('*/', 1)[1].strip()
                        if after and not after.startswith('//'):
                            effective += 1
                    else:
                        in_block_comment = True
                    continue
                
                code_part = stripped.split('//')[0].strip()
                if code_part:
                    effective += 1
                    
    except Exception as e:
        print(f"警告: 无法读取 {filepath}: {e}")
        return 0
    
    return effective

def scan_repo(repo_path: str):
    stats = defaultdict(lambda: {'files': 0, 'lines': 0})
    total_lines = 0
    
    for root, dirs, files in os.walk(repo_path):
        dirs[:] = [d for d in dirs if d not in IGNORED_DIRS]
        
        for filename in files:
            filepath = Path(root) / filename
            
            is_code, lang = is_code_file(filepath)
            if not is_code:
                continue
            
            lines = count_effective_lines(filepath)
            
            stats[lang]['files'] += 1
            stats[lang]['lines'] += lines
            total_lines += lines
    
    return stats, total_lines

def print_bar(percentage: float, width: int = 30) -> str:
    filled = int(width * percentage / 100)
    bar = '█' * filled + '░' * (width - filled)
    return bar

def main():
    if not os.path.exists(REPO_PATH):
        print(f"错误: 路径 {REPO_PATH} 不存在")
        return
    
    print(f"🔍 正在扫描仓库: {os.path.abspath(REPO_PATH)}\n")
    
    stats, total_lines = scan_repo(REPO_PATH)
    
    if total_lines == 0:
        print("未找到可统计的源码文件。")
        return
    
    sorted_stats = sorted(stats.items(), key=lambda x: x[1]['lines'], reverse=True)
    
    print("=" * 70)
    print(f"{'语言':<20} {'文件数':>8} {'有效代码行':>12} {'占比':>8} {'可视化':>30}")
    print("-" * 70)
    
    for lang, data in sorted_stats:
        lines = data['lines']
        files = data['files']
        pct = (lines / total_lines) * 100
        bar = print_bar(pct)
        
        print(f"{lang:<20} {files:>8} {lines:>12,} {pct:>7.1f}% {bar}")
    
    print("=" * 70)
    print(f"{'总计':<20} {sum(d['files'] for d in stats.values()):>8} {total_lines:>12,} {'100.0%':>8}")
    
    print("\n" + "编程语言占比:")
    bar_parts = []
    colors = {
        'C++': '\033[34m',      # 蓝
        'C': '\033[36m',        # 青
        'C/C++ Header': '\033[35m', # 紫
        'Python': '\033[33m',   # 黄
        'JavaScript': '\033[93m', # 亮黄
    }
    reset = '\033[0m'
    
    for lang, data in sorted_stats:
        pct = (data['lines'] / total_lines) * 100
        if pct < 0.5:  # 太小的不显示
            continue
        color = colors.get(lang, '\033[37m')
        width = max(1, int(pct / 2))  # 缩放
        bar_parts.append(f"{color}{'█' * width}{reset}")
    
    print(''.join(bar_parts))
    
    print("\n图例:")
    for lang, data in sorted_stats:
        pct = (data['lines'] / total_lines) * 100
        if pct < 0.5:
            continue
        color = colors.get(lang, '\033[37m')
        print(f"  {color}●{reset} {lang} {pct:.1f}%")

if __name__ == "__main__":
    main()