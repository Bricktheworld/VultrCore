// #include <gtest/gtest.h>
// #define private public
// #define protected public

// #include <types/utils/string/string.h>

// using namespace Vultr;
// TEST(String_Utils, Str)
// {
//     char *string = str(100);

//     strcat(string, "Hello");
//     ASSERT_STRCASEEQ(string, "Hello");

//     strcpy(string, "Hello \0");
//     strcat(string, "My Baby\0");
//     ASSERT_STRCASEEQ(string, "Hello My Baby");
//     free(string);

//     const char *string_literal = "Hello my baby, hello my honey, hello my ragtime gal";
//     string = strn(string_literal, 5);
//     ASSERT_STRCASEEQ(string, "Hello");
//     ASSERT_EQ(string[5], '\0');
//     free(string);

//     string = str(string_literal);
//     ASSERT_EQ(strcmp(string, string_literal), 0);
//     free(string);

//     char *empty_str = str("");
//     ASSERT_EQ(strcmp(empty_str, ""), 0);
//     free(empty_str);
// }

// TEST(String_Utils, StrEqual)
// {
//     const char *string_literal = "Hello my baby, hello my honey, hello my ragtime gal";
//     char *string = str(string_literal);

//     ASSERT_TRUE(strequal(string, string_literal));
//     ASSERT_TRUE(strnequal(string, "Hello", strlen("Hello")));
//     free(string);

//     string = str("");
//     ASSERT_TRUE(strequal(string, ""));
//     free(string);
// }

// TEST(String_Utils, Restr)
// {
//     const char *str1 = "Hello";
//     char *string = str(str1);

//     const char *str2 = " My ";
//     string = restr(string, strlen(str1) + strlen(str2));
//     strcat(string, str2);
//     ASSERT_STRCASEEQ(string, "Hello My ");
//     free(string);

//     string = str("");
//     restr(string, strlen(str2));
//     strcat(string, str2);
//     ASSERT_STRCASEEQ(string, str2);
//     free(string);
// }

// TEST(String_Utils, Strappend)
// {
//     const char *str1 = "Hello";
//     char *string = str(str1);

//     const char *str2 = " My ";
//     strappend(string, str2);
//     ASSERT_STRCASEEQ(string, "Hello My ");
//     free(string);

//     string = str("");
//     strappend(string, str1);
//     strappend(string, str2);
//     ASSERT_STRCASEEQ(string, "Hello My ");
//     free(string);
// }

// TEST(String_Utils, Strcreplace)
// {
//     const char *str1 = "Hello";
//     char *string = str(str1);

//     strcreplace(string, 'l', 'w');
//     EXPECT_STRCASEEQ(string, "Hewwo");
// }
