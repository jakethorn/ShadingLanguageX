"""Verify this skill against a real mxslc build.

  1. Every ```slx code block in the skill's markdown files must compile. A first line
     `// mxslc: <args>` supplies extra command-line arguments.
  2. (optional) Every signature in references/stdlib.md must compile when called with positional
     arguments and again with named arguments.

Usage:
    python check_skill.py --mxslc /path/to/mxslc [--stdlib] [-v]
"""
import argparse
import re
import shlex
import subprocess
import tempfile
from pathlib import Path

SKILL_DIR = Path(__file__).resolve().parent.parent

VALUES = {
    "bool": "true", "int": "1", "float": "0.5",
    "vec2": "vec2{0.5, 0.25}", "vec3": "vec3{0.5, 0.25, 0.75}", "vec4": "vec4{0.5, 0.25, 0.75, 1.0}",
    "color3": "color3{0.5, 0.25, 0.75}", "color4": "color4{0.5, 0.25, 0.75, 1.0}",
    "mat3": "default(mat3)", "mat4": "default(mat4)", "filename": '"a.png"',
}
KNOWN_UNCALLABLE = {"out"}  # `out` is an SLX keyword


def compile_code(mxslc, code, args=()):
    with tempfile.TemporaryDirectory() as d:
        src, dst = Path(d) / "block.mxsl", Path(d) / "block.mtlx"
        src.write_text(code)
        res = subprocess.run([mxslc, str(src), "-o", str(dst), *args], capture_output=True, text=True, cwd=d)
        ok = res.returncode == 0 and dst.exists()
        err = [l for l in (res.stderr + res.stdout).splitlines() if l.strip() and "DEBUG" not in l]
        return ok, err


def check_blocks(mxslc, verbose):
    total = fails = 0
    for md in sorted(SKILL_DIR.rglob("*.md")):
        text = md.read_text()
        for m in re.finditer(r"```slx\n(.*?)```", text, re.S):
            code = m.group(1)
            line = text[:m.start()].count("\n") + 1
            first = code.splitlines()[0] if code.strip() else ""
            args = shlex.split(first[len("// mxslc:"):], posix=False) if first.startswith("// mxslc:") else []
            total += 1
            ok, err = compile_code(mxslc, code, args)
            where = f"{md.relative_to(SKILL_DIR)}:{line}"
            if not ok:
                fails += 1
                print(f"FAIL {where} {args}\n    " + "\n    ".join(err[:6]))
            elif verbose:
                print(f"ok   {where}")
    print(f"code blocks: {total - fails}/{total} compiled")
    return fails


def split_top(s):
    parts, depth, cur = [], 0, ""
    for ch in s:
        depth += ch in "({"
        depth -= ch in ")}"
        if ch == "," and depth == 0:
            parts.append(cur.strip())
            cur = ""
        else:
            cur += ch
    return parts + ([cur.strip()] if cur.strip() else [])


def check_stdlib(mxslc, verbose):
    total = fails = 0
    pattern = re.compile(r"- `(.*?) ([A-Za-z_0-9]+)(<([A-Za-z_0-9]+)>)?\((.*)\)`( — ([TV]): (.*))?$")
    for line in (SKILL_DIR / "references" / "stdlib.md").read_text().splitlines():
        m = pattern.match(line)
        if not m:
            continue
        ret, name, _, tmpl, params_s, _, letter, types = m.groups()
        if name in KNOWN_UNCALLABLE:
            continue
        t = types.split("|")[0] if types else None
        params = []
        for p in split_top(params_s):
            enum = None
            em = re.search(r"\{([^}]*)\}$", p)
            if em and "=" in p:
                enum, p = em.group(1).split("|"), p[:em.start()].strip()
            ty, pname = p.split(" = ")[0].rsplit(" ", 1)
            params.append((t if (letter and ty == letter) else ty, pname, enum))

        def value(ty, enum):
            if ty == "string":
                return f'"{enum[0]}"' if enum else '"a"'
            return VALUES.get(ty, f"default({ty})")

        call_t = (f"<{t}>" if tmpl == letter else f"<{tmpl}>") if tmpl else ""
        dst = t if (letter and ret == letter) else ret
        dst = "auto" if dst.startswith("{") else dst
        for kind, args in (("positional", ", ".join(value(ty, e) for ty, _, e in params)),
                           ("named", ", ".join(f"{n} = {value(ty, e)}" for ty, n, e in params))):
            total += 1
            code = f"{dst} r = {name}{call_t}({args});\n"
            ok, err = compile_code(mxslc, code)
            if not ok:
                fails += 1
                print(f"FAIL [{kind}] {line[:120]}\n    {code.strip()[:160]}\n    {' | '.join(err)[:300]}")
            elif verbose:
                print(f"ok   [{kind}] {name}")
    print(f"stdlib signatures: {total - fails}/{total} compiled")
    return fails


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--mxslc", default="mxslc", help="path to the mxslc executable")
    ap.add_argument("--stdlib", action="store_true", help="also check every stdlib.md signature (slow)")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    fails = check_blocks(args.mxslc, args.verbose)
    if args.stdlib:
        fails += check_stdlib(args.mxslc, args.verbose)
    raise SystemExit(1 if fails else 0)


if __name__ == "__main__":
    main()
