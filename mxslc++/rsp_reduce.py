from pathlib import Path

with open("main.rsp", "r") as f:
    rsp = f.read()

args = rsp.split()

action = ""
if args[0] in ["compile", "decompile"]:
    action = args.pop(0) + "\n"

input_file = Path(args[0])
options = args[1:]

if "--no-reduce-graph" in options:
    options.remove("--no-reduce-graph")
    if "-o" not in options:
        options.append("-o " + str(input_file.with_stem(input_file.stem + "_reduced").with_suffix(".mtlx")))
else:
    options.clear()
    options.append("--no-reduce-graph")

with open("main.rsp", "w") as f:
    f.write(str(input_file) + "\n" + "\n".join(options) + "\n")
