from . import Statement
from .. import mx_utils, state
from ..CompileError import CompileError
from ..DataType import DataType, FLOAT, COLOR3, VECTOR3, BOOLEAN
from ..Expressions import Expression, IfExpression, IdentifierExpression
from ..Token import Token
from ..utils import type_of_swizzle, string


class VariableAssignment(Statement):
    def __init__(self, identifier: Token, property_: Token | str | None, right: Expression):
        self.__identifier = identifier
        self.__property = string(property_)
        self.__right = right

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        right = self.__right.instantiate_templated_types(template_type)
        return VariableAssignment(self.__identifier, self.__property, right)

    def execute(self) -> None:
        node = state.get_node(self.__identifier)
        if self.__property is None:
            self.execute_as_identifier(node)
        elif node.category == "standard_surface":
            self.execute_as_surface_input(node)
        elif node.category == "displacement":
            self.execute_as_surface_input(node)
        else:
            self.execute_as_swizzle(node)

    def execute_as_identifier(self, old_node: mx_utils.Node) -> None:
        new_node = self.evaluate_right(old_node.data_type)
        state.set_node(self.__identifier, new_node)

    def execute_as_surface_input(self, surface_node: mx_utils.Node) -> None:
        if self.__property in _standard_surface_inputs:
            input_type = _standard_surface_inputs[self.__property]
        else:
            raise CompileError(f"Input '{self.__property}' does not exist in the standard surface shader.", self.__identifier)
        surface_node.set_input(self.__property, self.evaluate_right(input_type))

    def execute_as_displacement_input(self, displacement_node: mx_utils.Node) -> None:
        if self.__property in _standard_surface_inputs:
            input_types = _standard_surface_inputs[self.__property]
        else:
            raise CompileError(f"Input '{self.__property}' does not exist in the displacement shader.", self.__identifier)
        displacement_node.set_input(self.__property, self.evaluate_right(input_types))

    def execute_as_swizzle(self, old_node: mx_utils.Node) -> None:
        # evaluate right hand expression
        right_node = self.evaluate_right(type_of_swizzle(self.__property))

        # split into channels corresponding to swizzle
        right_channels = mx_utils.extract_all(right_node)
        swizzle_channel_map = {"x": 0, "y": 1, "z": 2, "w": 3, "r": 0, "g": 1, "b": 2, "a": 3}
        swizzle_channels = [swizzle_channel_map[c] for c in self.__property]
        assert len(right_channels) == len(swizzle_channels)

        # get default channels of old variable
        data = mx_utils.extract_all(old_node)

        # override swizzle channels with right hand data
        for swizzle_channel, right_channel in zip(swizzle_channels, right_channels):
            data[swizzle_channel] = right_channel

        # combine into final node
        node = mx_utils.combine(data, old_node.data_type)
        state.set_node(self.__identifier, node)

    def evaluate_right(self, valid_types: DataType | set[DataType]) -> mx_utils.Node:
        # TODO fix if expressions to work with surface/displacement shader properties
        if isinstance(self.__right, IfExpression) and self.__right.otherwise is None:
            self.__right.otherwise = IdentifierExpression(self.__identifier)
        return self.__right.init_evaluate(valid_types)


_standard_surface_inputs: dict[str, DataType] = {
    "base": FLOAT,
    "base_color": COLOR3,
    "diffuse_roughness": FLOAT,
    "metalness": FLOAT,
    "specular": FLOAT,
    "specular_color": COLOR3,
    "specular_roughness": FLOAT,
    "specular_IOR": FLOAT,
    "specular_anisotropy": FLOAT,
    "specular_rotation": FLOAT,
    "transmission": FLOAT,
    "transmission_color": COLOR3,
    "transmission_depth": FLOAT,
    "transmission_scatter": COLOR3,
    "transmission_scatter_anisotropy": FLOAT,
    "transmission_dispersion": FLOAT,
    "transmission_extra_roughness": FLOAT,
    "subsurface": FLOAT,
    "subsurface_color": COLOR3,
    "subsurface_radius": COLOR3,
    "subsurface_scale": FLOAT,
    "subsurface_anisotropy": FLOAT,
    "sheen": FLOAT,
    "sheen_color": COLOR3,
    "sheen_roughness": FLOAT,
    "coat": FLOAT,
    "coat_color": COLOR3,
    "coat_roughness": FLOAT,
    "coat_anisotropy": FLOAT,
    "coat_rotation": FLOAT,
    "coat_IOR": FLOAT,
    "coat_normal": VECTOR3,
    "coat_affect_color": FLOAT,
    "coat_affect_roughness": FLOAT,
    "thin_film_thickness": FLOAT,
    "thin_film_IOR": FLOAT,
    "emission": FLOAT,
    "emission_color": COLOR3,
    "opacity": COLOR3,
    "thin_walled": BOOLEAN,
    "normal": VECTOR3,
    "tangent": VECTOR3
}

_displacement_inputs: dict[str, set[DataType]] = {
    "displacement": {FLOAT, VECTOR3},
    "scale": {FLOAT}
}
