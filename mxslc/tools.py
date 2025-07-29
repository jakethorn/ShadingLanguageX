from mxslc.mx_wrapper import Document

print("NodeDefs with multiple outputs:")
doc = Document()
doc.load_standard_library()
for nd in doc.node_defs:
    if nd.output_count > 1:
        print()
        print(nd)
        for input_ in nd.inputs:
            print(input_)
        for output in nd.outputs:
            print(output)
