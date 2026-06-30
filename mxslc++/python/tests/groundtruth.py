
RANDOMFLOAT_OUTPUT = """<?xml version="1.0"?>
<materialx version="1.39">
  <randomfloat name="in1" type="float" />
  <add name="f" type="float">
    <input name="in1" type="float" nodename="in1" />
    <input name="in2" type="float" value="1" />
  </add>
</materialx>
"""


STANDARD_OUTPUT = """<?xml version="1.0"?>
<materialx version="1.39">
  <add name="f" type="float">
    <input name="in1" type="float" value="1" />
    <input name="in2" type="float" value="1" />
  </add>
</materialx>
"""


REDUCED_OUTPUT = """<?xml version="1.0"?>
<materialx version="1.39" />
"""
