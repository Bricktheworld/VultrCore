// TODO: Reimplement using custom VTL
// #include <gtest/gtest.h>
// #define private public
// #define protected public

// #include <stdio.h>
// #include <filesystem/file.h>

// static const char *file_path = "test_file.txt";
// static const char *file_extension = ".txt";
// static const char *file_data = "string";

// using namespace Vultr;
// class FileTests : public testing::Test
// {
//   protected:
//     // Per-test-suite set-up.
//     // Called before the first test in this test suite.
//     // Can be omitted if not needed.
//     static void SetUpTestSuite()
//     {
//         FILE *f = fopen(file_path, "w+");
//         fputs(file_data, f);
//         fclose(f);
//     }

//     // Per-test-suite tear-down.
//     // Called after the last test in this test suite.
//     // Can be omitted if not needed.
//     static void TearDownTestSuite()
//     {
//         remove(file_path);
//     }
// };
// TEST_F(FileTests, File)
// {
//     Directory dir = Directory("./some/path");
//     GenericFile file = GenericFile(&dir, "file_name.cpp");
//     EXPECT_STRCASEEQ(file.path, "./some/path/file_name.cpp");
// }

// TEST_F(FileTests, FBasename)
// {
//     size_t len;
//     GenericFile file = GenericFile(file_path);
//     EXPECT_STRCASEEQ(fbasename(&file, &len), file_path);
//     EXPECT_EQ(len, strlen(file_path));

//     GenericFile local_file = GenericFile("./test_file.cpp");
//     EXPECT_STRCASEEQ(fbasename(&local_file, &len), "test_file.cpp");
//     EXPECT_EQ(len, strlen("test_file.cpp"));

//     GenericFile windows_file = GenericFile("C:\\Windows Path\\test_file.cpp");
//     EXPECT_STRCASEEQ(windows_file.path, "C:/Windows Path/test_file.cpp");
//     EXPECT_STRCASEEQ(fbasename(&windows_file, &len), "test_file.cpp");
//     EXPECT_EQ(len, strlen("test_file.cpp"));
// }

// TEST_F(FileTests, FExtension)
// {
//     GenericFile file = GenericFile(file_path);
//     EXPECT_STRCASEEQ(fextension(&file), file_extension);

//     file = GenericFile("no_extension");
//     EXPECT_EQ(fextension(&file), nullptr);
// }

// TEST_F(FileTests, FExtensionMatches)
// {
//     GenericFile file = GenericFile("some_file.jpeg");

//     const char *extension = fextension(&file);
//     EXPECT_TRUE(fextension_matches(extension, FileTypes::TEXTURE_SOURCE, FileTypes::TEXTURE_SOURCE_LEN));
//     EXPECT_FALSE(fextension_matches(extension, FileTypes::MODEL_SOURCE, FileTypes::MODEL_SOURCE_LEN));
// }

// TEST_F(FileTests, FRename)
// {
//     GenericFile file = GenericFile(file_path);
//     file_path = "new_test_file.text";
//     file_extension = ".text";
//     frename(&file, file_path);

//     size_t len;

//     EXPECT_STRCASEEQ(fextension(&file), file_extension);
//     EXPECT_STRCASEEQ(fbasename(&file, &len), file_path);
//     EXPECT_EQ(len, strlen(file_path));

//     FILE *f = fopen(file_path, "r");
// #define READ_BUFFER_SIZE 32768
//     char buffer[READ_BUFFER_SIZE];

//     // char *res_str = str("");

//     // while (!feof(f))
//     // {
//     //     size_t bytes = fread(buffer, 1, sizeof(buffer), f);

//     //     if (bytes)
//     //     {
//     //         res_str = strappend(res_str, buffer);
//     //     }
//     // }
//     // EXPECT_TRUE(strequal(res_str, file_data));

//     fclose(f);
// }

// TEST_F(FileTests, FCopy)
// {
//     const char *copy_file_path = "copy_file.txt";
//     GenericFile file = GenericFile(file_path);

//     bool res = fcopy(&file, copy_file_path);
//     ASSERT_TRUE(res);

//     ASSERT_STRCASEEQ(file.path, copy_file_path);

//     FILE *f = fopen(copy_file_path, "r");
// #define READ_BUFFER_SIZE 32768
//     char buffer[READ_BUFFER_SIZE];

//     // char *res_str = str("");

//     // while (!feof(f))
//     // {
//     //     size_t bytes = fread(buffer, 1, sizeof(buffer), f);

//     //     if (bytes)
//     //     {
//     //         res_str = strappend(res_str, buffer);
//     //     }
//     // }
//     // EXPECT_STRCASEEQ((res_str, file_data);

//     fclose(f);
// }

// TEST_F(FileTests, FMove)
// {
//     Directory some_dir = Directory("some/directory");
//     ASSERT_TRUE(dirmake(&some_dir));

//     GenericFile file = GenericFile(&some_dir, "some_file.txt");
//     ASSERT_STRCASEEQ(file.path, "some/directory/some_file.txt");

//     FILE *f = fopen(file.path, "w+");
//     fputs(file_data, f);
//     fclose(f);

//     Directory result_dir;
//     dirparent(&some_dir, &result_dir);

//     bool res = fmove(&file, &result_dir);
//     ASSERT_TRUE(res);
//     ASSERT_TRUE(fexists(&file));
//     ASSERT_STRCASEEQ(file.path, "some/some_file.txt");

//     ASSERT_TRUE(dirremove(&result_dir));
//     ASSERT_FALSE(direxists(&result_dir));
// }

// TEST_F(FileTests, FRemove_FExists)
// {
//     const char *copy_file_path = "copy_file.txt";
//     GenericFile file = GenericFile(copy_file_path);

//     bool removal_successful = fremove(&file);
//     ASSERT_TRUE(removal_successful);

//     bool copy_exists = fexists(&file);
//     ASSERT_FALSE(copy_exists);

//     GenericFile orig = GenericFile(file_path);

//     bool orig_exists = fexists(&orig);
//     ASSERT_TRUE(orig_exists);
// }
