#pragma once
/**
 * NOTE(Brandon): To dump file resources to header files that can be included into a binary, simply run
 * xxd -i <input_resource_file> > <output_header_file>.h
 */
namespace Vultr::EditorResources
{
	unsigned char *GET_FORKAWESOME_WEBFONT_TTF();
	static unsigned int FORKAWESOME_WEBFONT_TTF_LEN = 218132;

	unsigned char *GET_ROBOTO_TTF();
	static unsigned int ROBOTO_TTF_LEN = 171272;

	unsigned char *GET_MESH_PNG();
	static unsigned int MESH_PNG_LEN    = 1048576;
	static unsigned int MESH_PNG_WIDTH  = 512;
	static unsigned int MESH_PNG_HEIGHT = 512;

	unsigned char *GET_CPP_PNG();
	static unsigned int CPP_PNG_LEN    = 1048576;
	static unsigned int CPP_PNG_WIDTH  = 512;
	static unsigned int CPP_PNG_HEIGHT = 512;

	unsigned char *GET_FILE_PNG();
	static unsigned int FILE_PNG_LEN    = 20736;
	static unsigned int FILE_PNG_WIDTH  = 72;
	static unsigned int FILE_PNG_HEIGHT = 72;

	unsigned char *GET_FOLDER_PNG();
	static unsigned int FOLDER_PNG_LEN    = 1048576;
	static unsigned int FOLDER_PNG_WIDTH  = 512;
	static unsigned int FOLDER_PNG_HEIGHT = 512;

	unsigned char *GET_SHADER_PNG();
	static unsigned int SHADER_PNG_LEN    = 1048576;
	static unsigned int SHADER_PNG_WIDTH  = 512;
	static unsigned int SHADER_PNG_HEIGHT = 512;

	unsigned char *GET_TEXTURE_PNG();
	static unsigned int TEXTURE_PNG_LEN    = 20736;
	static unsigned int TEXTURE_PNG_WIDTH  = 72;
	static unsigned int TEXTURE_PNG_HEIGHT = 72;
} // namespace Vultr::EditorResources
