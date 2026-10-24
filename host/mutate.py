#!/usr/bin/env python3
"""Mutation check for the host tests.

Applies each mutant in a list to a scratch copy of the repository, one at a time, runs the native
suites (host/run-native.sh) and reports whether the tests caught it. A mutant the plain run misses is
run once more under the sanitizers.

Usage: host/mutate.py <mutants.json> <unity-src-dir> [--markdown]
Each mutant: {"file": ..., "line": N, "before": "<the line as written>", "after": "<the mutated line>"}
"""
import json, os, shutil, subprocess, sys, tempfile, pathlib

repo = pathlib.Path(__file__).resolve().parent.parent
mutants = json.loads(pathlib.Path(sys.argv[1]).read_text())
unity = sys.argv[2]
markdown = '--markdown' in sys.argv

def run(tree, sanitize=False):
    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
    cmd = [str(tree / 'host/run-native.sh'), str(tree), str(tree / 'host/lib'), unity] + (['sanitize'] if sanitize else [])
    return subprocess.run(cmd, env=env, capture_output=True, text=True).returncode == 0

if not run(repo):
    sys.exit('the unmutated tree does not pass; fix that first')

rows = []
for m in mutants:
    with tempfile.TemporaryDirectory() as tmp:
        tree = pathlib.Path(tmp) / 'r'
        shutil.copytree(repo, tree, ignore=shutil.ignore_patterns('.git', '.pio'))
        src = tree / m['file']
        lines = src.read_text().split('\n')
        i = m['line'] - 1
        if lines[i].strip() != m['before'].strip():
            rows.append((m, 'not applied: line differs')); continue
        lines[i] = lines[i].replace(m['before'].strip(), m['after'].strip())
        src.write_text('\n'.join(lines))
        if not run(tree):
            rows.append((m, 'caught'))
        elif not run(tree, sanitize=True):
            rows.append((m, 'caught only by the sanitized run'))
        else:
            rows.append((m, 'survived'))

caught = sum(1 for _, r in rows if r.startswith('caught'))
caught_plain = sum(1 for _, r in rows if r == 'caught')
if markdown:
    print(f'{caught_plain} of {len(rows)} mutants caught by the plain run; {caught} of {len(rows)} with the sanitized run.\n')
    print('| File:line | Mutation | Result |\n| --- | --- | --- |')
    for m, r in rows:
        esc = lambda t: t.strip().replace('|', '\\|')
        print(f"| `{m['file']}:{m['line']}` | `{esc(m['before'])}` → `{esc(m['after'])}` | {r} |")
else:
    for m, r in rows:
        print(f"{m['file']}:{m['line']}: {r}")
    print(f'caught {caught_plain}/{len(rows)} plain, {caught}/{len(rows)} with sanitizers')
