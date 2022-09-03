//#include <gtest/gtest.h>
//
//#include <platform/platform.h>
//
//using namespace Vultr;
//TEST(Filesystem, DirIteration)
//{
//	CHECK_UNWRAP(auto *dir, Platform::Filesystem::open_dir("/home/brandon"));
//	while (true)
//	{
//		auto entry = Platform::Filesystem::read_dir(dir);
//		if (entry.has_error())
//			break;
//		switch (entry.value().type)
//		{
//			case Platform::Filesystem::EntryType::FILE:
//				printf("File %s, serial %lu\n", entry.value().name, entry.value().uuid);
//				break;
//			case Platform::Filesystem::EntryType::DIR:
//				printf("Subdirectory %s, serial %lu\n", entry.value().name, entry.value().uuid);
//				break;
//		}
//	}
//	CHECK(Platform::Filesystem::close_dir(dir));
//}
//
//TEST(Filesystem, FileDateModified)
//{
//	CHECK_UNWRAP(auto ms, Platform::Filesystem::fdate_modified_ms("./src/renderer/shaders/basic.frag"));
//	printf("MS since epoch: %lu", ms);
//}
