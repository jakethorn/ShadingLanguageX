from pathlib import Path

import MaterialX as mx

type Constant = bool | int | float | mx.Vector2 | mx.Vector3 | mx.Vector4 | mx.Color3 | mx.Color4 | str | Path
type Value = mx.Node | Constant
