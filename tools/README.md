## `houdini/`
A Houdini plugin with an SLX Material LOP for Solaris, and tools to convert MaterialX materials to SLX. See [houdini/README.md](houdini/README.md).

## `mtlx_to_slx.py`
Decompiles a .mtlx file into an SLX source file. For example:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <viewdirection name="v" type="vector3" />
  <normal name="n" type="vector3" />
  <dotproduct name="node1" type="float">
    <input name="in1" type="vector3" nodename="v" />
    <input name="in2" type="vector3" nodename="n" />
  </dotproduct>
  <subtract name="theta" type="float">
    <input name="in1" type="float" value="0" />
    <input name="in2" type="float" nodename="node1" />
  </subtract>
  <smoothstep name="outline" type="float">
    <input name="in" type="float" nodename="theta" />
    <input name="low" type="float" value="0.2" />
    <input name="high" type="float" value="0.25" />
  </smoothstep>
  <position name="node6" type="vector3" />
  <multiply name="node12" type="vector3">
    <input name="in1" type="vector3" nodename="node6" />
    <input name="in2" type="float" nodename="outline" />
  </multiply>
  <convert name="c" type="color3">
    <input name="in" type="vector3" nodename="node12" />
  </convert>
  <standard_surface name="surface" type="surfaceshader">
    <input name="base_color" type="color3" nodename="c" />
    <input name="specular_roughness" type="float" value="1" />
  </standard_surface>
  <surfacematerial name="mxsl_material" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="surface" />
  </surfacematerial>
</materialx>

```
decompiles to:
```
vector3 v = viewdirection();
vector3 n = normal();
float node1 = dotproduct(in1=v, in2=n);
float theta = 0.0 - node1;
float outline = smoothstep(in=theta, low=0.2, high=0.25);
vector3 node6 = position();
vector3 node12 = node6 * outline;
color3 c = color3(node12);
surfaceshader surface = standard_surface(base_color=c, specular_roughness=1.0);
//material mxsl_material = {"surfacematerial", material: surfaceshader=surface};
```
