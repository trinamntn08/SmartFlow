add_library(smartflow_scene_math STATIC
  tp_math_utils/src/Geometry3D.cpp
  tp_math_utils/src/Material.cpp
  tp_math_utils/src/MeshKeyFrame.cpp
  tp_math_utils/src/materials/OpenGLMaterial.cpp
  tp_math_utils/src/materials/LegacyMaterial.cpp
  tp_math_utils/src/materials/Blender4Material.cpp
  tp_math_utils/src/materials/ExternalMaterial.cpp)
set_target_properties(smartflow_scene_math PROPERTIES AUTOMOC OFF)
target_include_directories(smartflow_scene_math PUBLIC
  tp_math_utils/inc lib_glm/inc lib_nanoflann/inc)
target_link_libraries(smartflow_scene_math PUBLIC smartflow_legacy)
target_compile_definitions(smartflow_scene_math PUBLIC GLM_ENABLE_EXPERIMENTAL)
