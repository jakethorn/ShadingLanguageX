//
// Tests for the ShadingLanguageX -> MaterialX decompiler.
//

#include "gtest/gtest.h"

#include <string>

#include "decompile/decompile.h"

using std::string;

namespace
{
    string decompile(const string& mtlx)
    {
        return mxslc::decompile::decompile_to_string(mtlx);
    }
}

TEST(decompiler, input_colorspace_attribute_is_preserved)
{
    const string mtlx = R"(
<?xml version="1.0"?>
<materialx version="1.39">
  <image name="c" type="color3">
    <input name="file" type="filename" colorspace="srgb_texture" value="butterfly1.png" />
  </image>
</materialx>
)";

    const string expected = "@file.colorspace \"srgb_texture\"\n"
                            "color3 c = image(file = \"butterfly1.png\");\n";

    EXPECT_EQ(decompile(mtlx), expected);
}

TEST(decompiler, input_colorspace_attribute_only_emitted_when_present)
{
    const string mtlx = R"(
<?xml version="1.0"?>
<materialx version="1.39">
  <image name="c" type="color3">
    <input name="file" type="filename" value="butterfly1.png" />
  </image>
</materialx>
)";

    const string expected = "color3 c = image(file = \"butterfly1.png\");\n";

    EXPECT_EQ(decompile(mtlx), expected);
}

TEST(decompiler, colorspace_attribute_handles_multiple_inputs)
{
    const string mtlx = R"(
<?xml version="1.0"?>
<materialx version="1.39">
  <image name="c" type="color3">
    <input name="file" type="filename" colorspace="srgb_texture" value="butterfly1.png" />
    <input name="layer" type="string" value="default" />
  </image>
</materialx>
)";

    const string expected = "@file.colorspace \"srgb_texture\"\n"
                            "color3 c = image(file = \"butterfly1.png\", layer = \"default\");\n";

    EXPECT_EQ(decompile(mtlx), expected);
}

TEST(decompiler, untranslated_attributes_are_emitted_generically)
{
    // Attributes that are not part of the translated ShadingLanguageX syntax
    // (node-level `doc`, input `colorspace`, `unit`, `unittype`) are emitted as
    // `@` declarations, while structural attributes (`name`, `type`, `value`,
    // `nodename`, `xpos`, `ypos`) are skipped.
    const string mtlx = R"(
<?xml version="1.0"?>
<materialx version="1.39">
  <image name="c" type="color3" doc="an image of a butterfly" xpos="1.5" ypos="2.5">
    <input name="file" type="filename" colorspace="srgb_texture" unit="default" unittype="filename" value="butterfly1.png" />
  </image>
</materialx>
)";

    const string expected = "@doc \"an image of a butterfly\"\n"
                            "@file.colorspace \"srgb_texture\"\n"
                            "@file.unit \"default\"\n"
                            "@file.unittype \"filename\"\n"
                            "color3 c = image(file = \"butterfly1.png\");\n";

    EXPECT_EQ(decompile(mtlx), expected);
}
