import MaterialX as mx

doc: mx.Document = mx.createDocument()

# nodedef
nd: mx.NodeDef = doc.addNodeDef("ND_my_color", "color3", "my_color")
nd_output: mx.Output = nd.getOutput("out")
nd_output.setAttribute("default", str(mx.Color3()))
nd_input: mx.Input = nd.addInput("input_color", "color3")
nd_input.setValue(mx.Color3())

# nodegraph
ng: mx.NodeGraph = doc.addNodeGraph("NG_my_color")
ng.setNodeDef(nd)

node1: mx.Node = ng.addNode("constant", "node1", "multioutput")
node1.setInputValue("value", mx.Color3(1, 0, 0))
#node1_output = node1.addOutput("my_output1", "color3")
#node1_output.setValue(mx.Color3())
#node1_output = node1.addOutput("my_output2", "color3")
#node1_output.setValue(mx.Color3())

node2: mx.Node = ng.addNode("add", "node2", "color3")
#node2.setConnectedNode("in1", node1)
node2.setConnectedNode("in1", node1)
node2.getInput("in1").setOutputString("my_output1")
#print(node2.getInput("in1").getOutputString(), "hey")
#node2.setConnectedOutput("in2", node1.getOutput("my_output2"))
#print(node2.getInput("in2").getOutputString(), "hey")
#print(node2.getInput("in3").getOutputString(), "hey")
#add_input: mx.Input = node2.addInput("in2", "color3")
#add_input.setInterfaceName("input_color")
#print(add_input.getInterfaceInput())

output: mx.Output = ng.addOutput("out", "color3")
output.setConnectedNode(node2)

# node
node3: mx.Node = doc.addNode("my_color", "node3", "color3")
node3.setInputValue("input_color", mx.Color3(0, 1, 0))

# surfaceshader
node4: mx.Node = doc.addNode("standard_surface", "node4", "surfaceshader")
node4.setConnectedNode("base_color", node3)

# surfacematerial
doc.addMaterialNode("node5", node4)

print(doc.validate())

print(mx.writeToXmlString(doc))
with open("./demo.mtlx", "w") as f:
    f.write(mx.writeToXmlString(doc))
