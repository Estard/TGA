if(WIN32)
    add_definitions(-DNOMINMAX)
    set(GLSLC glslc.exe) 
else()
    set(GLSLC glslc)
endif(WIN32)

# Compiles all shaders in /glsl folder to SPIR-V

file(GLOB_RECURSE GLSL_SHADERS CONFIGURE_DEPENDS "glsl/*")


foreach(GLSL ${GLSL_SHADERS})
    get_filename_component(FILE_NAME ${GLSL} NAME_WE)
    get_filename_component(FILE_EXT ${GLSL} LAST_EXT)
    string(REPLACE "." "" FILE_TYPE ${FILE_EXT})
    set(SPIRV "${FILE_NAME}_${FILE_TYPE}.spv")
    add_custom_command( OUTPUT ${SPIRV}
                        COMMAND ${GLSLC} ${GLSL} -O -o ${SPIRV}
                        DEPENDS ${GLSL})
    list(APPEND SPIRV_SHADERS ${SPIRV})
endforeach(GLSL)


add_custom_target(example_shaders DEPENDS ${SPIRV_SHADERS})