//
// Created by jaket on 01/09/2026.
//

#include <gtest/gtest.h>
#include "Primitive.h"
#include "utils/data_utils.h"

TEST(overwrite_data_files_tests, overwrite_data_files_is_false)
{
    ASSERT_FALSE(overwrite_data_files());
}
