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

float mix(float fg, float bg, float mix)
{
    return {"mix", float: fg=fg, bg=bg, mix=mix};
}

color3 mix(color3 fg, color3 bg, float mix)
{
    return {"mix", color3: fg=fg, bg=bg, mix=mix};
}

color3 mix(color3 fg, color3 bg, color3 mix)
{
    return {"mix", color3: fg=fg, bg=bg, mix=mix};
}

color4 mix(color4 fg, color4 bg, float mix)
{
    return {"mix", color4: fg=fg, bg=bg, mix=mix};
}

color4 mix(color4 fg, color4 bg, color4 mix)
{
    return {"mix", color4: fg=fg, bg=bg, mix=mix};
}

vec2 mix(vec2 fg, vec2 bg, float mix)
{
    return {"mix", vec2: fg=fg, bg=bg, mix=mix};
}

vec2 mix(vec2 fg, vec2 bg, vec2 mix)
{
    return {"mix", vec2: fg=fg, bg=bg, mix=mix};
}

vec3 mix(vec3 fg, vec3 bg, float mix)
{
    return {"mix", vec3: fg=fg, bg=bg, mix=mix};
}

vec3 mix(vec3 fg, vec3 bg, vec3 mix)
{
    return {"mix", vec3: fg=fg, bg=bg, mix=mix};
}

vec4 mix(vec4 fg, vec4 bg, float mix)
{
    return {"mix", vec4: fg=fg, bg=bg, mix=mix};
}

vec4 mix(vec4 fg, vec4 bg, vec4 mix)
{
    return {"mix", vec4: fg=fg, bg=bg, mix=mix};
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

float dotproduct(vec2 in1, vec2 in2)
{
    return {"dotproduct", float: in1=in1, in2=in2};
}

float dotproduct(vec3 in1, vec3 in2)
{
    return {"dotproduct", float: in1=in1, in2=in2};
}

float dotproduct(vec4 in1, vec4 in2)
{
    return {"dotproduct", float: in1=in1, in2=in2};
}

// ----------------  distance  ----------------

float distance(vec2 in1, vec2 in2)
{
    return {"distance", float: in1=in1, in2=in2};
}

float distance(vec3 in1, vec3 in2)
{
    return {"distance", float: in1=in1, in2=in2};
}

float distance(vec4 in1, vec4 in2)
{
    return {"distance", float: in1=in1, in2=in2};
}

// ----------------  magnitude  ----------------

float magnitude(vec2 in)
{
    return {"magnitude", float: in=in};
}

float magnitude(vec3 in)
{
    return {"magnitude", float: in=in};
}

float magnitude(vec4 in)
{
    return {"magnitude", float: in=in};
}

// ----------------  normalize  ----------------

vec2 normalize(vec2 in)
{
    return {"normalize", vec2: in=in};
}

vec3 normalize(vec3 in)
{
    return {"normalize", vec3: in=in};
}

vec4 normalize(vec4 in)
{
    return {"normalize", vec4: in=in};
}

// ----------------  max  ----------------

float max(float in1, float in2)
{
    return {"max", float: in1=in1, in2=in2};
}

vec2 max(vec2 in1, vec2 in2)
{
    return {"max", vec2: in1=in1, in2=in2};
}

vec3 max(vec3 in1, vec3 in2)
{
    return {"max", vec3: in1=in1, in2=in2};
}

vec4 max(vec4 in1, vec4 in2)
{
    return {"max", vec4: in1=in1, in2=in2};
}

color3 max(color3 in1, color3 in2)
{
    return {"max", color3: in1=in1, in2=in2};
}

color4 max(color4 in1, color4 in2)
{
    return {"max", color4: in1=in1, in2=in2};
}

vec2 max(vec2 in1, float in2)
{
    return {"max", vec2: in1=in1, in2=in2};
}

vec3 max(vec3 in1, float in2)
{
    return {"max", vec3: in1=in1, in2=in2};
}

vec4 max(vec4 in1, float in2)
{
    return {"max", vec4: in1=in1, in2=in2};
}

color3 max(color3 in1, float in2)
{
    return {"max", color3: in1=in1, in2=in2};
}

color4 max(color4 in1, float in2)
{
    return {"max", color4: in1=in1, in2=in2};
}

float max(float in1, float in2, float in3)
{
    return max(in1, max(in2, in3));
}

// ----------------  min  ----------------

float min(float in1, float in2)
{
    return {"min", float: in1=in1, in2=in2};
}

vec2 min(vec2 in1, vec2 in2)
{
    return {"min", vec2: in1=in1, in2=in2};
}

vec3 min(vec3 in1, vec3 in2)
{
    return {"min", vec3: in1=in1, in2=in2};
}

vec4 min(vec4 in1, vec4 in2)
{
    return {"min", vec4: in1=in1, in2=in2};
}

color3 min(color3 in1, color3 in2)
{
    return {"min", color3: in1=in1, in2=in2};
}

color4 min(color4 in1, color4 in2)
{
    return {"min", color4: in1=in1, in2=in2};
}

vec2 min(vec2 in1, float in2)
{
    return {"min", vec2: in1=in1, in2=in2};
}

vec3 min(vec3 in1, float in2)
{
    return {"min", vec3: in1=in1, in2=in2};
}

vec4 min(vec4 in1, float in2)
{
    return {"min", vec4: in1=in1, in2=in2};
}

color3 min(color3 in1, float in2)
{
    return {"min", color3: in1=in1, in2=in2};
}

color4 min(color4 in1, float in2)
{
    return {"min", color4: in1=in1, in2=in2};
}

float min(float in1, float in2, float in3)
{
    return min(in1, min(in2, in3));
}

// ----------------  clamp  ----------------

float clamp(float in, float low = null, float high = null)
{
    return {"clamp", float: in=in, low=low, high=high};
}

vec2 clamp(vec2 in, vec2 low = null, vec2 high = null)
{
    return {"clamp", vec2: in=in, low=low, high=high};
}

vec3 clamp(vec3 in, vec3 low = null, vec3 high = null)
{
    return {"clamp", vec3: in=in, low=low, high=high};
}

vec4 clamp(vec4 in, vec4 low = null, vec4 high = null)
{
    return {"clamp", vec4: in=in, low=low, high=high};
}

color3 clamp(color3 in, color3 low = null, color3 high = null)
{
    return {"clamp", color3: in=in, low=low, high=high};
}

color4 clamp(color4 in, color4 low = null, color4 high = null)
{
    return {"clamp", color4: in=in, low=low, high=high};
}

vec2 clamp(vec2 in, float low, float high)
{
    return {"clamp", vec2: in=in, low=low, high=high};
}

vec3 clamp(vec3 in, float low, float high)
{
    return {"clamp", vec3: in=in, low=low, high=high};
}

vec4 clamp(vec4 in, float low, float high)
{
    return {"clamp", vec4: in=in, low=low, high=high};
}

color3 clamp(color3 in, float low, float high)
{
    return {"clamp", color3: in=in, low=low, high=high};
}

color4 clamp(color4 in, float low, float high)
{
    return {"clamp", color4: in=in, low=low, high=high};
}

// ----------------  exp  ----------------

float exp(float in)
{
    return {"exp", float: in=in};
}

vec2 exp(vec2 in)
{
    return {"exp", vec2: in=in};
}

vec3 exp(vec3 in)
{
    return {"exp", vec3: in=in};
}

vec4 exp(vec4 in)
{
    return {"exp", vec4: in=in};
}

// ----------------  cos  ----------------

float cos(float in)
{
    return {"cos", float: in=in};
}

vec2 cos(vec2 in)
{
    return {"cos", vec2: in=in};
}

vec3 cos(vec3 in)
{
    return {"cos", vec3: in=in};
}

vec4 cos(vec4 in)
{
    return {"cos", vec4: in=in};
}

// ----------------  sin  ----------------

float sin(float in)
{
    return {"sin", float: in=in};
}

vec2 sin(vec2 in)
{
    return {"sin", vec2: in=in};
}

vec3 sin(vec3 in)
{
    return {"sin", vec3: in=in};
}

vec4 sin(vec4 in)
{
    return {"sin", vec4: in=in};
}

// ----------------  round  ----------------

float round(float in)
{
    return {"round", float: in=in};
}

vec2 round(vec2 in)
{
    return {"round", vec2: in=in};
}

vec3 round(vec3 in)
{
    return {"round", vec3: in=in};
}

vec4 round(vec4 in)
{
    return {"round", vec4: in=in};
}

color3 round(color3 in)
{
    return {"round", color3: in=in};
}

color4 round(color4 in)
{
    return {"round", color4: in=in};
}

int round(float in)
{
    return {"round", float: in=in};
}

// ----------------  ceil  ----------------

float ceil(float in)
{
    return {"ceil", float: in=in};
}

vec2 ceil(vec2 in)
{
    return {"ceil", vec2: in=in};
}

vec3 ceil(vec3 in)
{
    return {"ceil", vec3: in=in};
}

vec4 ceil(vec4 in)
{
    return {"ceil", vec4: in=in};
}

color3 ceil(color3 in)
{
    return {"ceil", color3: in=in};
}

color4 ceil(color4 in)
{
    return {"ceil", color4: in=in};
}

int ceil(float in)
{
    return {"ceil", float: in=in};
}

// ----------------  floor  ----------------

float floor(float in)
{
    return {"floor", float: in=in};
}

vec2 floor(vec2 in)
{
    return {"floor", vec2: in=in};
}

vec3 floor(vec3 in)
{
    return {"floor", vec3: in=in};
}

vec4 floor(vec4 in)
{
    return {"floor", vec4: in=in};
}

color3 floor(color3 in)
{
    return {"floor", color3: in=in};
}

color4 floor(color4 in)
{
    return {"floor", color4: in=in};
}

int floor(float in)
{
    return {"floor", float: in=in};
}

// ----------------  absval  ----------------

float absval(float in)
{
    return {"absval", float: in=in};
}

vec2 absval(vec2 in)
{
    return {"absval", vec2: in=in};
}

vec3 absval(vec3 in)
{
    return {"absval", vec3: in=in};
}

vec4 absval(vec4 in)
{
    return {"absval", vec4: in=in};
}

color3 absval(color3 in)
{
    return {"absval", color3: in=in};
}

color4 absval(color4 in)
{
    return {"absval", color4: in=in};
}

// ----------------  fract  ----------------

float fract(float in)
{
    return {"fract", float: in=in};
}

vec2 fract(vec2 in)
{
    return {"fract", vec2: in=in};
}

vec3 fract(vec3 in)
{
    return {"fract", vec3: in=in};
}

vec4 fract(vec4 in)
{
    return {"fract", vec4: in=in};
}

color3 fract(color3 in)
{
    return {"fract", color3: in=in};
}

color4 fract(color4 in)
{
    return {"fract", color4: in=in};
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

float noise2d(float amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", float: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

color3 noise2d(vec3 amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", color3: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

color4 noise2d(vec4 amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", color4: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

vec2 noise2d(vec2 amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", vec2: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

vec3 noise2d(vec3 amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", vec3: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

vec4 noise2d(vec4 amplitude = null, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", vec4: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

color3 noise2d(float amplitude, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", color3: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

color4 noise2d(float amplitude, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", color4: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

vec2 noise2d(float amplitude, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", vec2: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

vec3 noise2d(float amplitude, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", vec3: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
}

vec4 noise2d(float amplitude, float pivot = null, vector2 texcoord = null)
{
    return {"noise2d", vec4: amplitude=amplitude, pivot=pivot, texcoord=texcoord};
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

float image(filename file, string layer = null, float default = null, vec2 texcoord = null, string uaddressmode = null, string vaddressmode = null, string filtertype = null)
{
    return {"image", float: file=file, layer=layer, default=default, texcoord=texcoord, uaddressmode=uaddressmode, vaddressmode=vaddressmode, filtertype=filtertype};
}

vec2 image(filename file, string layer = null, vec2 default = null, vec2 texcoord = null, string uaddressmode = null, string vaddressmode = null, string filtertype = null)
{
    return {"image", vec2: file=file, layer=layer, default=default, texcoord=texcoord, uaddressmode=uaddressmode, vaddressmode=vaddressmode, filtertype=filtertype};
}

vec3 image(filename file, string layer = null, vec3 default = null, vec2 texcoord = null, string uaddressmode = null, string vaddressmode = null, string filtertype = null)
{
    return {"image", vec3: file=file, layer=layer, default=default, texcoord=texcoord, uaddressmode=uaddressmode, vaddressmode=vaddressmode, filtertype=filtertype};
}

vec4 image(filename file, string layer = null, vec4 default = null, vec2 texcoord = null, string uaddressmode = null, string vaddressmode = null, string filtertype = null)
{
    return {"image", vec4: file=file, layer=layer, default=default, texcoord=texcoord, uaddressmode=uaddressmode, vaddressmode=vaddressmode, filtertype=filtertype};
}

color3 image(filename file, string layer = null, color3 default = null, vec2 texcoord = null, string uaddressmode = null, string vaddressmode = null, string filtertype = null)
{
    return {"image", color3: file=file, layer=layer, default=default, texcoord=texcoord, uaddressmode=uaddressmode, vaddressmode=vaddressmode, filtertype=filtertype};
}

color4 image(filename file, string layer = null, color4 default = null, vec2 texcoord = null, string uaddressmode = null, string vaddressmode = null, string filtertype = null)
{
    return {"image", color4: file=file, layer=layer, default=default, texcoord=texcoord, uaddressmode=uaddressmode, vaddressmode=vaddressmode, filtertype=filtertype};
}

#endif // STDLIB_DEFS

"""
