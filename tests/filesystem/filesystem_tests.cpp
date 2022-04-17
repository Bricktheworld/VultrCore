#include <gtest/gtest.h>

#include <filesystem/filesystem.h>
#include "filesystem/filestream.h"

using namespace Vultr;
TEST(Filesystem, CreatePath)
{
	Path p("./src");
	ASSERT_TRUE(p.string() == "./src");

	ASSERT_TRUE(exists(p));
	ASSERT_TRUE(p.is_directory());
	ASSERT_FALSE(p.is_file());

	p = Path("/src");
	ASSERT_TRUE(p.string() == "/src");
	ASSERT_FALSE(exists(p));
}

TEST(Filesystem, Path)
{
	Path p("./src");
	Path sub_path = p / "test.cpp";
	ASSERT_TRUE(sub_path.string() == "./src/test.cpp");
}

TEST(Filesystem, FileInputStream)
{
	Path p("./src/renderer/shaders/basic.frag");
	ASSERT_TRUE(exists(p));
	ASSERT_TRUE(p.is_file());

	String buf;
	ASSERT_TRUE(try_fread_all(p, &buf).has_value());
}

TEST(Filesystem, Parent)
{
	Path p("./src/test.cpp");
	ASSERT_TRUE(get_parent(p).value() == Path("./src/"));

	p = Path("./src/a");
	ASSERT_TRUE(get_parent(p).value() == Path("./src/"));

	p = Path("./src/a/");
	ASSERT_TRUE(get_parent(p).value() == Path("./src/"));

	p = Path("/src/test.cpp");
	ASSERT_TRUE(get_parent(p).value() == Path("/src/"));

	p = Path("/src/a");
	ASSERT_TRUE(get_parent(p).value() == Path("/src/"));

	p = Path("/src/a/");
	ASSERT_TRUE(get_parent(p).value() == Path("/src/"));

	p = Path("src/test.cpp");
	ASSERT_TRUE(get_parent(p).value() == Path("src/"));

	p = Path("src/a");
	ASSERT_TRUE(get_parent(p).value() == Path("src/"));

	p = Path("src/a/");
	ASSERT_TRUE(get_parent(p).value() == Path("src/"));

	p = Path("test/");
	ASSERT_TRUE(get_parent(p).has_error());

	p = Path("test");
	ASSERT_TRUE(get_parent(p).has_error());

	p = Path("./test");
	ASSERT_TRUE(get_parent(p).value() == Path("./"));

	p = Path("/test");
	ASSERT_TRUE(get_parent(p).value() == Path("/"));

	p = Path("/test/");
	ASSERT_TRUE(get_parent(p).value() == Path("/"));

	p = Path("a/");
	ASSERT_TRUE(get_parent(p).has_error());

	p = Path("a");
	ASSERT_TRUE(get_parent(p).has_error());

	p = Path("./a");
	ASSERT_TRUE(get_parent(p).value() == Path("./"));

	p = Path("/a");
	ASSERT_TRUE(get_parent(p).value() == Path("/"));

	p = Path("/a/");
	ASSERT_TRUE(get_parent(p).value() == Path("/"));
}
