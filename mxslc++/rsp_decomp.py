from pathlib import Path

with open("main.rsp", "r") as f:
    rsp = f.read()

args = rsp.split()

action = "compile"
if args[0] in ["compile", "decompile"]:
    action = args.pop(0)

input_file = Path(args[0])
options = args[1:]

if action == "compile":
    action = "decompile"
    input_file = input_file.with_suffix(".mtlx")
    options.clear()
    options.append("-o " + str(input_file.with_stem(input_file.stem + "_decomped").with_suffix(".mxsl")))
else:
    action = "compile"
    input_file = input_file.with_suffix(".mxsl")
    options.clear()
    options.append("--no-reduce-graph")

with open("main.rsp", "w") as f:
    f.write(action + "\n" + str(input_file) + "\n" + "\n".join(options) + "\n")
