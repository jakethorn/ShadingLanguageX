from pathlib import Path

from .Preprocessor.process import process as preprocess
from .parse import parse
from .scan import scan


def compile_(source: str | Path, include_dirs: list[Path], is_main: bool) -> None:
        tokens = scan(_stdlib_defs) + scan(source)
        processed_tokens = preprocess(tokens, include_dirs, is_main=is_main)
        statements = parse(processed_tokens)
        for statement in statements:
            statement.execute()


_stdlib_defs: str = """
#ifndef STDLIB_DEFS
#define STDLIB_DEFS

// ----------------  displacement  ----------------

displacementshader displacement(float displacement = null, float scale = null)
{
    return {"displacement", displacementshader: displacement=displacement, scale=scale};
}

displacementshader displacement(vec3 displacement, float scale = null)
{
    return {"displacement", displacementshader: displacement=displacement, scale=scale};
}

// ----------------  standard_surface  ----------------

surfaceshader standard_surface(
    float base = null,
    color3 base_color = null,
    float diffuse_roughness = null,
    float metalness = null,
    float specular = null,
    color3 specular_color = null,
    float specular_roughness = null,
    float specular_IOR = null,
    float specular_anisotropy = null,
    float specular_rotation = null,
    float transmission = null,
    color3 transmission_color = null,
    float transmission_depth = null,
    color3 transmission_scatter = null,
    float transmission_scatter_anisotropy = null,
    float transmission_dispersion = null,
    float transmission_extra_roughness = null,
    float subsurface = null,
    color3 subsurface_color = null,
    color3 subsurface_radius = null,
    float subsurface_scale = null,
    float subsurface_anisotropy = null,
    float sheen = null,
    color3 sheen_color = null,
    float sheen_roughness = null,
    float coat = null,
    color3 coat_color = null,
    float coat_roughness = null,
    float coat_anisotropy = null,
    float coat_rotation = null,
    float coat_IOR = null,
    vec3 coat_normal = null,
    float coat_affect_color = null,
    float coat_affect_roughness = null,
    float thin_film_thickness = null,
    float thin_film_IOR = null,
    float emission = null,
    color3 emission_color = null,
    color3 opacity = null,
    bool thin_walled = null,
    vec3 normal = null,
    vec3 tangent = null
)
{
    return {"standard_surface", surfaceshader:
        base=base,
        base_color=base_color,
        diffuse_roughness=diffuse_roughness,
        metalness=metalness,
        specular=specular,
        specular_color=specular_color,
        specular_roughness=specular_roughness,
        specular_IOR=specular_IOR,
        specular_anisotropy=specular_anisotropy,
        specular_rotation=specular_rotation,
        transmission=transmission,
        transmission_color=transmission_color,
        transmission_depth=transmission_depth,
        transmission_scatter=transmission_scatter,
        transmission_scatter_anisotropy=transmission_scatter_anisotropy,
        transmission_dispersion=transmission_dispersion,
        transmission_extra_roughness=transmission_extra_roughness,
        subsurface=subsurface,
        subsurface_color=subsurface_color,
        subsurface_radius=subsurface_radius,
        subsurface_scale=subsurface_scale,
        subsurface_anisotropy=subsurface_anisotropy,
        sheen=sheen,
        sheen_color=sheen_color,
        sheen_roughness=sheen_roughness,
        coat=coat,
        coat_color=coat_color,
        coat_roughness=coat_roughness,
        coat_anisotropy=coat_anisotropy,
        coat_rotation=coat_rotation,
        coat_IOR=coat_IOR,
        coat_normal=coat_normal,
        coat_affect_color=coat_affect_color,
        coat_affect_roughness=coat_affect_roughness,
        thin_film_thickness=thin_film_thickness,
        thin_film_IOR=thin_film_IOR,
        emission=emission,
        emission_color=emission_color,
        opacity=opacity,
        thin_walled=thin_walled,
        normal=normal,
        tangent=tangent
    };
}

// ----------------  mix  ----------------

T mix<float, vec2, vec3, vec4, color3, color4>(T fg, T bg, T mix)
{
    return {"mix", T: fg=fg, bg=bg, mix=mix};
}

T mix<vec2, vec3, vec4, color3, color4>(T fg, T bg, float mix)
{
    return {"mix", T: fg=fg, bg=bg, mix=mix};
}

// ----------------  normalmap  ----------------

vec3 normalmap(vec3 in, float scale = null, vec3 normal = null, vec3 tangent = null, vec3 bitangent = null)
{
    return {"normalmap", vec3: in=in, scale=scale, normal=normal, tangent=tangent, bitangent=bitangent};
}

vec3 normalmap(vec3 in, vec2 scale, vec3 normal = null, vec3 tangent = null, vec3 bitangent = null)
{
    return {"normalmap", vec3: in=in, scale=scale, normal=normal, tangent=tangent, bitangent=bitangent};
}

// ----------------  crossproduct  ----------------

vec3 dotproduct(vec3 in1, vec3 in2)
{
    return {"crossproduct", vec3: in1=in1, in2=in2};
}

// ----------------  dotproduct  ----------------

float dotproduct<vec2, vec3, vec4>(T in1, T in2)
{
    return {"dotproduct", float: in1=in1, in2=in2};
}

// ----------------  distance  ----------------

float distance<vec2, vec3, vec4>(T in1, T in2)
{
    return {"distance", float: in1=in1, in2=in2};
}

// ----------------  magnitude  ----------------

float magnitude<vec2, vec3, vec4>(T in)
{
    return {"magnitude", float: in=in};
}

// ----------------  normalize  ----------------

T normalize<vec2, vec3, vec4>(T in)
{
    return {"normalize", T: in=in};
}

// ----------------  max  ----------------

T max<float, vec2, vec3, vec4, color3, color4>(T in1, T in2)
{
    return {"max", T: in1=in1, in2=in2};
}

T max<vec2, vec3, vec4, color3, color4>(T in1, float in2)
{
    return {"max", T: in1=in1, in2=in2};
}

T max<vec2, vec3, vec4, color3, color4>(float in1, T in2)
{
    return {"max", T: in1=in2, in2=in1};
}

T max<float, vec2, vec3, vec4, color3, color4>(T in1, T in2, T in3)
{
    return max(in1, max(in2, in3));
}

T max<float, vec2, vec3, vec4, color3, color4>(T in1, T in2, T in3, T in4)
{
    return max(in1, max(in2, max(in3, in4)));
}

T max<float, vec2, vec3, vec4, color3, color4>(T in1, T in2, T in3, T in4, T in5)
{
    return max(in1, max(in2, max(in3, max(in4, in5))));
}

// ----------------  min  ----------------

T min<float, vec2, vec3, vec4, color3, color4>(T in1, T in2)
{
    return {"min", T: in1=in1, in2=in2};
}

T min<vec2, vec3, vec4, color3, color4>(T in1, float in2)
{
    return {"min", T: in1=in1, in2=in2};
}

T min<vec2, vec3, vec4, color3, color4>(float in1, T in2)
{
    return {"min", T: in1=in2, in2=in1};
}

T min<float, vec2, vec3, vec4, color3, color4>(T in1, T in2, T in3)
{
    return min(in1, min(in2, in3));
}

T min<float, vec2, vec3, vec4, color3, color4>(T in1, T in2, T in3, T in4)
{
    return min(in1, min(in2, min(in3, in4)));
}

T min<float, vec2, vec3, vec4, color3, color4>(T in1, T in2, T in3, T in4, T in5)
{
    return min(in1, min(in2, min(in3, min(in4, in5))));
}

// ----------------  clamp  ----------------

T clamp<float, vec2, vec3, vec4, color3, color4>(T in, T low = null, T high = null)
{
    return {"clamp", T: in=in, low=low, high=high};
}

T clamp<vec2, vec3, vec4, color3, color4>(T in, float low, float high)
{
    return {"clamp", T: in=in, low=low, high=high};
}

// ----------------  exp  ----------------

T exp<float, vec2, vec3, vec4>(T in)
{
    return {"exp", T: in=in};
}

// ----------------  cos  ----------------

T cos<float, vec2, vec3, vec4>(T in)
{
    return {"cos", T: in=in};
}

// ----------------  sin  ----------------

T sin<float, vec2, vec3, vec4>(T in)
{
    return {"sin", T: in=in};
}

// ----------------  round  ----------------

T round<float, vec2, vec3, vec4, color3, color4>(T in)
{
    return {"round", T: in=in};
}

int round(float in)
{
    return {"round", int: in=in};
}

// ----------------  ceil  ----------------

T ceil<float, vec2, vec3, vec4, color3, color4>(T in)
{
    return {"ceil", T: in=in};
}

int ceil(float in)
{
    return {"ceil", int: in=in};
}

// ----------------  floor  ----------------

T floor<float, vec2, vec3, vec4, color3, color4>(T in)
{
    return {"floor", T: in=in};
}

int floor(float in)
{
    return {"floor", int: in=in};
}

// ----------------  absval  ----------------

T absval<float, vec2, vec3, vec4, color3, color4>(T in)
{
    return {"absval", T: in=in};
}

// ----------------  fract  ----------------

T fract<float, vec2, vec3, vec4, color3, color4>(T in)
{
    return {"fract", T: in=in};
}

// ----------------  time  ----------------

float time(float fps = null)
{
    return {"time", float: fps=fps};
}

// ----------------  texcoord  ----------------

vec2 texcoord(int index = null)
{
    return {"texcoord", vec2: index=index};
}

vec3 texcoord(int index = null)
{
    return {"texcoord", vec3: index=index};
}

// ----------------  viewdirection  ----------------

vec3 viewdirection(string space = null)
{
    return {"viewdirection", vec3: space=space};
}

// ----------------  tangent  ----------------

vec3 tangent(string space = null, int index = null)
{
    return {"tangent", vec3: space=space, index=index};
}

// ----------------  normal  ----------------

vec3 normal(string space = null)
{
    return {"normal", vec3: space=space};
}

// ----------------  position  ----------------

vec3 position(string space = null)
{
    return {"position", vec3: space=space};
}

// ----------------  noise2d  ----------------

T noise2d<float, vec2, vec3, vec4>(T amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", T: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

color3 noise2d(vec3 amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", color3: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

color4 noise2d(vec4 amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", color4: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

T noise2d<vec2, vec3, vec4, color3, color4>(float amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", T: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

// ----------------  randomfloat  ----------------

float randomfloat(float in = null, float min = null, float max = null, int seed = null)
{
    return {"randomfloat", float: in=in, min=min, max=max, seed=seed};
}

float randomfloat(int in, float min = null, float max = null, int seed = null)
{
    return {"randomfloat", float: in=in, min=min, max=max, seed=seed};
}

// ----------------  image  ----------------

T image<float, vec2, vec3, vec4, color3, color4>
(filename file, string layer = null, T default = null, vec2 texcoord = null, string uaddressmode = null, string vaddressmode = null, string filtertype = null)
{
    return {"image", T: file=file, layer=layer, default=default, texcoord=texcoord, uaddressmode=uaddressmode, vaddressmode=vaddressmode, filtertype=filtertype};
}

#endif // STDLIB_DEFS

"""
