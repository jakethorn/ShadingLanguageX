"""Generate the SLX signature listing of a MaterialX data library (the "Full listing" in stdlib.md).

Usage:
    python gen_stdlib.py <path/to/libraries/1.39.5>                      # print the listing
    python gen_stdlib.py <path/to/libraries/1.39.5> --update ../references/stdlib.md

Mirrors how mxslc++ turns NodeDefs into functions (source/utils/load_mtlx.cpp):
  * function name   = NodeDef `node` attribute (category)
  * template type   = postfix of the NodeDef name after the last '_' when it is a type name
  * parameters      = active inputs (inheritance resolved), in order, all defaulting to null
  * return type     = the single output type, or an unnamed struct {type name, ...} of all outputs
  * NodeDefs with a version string that are not the default version are skipped
Variants with identical port names are merged: T marks positions that track the return type, V marks
inputs that vary in lock-step when the return type is fixed.
"""
import argparse
import xml.etree.ElementTree as ET
from pathlib import Path

ALIASES = {"boolean": "bool", "integer": "int", "vector2": "vec2", "vector3": "vec3",
           "vector4": "vec4", "matrix33": "mat3", "matrix44": "mat4"}
TYPE_ORDER = ["bool", "int", "float", "vec2", "vec3", "vec4", "color3", "color4", "mat3", "mat4",
              "string", "filename"]
LIBS = ["stdlib", "pbrlib", "bxdf", "nprlib", "lights", "cmlib"]
LISTING_MARKER = "# Full listing"


def load(lib_root):
    all_nds, nodedefs, typedefs = {}, [], set()
    for f in sorted(lib_root.rglob("*.mtlx")):
        rel = f.relative_to(lib_root)
        if rel.parts[0] == "targets":
            continue
        root = ET.parse(f).getroot()
        for td in root.iter("typedef"):
            typedefs.add(td.get("name"))
        for nd in root.iter("nodedef"):
            all_nds[nd.get("name")] = nd
            if nd.get("version") and nd.get("isdefaultversion") != "true":
                continue
            nodedefs.append((rel.parts[0], nd))
    types = typedefs | {"boolean", "integer", "float", "vector2", "vector3", "vector4", "color3", "color4",
                        "matrix33", "matrix44", "string", "filename"}
    return all_nds, nodedefs, types


def al(t):
    return ALIASES.get(t, t)


def sort_types(ts):
    return sorted(ts, key=lambda t: (TYPE_ORDER.index(t) if t in TYPE_ORDER else 99, t))


def norm_default(inp):
    v, t = inp.get("value"), inp.get("type")
    if v is None or (v == "" and t not in ("string", "filename")):
        return None
    if t in ("string", "filename"):
        return f'"{v}"'
    parts = [p.strip() for p in v.split(",")]
    if len(parts) > 1 and len(set(parts)) == 1:
        return parts[0]
    return v if len(parts) == 1 else "(" + ", ".join(parts) + ")"


def generate(lib_root):
    all_nds, nodedefs, types = load(lib_root)

    def active(nd, tag):
        seen, result = set(), []
        while nd is not None:
            for c in nd.findall(tag):
                if c.get("name") not in seen:
                    seen.add(c.get("name"))
                    result.append(c)
            nd = all_nds.get(nd.get("inherit")) if nd.get("inherit") else None
        return result

    def outputs(nd):
        outs = [(o.get("name"), o.get("type")) for o in active(nd, "output")]
        return outs or [("out", nd.get("type"))]

    groups, lib_of = {}, {}
    for lib, nd in nodedefs:
        cat = nd.get("node")
        if cat not in lib_of or LIBS.index(lib) < LIBS.index(lib_of[cat]):
            lib_of[cat] = lib
        ins, outs = active(nd, "input"), outputs(nd)
        key = (cat, tuple(i.get("name") for i in ins), tuple(n for n, _ in outs))
        groups.setdefault(key, []).append((nd, ins, outs))

    by_cat = {}
    for key, variants in groups.items():
        by_cat.setdefault(key[0], []).append((key, variants))

    lines = []
    for lib in LIBS:
        cats = sorted(c for c in by_cat if lib_of[c] == lib)
        if not cats:
            continue
        lines.append(f"\n## {lib}\n")
        for cat in cats:
            multi_overload = sum(len(vs) for _, vs in by_cat[cat]) > 1
            for (_, in_names, out_names), variants in by_cat[cat]:
                lines.extend(format_group(cat, in_names, out_names, variants, types, multi_overload))
    return "\n".join(lines)


def format_group(cat, in_names, out_names, variants, types, multi_overload):
    single = len(out_names) == 1
    rets = [v[2][0][1] for v in variants]
    templated_ret = single and len(set(rets)) > 1
    nparams = len(in_names)
    varying = [i for i in range(nparams) if len({v[1][i].get("type") for v in variants}) > 1]

    # positions that genuinely track the return type rather than matching it by coincidence
    linked = set()
    if templated_ret:
        for i in range(nparams):
            same = [v[1][i].get("type") for v, r in zip(variants, rets) if v[1][i].get("type") == r]
            if len(same) == len(variants) or len(set(same)) >= 3:
                linked.add(i)
    elif varying:
        linked = set(varying)

    shapes = {}
    for (nd, ins, outs), r in zip(variants, rets):
        ptypes = [i.get("type") for i in ins]
        postfix = nd.get("name").rsplit("_", 1)[-1]
        postfix = postfix if postfix in types else None
        if templated_ret:
            tv, letter = r, "T"
        elif varying and len({ptypes[i] for i in varying}) == 1:
            tv, letter = ptypes[varying[0]], "V"
        else:
            tv, letter = None, None

        def sub(t):
            return letter if (tv is not None and t == tv) else al(t)

        ret_s = sub(r) if single else "{" + ", ".join(f"{sub(t)} {n}" for n, t in outs) + "}"
        params = [(sub(ptypes[i]) if i in linked else al(ptypes[i]), inp) for i, inp in enumerate(ins)]
        has_tform = tv is not None and postfix == tv
        key = (ret_s, tuple(t for t, _ in params), has_tform, letter)
        concrete_ret = al(r) if single else "{" + ", ".join(f"{al(t)} {n}" for n, t in outs) + "}"
        entry = shapes.setdefault(key, {
            "types": [], "params": params, "postfixes": set(),
            "concrete": (concrete_ret, [(al(t), inp) for t, inp in zip(ptypes, ins)]),
            "defaults": [[] for _ in ins],
        })
        entry["postfixes"].add(postfix)
        if tv is not None:
            entry["types"].append(tv)
        for i, inp in enumerate(ins):
            entry["defaults"][i].append(inp)

    lines = []
    for (ret_s, _, has_tform, letter), e in shapes.items():
        params = e["params"]
        single_postfix = len(e["postfixes"]) == 1 and None not in e["postfixes"]
        explicit = None
        if letter and len(set(e["types"])) == 1:
            ret_s, params = e["concrete"]
            letter = None
            if single_postfix:
                explicit = al(next(iter(e["postfixes"])))
        if not letter and not explicit and multi_overload and single_postfix:
            explicit = al(next(iter(e["postfixes"])))
        parts = []
        for i, (ty, inp0) in enumerate(params):
            ds = {norm_default(x) for x in e["defaults"][i]}
            extra = ""
            if inp0.get("defaultgeomprop"):
                extra = f" = <{inp0.get('defaultgeomprop')}>"
            elif len(ds) == 1 and None not in ds:
                extra = f" = {ds.pop()}"
            if inp0.get("enum"):
                extra += " {" + "|".join(x.strip() for x in inp0.get("enum").split(",")) + "}"
            parts.append(f"{ty} {inp0.get('name')}{extra}")
        if has_tform and letter:
            name = f"{cat}<{letter}>"
        elif explicit:
            name = f"{cat}<{explicit}>"
        else:
            name = cat
        tinfo = f" — {letter}: " + "|".join(sort_types({al(t) for t in e["types"]})) if letter else ""
        lines.append(f"- `{ret_s} {name}({', '.join(parts)})`{tinfo}")
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("libraries", type=Path, help="a versioned MaterialX libraries folder, e.g. libraries/1.39.5")
    ap.add_argument("--update", type=Path, help="rewrite everything after the '# Full listing' heading of this file")
    args = ap.parse_args()

    listing = generate(args.libraries)
    if not args.update:
        print(listing)
        return

    text = args.update.read_text()
    head = text[:text.index(LISTING_MARKER)]
    args.update.write_text(f"{head}{LISTING_MARKER} (MaterialX {args.libraries.name})\n{listing}\n")


if __name__ == "__main__":
    main()
