// Static MXSL keyword lists used for CodeMirror syntax highlighting.
// There is no backend (everything runs client-side in the WASM module), so the
// keyword sets are embedded here. Edit these lists as the language evolves.
//
// "control" mirrors the mxslc TokenType keyword enum (include/TokenType.h).
// "dataTypes" mirrors the mxslc data-type keywords.
// "functions" is a curated set of common MaterialX node names (used as function
// tokens when followed by "(").

export const MXSL_KEYWORDS = {
    // Control / language keywords
    control: [
        'if', 'else', 'for', 'from', 'to', 'return', 'null', 'ref', 'out',
        'const', 'mutable', 'consteval', 'global', 'geomprop', 'inline',
        'default', 'comptime', 'using', 'class', 'this', 'uniform', 'varying',
        'namespace', 'print', 'typeof', 'break',
    ],

    // Data types
    dataTypes: [
        'bool', 'int', 'float', 'string',
        'bool2', 'bool3', 'bool4',
        'int2', 'int3', 'int4',
        'float2', 'float3', 'float4',
        'double', 'double2', 'double3', 'double4',
        'color2', 'color3', 'color4',
        'vector2', 'vector3', 'vector4',
        'matrix33', 'matrix44',
        'filename',
        'surfaceshader', 'displacementshader', 'volumeshader',
        'lightshader', 'material',
    ],

    // Common MaterialX node / function names (highlighted as functions when
    // immediately followed by "(").
    functions: [
        'add', 'subtract', 'multiply', 'divide', 'modulo', 'power', 'sqrt',
        'invert', 'absval', 'floor', 'ceil', 'round', 'sign', 'min', 'max',
        'clamp', 'mix', 'smoothstep', 'normalize', 'dotproduct', 'crossproduct',
        'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'atan2', 'exp', 'log',
        'log10', 'sinh', 'cosh', 'tanh', 'exp2', 'log2',
        'transformpoint', 'transformvector', 'transformnormal',
        'combine2', 'combine3', 'combine4', 'extract', 'convert',
        'noise2d', 'noise3d', 'fractal3d', 'worleynoise3d', 'cellnoise2d', 'cellnoise3d',
        'image', 'constant', 'ramplr', 'ramptb', 'splitlr', 'splittb',
        'standard_surface', 'usd_preview_surface', 'surfacematerial',
        'diffuse_bsdf', 'glossy_bsdf', 'dielectric_bsdf', 'conductor_bsdf',
        'sheen_bsdf', 'subsurface_bsdf', 'transmission_bsdf', 'generalized_schlick_bsdf',
        'displacement', 'volume',
    ],
};
