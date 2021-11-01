// TODO: Reimplement using custom VTL
// #include <gtest/gtest.h>
// #define private public
// #define protected public

// #include <filesystem/directory.h>
// #include <filesystem/file.h>

// static const char *dir_path = "res";

// using namespace Vultr;
// class DirectoryTests : public testing::Test
// {
//   protected:
//     // Per-test-suite set-up.
//     // Called before the first test in this test suite.
//     // Can be omitted if not needed.
//     static void SetUpTestSuite()
//     {
//     }

//     // Per-test-suite tear-down.
//     // Called after the last test in this test suite.
//     // Can be omitted if not needed.
//     static void TearDownTestSuite()
//     {
//     }
// };

// TEST_F(DirectoryTests, Directory)
// {
//     auto dir = Directory(dir_path);
//     EXPECT_STRCASEEQ(dir.path, "res/");
//     dir = Directory("res/");
//     EXPECT_STRCASEEQ(dir.path, "res/");
//     dir = Directory("C:\\Windows Path\\");
//     EXPECT_STRCASEEQ(dir.path, "C:/Windows Path/");
//     auto sub_dir = Directory(&dir, "subdirectory");
//     EXPECT_STRCASEEQ(sub_dir.path, "C:/Windows Path/subdirectory/");
// }

// TEST_F(DirectoryTests, Dirbasename)
// {
//     size_t len;
//     auto dir = Directory("./res/");
//     EXPECT_STRCASEEQ(dir.path, "./res/");

//     EXPECT_STRCASEEQ(dirbasename(&dir, &len), "res/");
//     EXPECT_EQ(len, strlen("res"));

//     dir = Directory("/res/otherpath/");
//     EXPECT_STRCASEEQ(dirbasename(&dir, &len), "otherpath/");
//     EXPECT_EQ(len, strlen("otherpath"));

//     dir = Directory("/res/otherpath");
//     EXPECT_STRCASEEQ(dirbasename(&dir, &len), "otherpath/");
//     EXPECT_EQ(len, strlen("otherpath"));

//     dir = Directory("/");
//     EXPECT_STRCASEEQ(dirbasename(&dir, &len), "/");
//     EXPECT_EQ(len, strlen("/"));

//     dir = Directory("./");
//     EXPECT_STRCASEEQ(dirbasename(&dir, &len), "./");
//     EXPECT_EQ(len, strlen("./"));

//     dir = Directory("//");
//     EXPECT_STRCASEEQ(dirbasename(&dir, &len), "/");
//     EXPECT_EQ(len, 0);

//     dir = Directory(".//");
//     EXPECT_STRCASEEQ(dirbasename(&dir, &len), "/");
//     EXPECT_EQ(len, 0);

//     dir = Directory("res/");
//     EXPECT_STRCASEEQ(dir.path, "res/");
// }

// TEST_F(DirectoryTests, Dirparent)
// {
//     Directory parent;

//     Directory dir = Directory("res");
//     dirparent(&dir, &parent);
//     EXPECT_STRCASEEQ(parent.path, "./");

//     dir = Directory("/usr/bin");
//     dirparent(&dir, &parent);
//     EXPECT_STRCASEEQ(parent.path, "/usr/");

//     dir = Directory("/usr/bin/");
//     dirparent(&dir, &parent);
//     EXPECT_STRCASEEQ(parent.path, "/usr/");

//     dir = Directory("./res/some_path/");
//     dirparent(&dir, &parent);
//     EXPECT_STRCASEEQ(parent.path, "./res/");

//     dir = Directory("./res/some_path");
//     dirparent(&dir, &parent);
//     EXPECT_STRCASEEQ(parent.path, "./res/");

//     dir = Directory("res/some_path");
//     dirparent(&dir, &parent);
//     EXPECT_STRCASEEQ(parent.path, "res/");
// }

// TEST_F(DirectoryTests, Dirhasparent)
// {
//     Directory dir = Directory("res");
//     EXPECT_FALSE(dirhasparent(&dir));

//     dir = Directory("./res");
//     EXPECT_FALSE(dirhasparent(&dir));

//     dir = Directory("/res");
//     EXPECT_FALSE(dirhasparent(&dir));

//     dir = Directory("");
//     EXPECT_FALSE(dirhasparent(&dir));

//     dir = Directory("res/subdir");
//     EXPECT_TRUE(dirhasparent(&dir));

//     dir = Directory("./res/subdir");
//     EXPECT_TRUE(dirhasparent(&dir));

//     dir = Directory("/res/subdir");
//     EXPECT_TRUE(dirhasparent(&dir));
// }

// TEST_F(DirectoryTests, Dirmake_Direxists_Dirremove)
// {
//     const char *second_dir_path = "testdir/recursivedir/";
//     Directory dir;
//     EXPECT_TRUE(dirmake(second_dir_path, &dir));
//     EXPECT_TRUE(direxists(&dir));
//     EXPECT_STRCASEEQ(dir.path, second_dir_path);

//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));

//     dir = Directory(second_dir_path);
//     EXPECT_TRUE(dirmake(&dir));
//     EXPECT_TRUE(direxists(&dir));

//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));

//     dir = Directory("testdir");
//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));
// }

// TEST_F(DirectoryTests, Dirrename)
// {
//     const char *path = "toberemoveddir/oldname/";

//     Directory dir;
//     Directory parent;
//     EXPECT_TRUE(dirmake(path, &dir));
//     EXPECT_TRUE(direxists(&dir));

//     EXPECT_TRUE(dirrename(&dir, "newname"));
//     EXPECT_STRCASEEQ(dir.path, "toberemoveddir/newname/");
//     EXPECT_TRUE(direxists(&dir));

//     dirparent(&dir, &parent);
//     EXPECT_TRUE(dirremove(&parent));
//     EXPECT_FALSE(direxists(&parent));
//     EXPECT_FALSE(direxists(&parent));

//     const char *paths[] = {"toberemoveddir/oldname/insidedir/"};

//     EXPECT_TRUE(dirmake(paths[0], &dir));
//     EXPECT_TRUE(direxists(&dir));
//     dirparent(&dir, &parent);
//     dir = parent;

//     EXPECT_TRUE(dirrename(&dir, "newname"));
//     EXPECT_STRCASEEQ(dir.path, "toberemoveddir/newname/");
//     EXPECT_TRUE(direxists(&dir));

//     dirparent(&dir, &parent);
//     EXPECT_TRUE(direxists(&dir));
//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));
// }

// TEST_F(DirectoryTests, Dirmove)
// {
//     const char *path = "toberemoveddir/oldname/";

//     Directory dir;
//     Directory parent;
//     ASSERT_TRUE(dirmake(path, &dir));
//     ASSERT_TRUE(direxists(&dir));

//     EXPECT_TRUE(dirmove(&dir, "./"));
//     EXPECT_STRCASEEQ(dir.path, "./oldname/");
//     EXPECT_TRUE(direxists(&dir));

//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));

//     dir = Directory("toberemoveddir");
//     EXPECT_TRUE(direxists(&dir));
//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));
// }

// TEST_F(DirectoryTests, Dircount)
// {
//     const char *file_data = "DOIN UR MOM DOIN UR MOM WE KNOW WE STRAIGHT DOIN YOUR MOM";
//     Directory dir;
//     ASSERT_TRUE(dirmake("testdir/stuff", &dir));

//     const u32 file_count = 10;
//     for (u32 i = 0; i < file_count; i++)
//     {
//         const char *basefilename = "createdfile.txt";
//         char filename[strlen(basefilename) + 2];
//         sprintf(filename, "createdfile%i.txt", i);
//         GenericFile file = GenericFile(&dir, filename);
//         FILE *f = fopen(file.path, "w+");
//         fputs(file_data, f);
//         fclose(f);
//     }

//     EXPECT_EQ(dirfilecount(&dir), file_count);

//     Directory parent;
//     dirparent(&dir, &parent);
//     EXPECT_EQ(dirsubdirectorycount(&parent), 1);

//     EXPECT_TRUE(direxists(&parent));
//     EXPECT_TRUE(dirremove(&parent));
//     EXPECT_FALSE(direxists(&parent));
// }

// TEST_F(DirectoryTests, Dirfiles)
// {
//     const char *file_data = "DOIN UR MOM DOIN UR MOM WE KNOW WE STRAIGHT DOIN YOUR MOM";
//     Directory dir;
//     ASSERT_TRUE(dirmake("testdir", &dir));

//     const u32 file_count = 10;
//     const char *basefilename = "createdfile.txt";

//     char filename[strlen(basefilename) + 2];
//     for (u32 i = 0; i < file_count; i++)
//     {
//         sprintf(filename, "createdfile%i.txt", i);
//         GenericFile file = GenericFile(&dir, filename);
//         FILE *f = fopen(file.path, "w+");
//         fputs(file_data, f);
//         fclose(f);
//     }

//     size_t len;
//     auto *files = dirfiles(&dir, &len);

//     EXPECT_EQ(len, file_count);

//     for (u32 i = 0; i < len; i++)
//     {
//         size_t _;
//         const char *basename = fbasename(&files[i], &_);
//         bool found_file = false;
//         for (u32 j = 0; j < file_count; j++)
//         {
//             sprintf(filename, "createdfile%i.txt", j);
//             // if (strequal(basename, filename))
//             // {
//             //     found_file = true;
//             //     break;
//             // }
//         }
//         EXPECT_TRUE(found_file) << "File name is " << basename;
//     }
//     delete[] files;

//     EXPECT_TRUE(direxists(&dir));
//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));
// }

// TEST_F(DirectoryTests, Dirsubdirs)
// {
//     Directory dir;
//     ASSERT_TRUE(dirmake("testdir", &dir));

//     const u32 dir_count = 10;
//     const char *basedirname = "createdsubdir";

//     char dirname[strlen(basedirname) + 2];
//     for (u32 i = 0; i < dir_count; i++)
//     {
//         sprintf(dirname, "createdsubdir%i", i);
//         Directory subdir = Directory(&dir, dirname);
//         dirmake(&subdir);
//     }

//     size_t len;
//     auto *dirs = dirsubdirs(&dir, &len);

//     EXPECT_EQ(len, dir_count);

//     for (u32 i = 0; i < len; i++)
//     {
//         size_t _;
//         const char *basename = dirbasename(&dirs[i], &_);

//         bool found_dir = false;
//         for (u32 j = 0; j < dir_count; j++)
//         {
//             sprintf(dirname, "createdsubdir%i/", j);
//             // if (strequal(basename, dirname))
//             // {
//             //     found_dir = true;
//             //     break;
//             // }
//         }
//         EXPECT_TRUE(found_dir) << "Subdir name is " << basename;
//     }
//     delete[] dirs;

//     EXPECT_TRUE(direxists(&dir));
//     EXPECT_TRUE(dirremove(&dir));
//     EXPECT_FALSE(direxists(&dir));
// }
