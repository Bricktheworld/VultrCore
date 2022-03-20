#include <types/types.h>
#include <filesystem/filesystem.h>
#include <filesystem/filestream.h>
#include <spirv_reflect/spirv_reflect.h>

int main(int argc, char **argv)
{
	using namespace Vultr;

	Path vertex_shader(argv[1]);
	if (!exists(vertex_shader))
	{
		fprintf(stderr, "Vertex shader %s not found.", vertex_shader.c_str());
		return 1;
	}

	Buffer vertex_shader_code{};
	fread_all(vertex_shader, &vertex_shader_code);

	SpvReflectShaderModule module{};
	SpvReflectResult result = spvReflectCreateShaderModule(vertex_shader_code.size(), vertex_shader_code.storage, &module);
	PRODUCTION_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to reflect vertex shader!");

	u32 var_count = 0;
	result        = spvReflectEnumerateInputVariables(&module, &var_count, nullptr);
	PRODUCTION_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate input variables for vertex shader!");

	SpvReflectInterfaceVariable *input_vars[var_count];
	result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
	PRODUCTION_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate input variables for vertex shader!");

	return 0;
}