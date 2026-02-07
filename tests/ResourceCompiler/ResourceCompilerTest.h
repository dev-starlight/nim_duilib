#ifndef TESTS_RESOURCE_COMPILER_TEST_H_
#define TESTS_RESOURCE_COMPILER_TEST_H_

#pragma once

#include <gtest/gtest.h>
#include <filesystem>

namespace ui {
namespace test {

class ResourceCompilerTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    std::filesystem::path m_testResourceDir;
    std::filesystem::path m_tempOutputDir;

    void CreateTestResourceFiles();
    void CreateTestQrcFile();
    void CleanupTempFiles();
};

} // namespace test
} // namespace ui

#endif // TESTS_RESOURCE_COMPILER_TEST_H_
