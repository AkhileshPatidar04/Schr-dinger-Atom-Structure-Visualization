cmake_minimum_required(VERSION 3.10)
project(Schrodinger_Atom_Structure_Visualization CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_executable(atom_viewer
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Wavefunction.cpp
    src/OrbitalCloud.cpp
)

target_include_directories(atom_viewer PRIVATE src)

target_link_libraries(atom_viewer PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

if (MSVC)
    # vcpkg's GLEW/GLFW static+shared mix needs this on some MSVC configs;
    # harmless to set unconditionally.
    target_compile_definitions(atom_viewer PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(atom_viewer PRIVATE /O2 /Oi /Ot)
else()
    target_compile_options(atom_viewer PRIVATE -O3)
endif()

# Copy shaders next to the actual executable after every build. Using
# $<TARGET_FILE_DIR:...> (instead of CMAKE_BINARY_DIR) makes this correct
# for multi-config generators too, i.e. the Visual Studio generator, where
# the .exe ends up in build/Debug/ or build/Release/, not build/ itself.
add_custom_command(TARGET atom_viewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:atom_viewer>/shaders"
    COMMENT "Copying shaders next to atom_viewer executable"
)

# Make Visual Studio's built-in debugger ("Local Windows Debugger" / F5)
# launch with its working directory set to the exe's folder, so the
# relative "shaders/..." paths in main.cpp resolve without extra setup.
set_target_properties(atom_viewer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:atom_viewer>"
)
