mkdir -p build/shaders

glslc ./src/renderer/shaders/basic.vert -o build/shaders/basic_vert.spv
glslc ./src/renderer/shaders/basic.frag -o build/shaders/basic_frag.spv
