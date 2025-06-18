import MaterialX as mx

d: mx.Document = mx.createDocument()

n = d.addNode("add", "node1", "float")
n.setInputValue("in1", 5.0)
n.setInputValue("in3", "hello")

a, b = d.validate()
print(a, b)
print(mx.writeToXmlString(d))
