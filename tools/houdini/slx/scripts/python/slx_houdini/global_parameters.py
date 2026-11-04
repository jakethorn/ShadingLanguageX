"""Shows the `global` variables an SLX source declares as parameters on its SLX Material node.

The parameters are spare parameters in the node's Globals tab. slx_houdini.lop decides when to update them.
"""

import hou

FOLDER_NAME = "globals_folder"
FOLDER_LABEL = "Globals"  # tab folders can only be found by label
NO_GLOBALS_LABEL_NAME = "noglobals"
PARM_PREFIX = "global_"
TYPE_TAG = "slx_global_type"
DEFAULT_TAG = "slx_global_default"

VECTOR_SIZES = {"vector2": 2, "vector3": 3, "vector4": 4, "color3": 3, "color4": 4}
EDITABLE_TYPES = {"float", "integer", "boolean", "string", "filename"} | set(VECTOR_SIZES)


class GlobalParameter:
    """A global variable's name, SLX type and default value, as plain Python values."""

    def __init__(self, name, type_name, default):
        self.name = name
        self.type_name = type_name
        self.default = default  # None when the initializer isn't known at compile time

    def parm_name(self):
        return PARM_PREFIX + self.name

    def is_editable(self):
        # A parameter would replace a runtime initializer, such as texcoord().x, with a constant.
        return self.type_name in EDITABLE_TYPES and self.default is not None

    def signature(self):
        return self.parm_name(), self.type_name, repr(self.default)


def get_global_parameters(mxslc, source, options, is_file):
    """Returns the globals that source declares, or [] if this mxslc can't reflect them (before mxslcxx 0.3.3).

    options shouldn't give the globals values, because a global's default is the value options gives it.
    """
    reflect = getattr(mxslc, "reflect_from_file" if is_file else "reflect_from_string", None)
    if reflect is None:
        return []
    reflection = reflect(source, options)
    return [GlobalParameter(g.name, g.type, to_python_value(g.default_value)) for g in reflection.globals]


def to_python_value(value):
    # mxslc returns vectors and colours as MaterialX objects, e.g. MaterialX.Color3.
    if hasattr(value, "asTuple"):
        return tuple(float(v) for v in value.asTuple())
    if isinstance(value, list):
        return None  # a struct, which has no parameter
    return value


def get_global_values(node, parameters):
    """Returns {global name: value} from the node's parameters, for globals whose parameter matches the declaration."""
    values = {}
    for parameter in parameters:
        parm_tuple = node.parmTuple(parameter.parm_name())
        if parm_tuple is None or not parameter.is_editable():
            continue
        if parm_tuple.parmTemplate().tags().get(TYPE_TAG) != parameter.type_name:
            continue
        values[parameter.name] = get_parm_tuple_value(parm_tuple, parameter.type_name)
    return values


def get_parm_tuple_value(parm_tuple, type_name):
    if type_name in VECTOR_SIZES:
        return [float(v) for v in parm_tuple.eval()]
    if type_name in ("string", "filename"):
        return parm_tuple[0].evalAsString()
    if type_name == "boolean":
        return bool(parm_tuple[0].eval())
    return parm_tuple[0].eval()


def has_matching_parameters(node, parameters):
    return get_node_signature(node) == [p.signature() for p in parameters]


def get_node_signature(node):
    folder = node.parmTemplateGroup().findFolder(FOLDER_LABEL)
    templates = folder.parmTemplates() if folder is not None else ()
    return [(t.name(), t.tags()[TYPE_TAG], t.tags()[DEFAULT_TAG]) for t in templates if TYPE_TAG in t.tags()]


def update_parameters(node, parameters):
    """Makes node's Globals tab hold one parameter per global in parameters.

    Values the user changed are kept. A parameter still at its default takes the new default, so editing a global's
    initializer in the SLX source moves its parameter too.
    """
    group = node.parmTemplateGroup()
    old_folder = group.findFolder(FOLDER_LABEL)
    folder = old_folder.clone()
    at_default = {t.name() for t in folder.parmTemplates()
                  if TYPE_TAG in t.tags() and node.parmTuple(t.name()).isAtDefault()}

    templates = [t for t in folder.parmTemplates() if TYPE_TAG not in t.tags()]
    for template in templates:
        if template.name() == NO_GLOBALS_LABEL_NAME:
            template.hide(bool(parameters))
    templates += [create_parm_template(p) for p in parameters]
    folder.setParmTemplates(templates)
    group.replace(old_folder, folder)
    with hou.undos.disabler():
        node.setParmTemplateGroup(group)
        for parameter in parameters:
            if parameter.parm_name() in at_default and parameter.is_editable():
                node.parmTuple(parameter.parm_name()).revertToDefaults()


def create_parm_template(parameter):
    name, label, type_name, default = parameter.parm_name(), parameter.name, parameter.type_name, parameter.default
    if parameter.is_editable():
        template = create_value_parm_template(name, label, type_name, default)
        template.setHelp("Sets the global variable {} in the SLX source. Its default is the variable's "
                         "initializer.".format(parameter.name))
    else:
        template = hou.LabelParmTemplate(name, label, column_labels=("{}, set by its initializer".format(type_name),))
        template.setHelp("The global variable {} keeps its initializer from the SLX source, because a parameter "
                         "can't hold it.".format(parameter.name))
    template.setTags({TYPE_TAG: type_name, DEFAULT_TAG: repr(default)})
    return template


def create_value_parm_template(name, label, type_name, default):
    """Returns a parameter template for a value of an editable type, such as a global's or an entry argument's.

    Sliders range from 0 to twice the default, or from -2x to 2x for a negative default.
    """
    if type_name == "float":
        limit = max(1.0, abs(default) * 2)
        return hou.FloatParmTemplate(name, label, 1, default_value=(default,), min=-limit if default < 0 else 0.0,
                                     max=limit, min_is_strict=False, max_is_strict=False)
    if type_name == "integer":
        limit = max(10, abs(default) * 2)
        return hou.IntParmTemplate(name, label, 1, default_value=(default,), min=-limit if default < 0 else 0,
                                   max=limit, min_is_strict=False, max_is_strict=False)
    if type_name == "boolean":
        return hou.ToggleParmTemplate(name, label, default_value=bool(default))
    if type_name == "filename":
        return hou.StringParmTemplate(name, label, 1, default_value=(default,),
                                      string_type=hou.stringParmType.FileReference)
    if type_name == "string":
        return hou.StringParmTemplate(name, label, 1, default_value=(default,))
    if type_name.startswith("color"):
        return hou.FloatParmTemplate(name, label, VECTOR_SIZES[type_name], default_value=default,
                                     look=hou.parmLook.ColorSquare, naming_scheme=hou.parmNamingScheme.RGBA)
    return hou.FloatParmTemplate(name, label, VECTOR_SIZES[type_name], default_value=default,
                                 naming_scheme=hou.parmNamingScheme.XYZW)


def get_zero_value(type_name):
    """Returns an editable type's zero value, e.g. 0.0 for float, (0.0, 0.0) for vector2 and "" for string."""
    if type_name in VECTOR_SIZES:
        return (0.0,) * VECTOR_SIZES[type_name]
    return {"float": 0.0, "integer": 0, "boolean": False}.get(type_name, "")
