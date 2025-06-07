from . import Statement
from .. import mtlx, state
from ..CompileError import CompileError
from ..DataType import DataType, FLOAT, COLOR3, VECTOR3, BOOLEAN
from ..Expressions import Expression, IfExpression, IdentifierExpression
from ..Token import Token
from ..token_types import IDENTIFIER
from ..utils import type_of_swizzle


# TODO rename swizzle to property
class VariableAssignment(Statement):
    def __init__(self, identifier: Token, swizzle: Token, right: Expression):
        self.identifier = identifier
        self.swizzle = swizzle.lexeme if swizzle else None
        self.right = right

    def instantiate_templated_types(self, data_type: DataType) -> Statement:
        swizzle = Token(IDENTIFIER, self.swizzle) if self.swizzle else None
        right = self.right.instantiate_templated_types(data_type)
        return VariableAssignment(self.identifier, swizzle, right)

    def execute(self) -> None:
        node = state.get_node(self.identifier)
        if self.swizzle is None:
            self.execute_as_identifier(node)
        # TODO do the same for displacement
        elif node.category == "standard_surface":
            self.execute_as_surface_input(node)
        else:
            self.execute_as_swizzle(node)

    def execute_as_identifier(self, old_node: mtlx.Node) -> None:
        new_node = self.evaluate_right(old_node.data_type)
        state.set_node(self.identifier, new_node)

    def execute_as_surface_input(self, surface_node: mtlx.Node) -> None:
        if self.swizzle in _standard_surface_inputs:
            input_type = _standard_surface_inputs[self.swizzle]
        else:
            raise CompileError(f"Input '{self.swizzle}' does not exist in the standard surface.", self.identifier)
        surface_node.set_input(self.swizzle, self.evaluate_right(input_type))

    def execute_as_swizzle(self, old_node: mtlx.Node) -> None:
        # evaluate right hand expression
        right_node = self.evaluate_right(type_of_swizzle(self.swizzle))

        # split into channels corresponding to swizzle
        right_channels = mtlx.extract_all(right_node)
        swizzle_channel_map = {"x": 0, "y": 1, "z": 2, "w": 3, "r": 0, "g": 1, "b": 2, "a": 3}
        swizzle_channels = [swizzle_channel_map[c] for c in self.swizzle]
        assert len(right_channels) == len(swizzle_channels)

        # get default channels of old variable
        data = mtlx.extract_all(old_node)

        # override swizzle channels with right hand data
        for swizzle_channel, right_channel in zip(swizzle_channels, right_channels):
            data[swizzle_channel] = right_channel

        # combine into final node
        node = mtlx.combine(data, old_node.data_type)
        state.set_node(self.identifier, node)

    def evaluate_right(self, valid_type: DataType) -> mtlx.Node:
        if isinstance(self.right, IfExpression) and self.right.otherwise is None:
            self.right.otherwise = IdentifierExpression(self.identifier)
        return self.right.init_evaluate(valid_type)


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
