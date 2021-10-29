// #include <gtest/gtest.h>
// #define private public
// #define protected public
// #include <hashtable.h>

// using namespace vtl;

// TEST(HashTable, Iterator)
// {
//     const char *string_keys[] = {"urmom", "chungus", "poop", "awoooga"};
//     const char *string_vals[] = {"joe", "big", "shit", "eyes pop out"};
//     auto table = new_hashtable<const char *, const char *>();

//     for (int i = 0; i < 4; i++)
//     {
//         table[string_keys[i]] = string_vals[i];
//     }

//     uint i = 0;
//     for (auto iterator = table.begin(); iterator != table.end(); iterator++)
//     {
//         EXPECT_STREQ((*iterator).key, string_keys[i]) << "Index " << i;
//         EXPECT_STREQ((*iterator).value, string_vals[i]) << "Index " << i;
//         i++;
//     }

//     i = 0;
//     for (auto pair : table)
//     {
//         EXPECT_STREQ(pair.key, string_keys[i]) << "Index " << i;
//         EXPECT_STREQ(pair.value, string_vals[i]) << "Index " << i;
//         i++;
//     }

//     i = 0;
//     for (auto [key, value] : table)
//     {
//         EXPECT_STREQ(key, string_keys[i]) << "Index " << i;
//         EXPECT_STREQ(value, string_vals[i]) << "Index " << i;
//         i++;
//     }
// }
