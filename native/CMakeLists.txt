file(GLOB QTNODES_SOURCES CONFIGURE_DEPENDS vendor/qtnodes/src/*.cpp)
file(GLOB QTNODES_HEADERS CONFIGURE_DEPENDS vendor/qtnodes/inc/QtNodes/internal/*.hpp)
add_library(smartflow_qtnodes STATIC ${QTNODES_SOURCES} ${QTNODES_HEADERS})
target_include_directories(smartflow_qtnodes PUBLIC vendor/qtnodes/inc)
target_compile_definitions(smartflow_qtnodes PUBLIC SMARTFLOW_QTNODES_STATIC)
target_link_libraries(smartflow_qtnodes PUBLIC Qt6::Widgets Qt6::OpenGLWidgets)
add_subdirectory(vendor/legacy)
add_library(smartflow_pipeline STATIC app/pipeline/PipelineExecution.cpp)
target_include_directories(smartflow_pipeline PUBLIC app)
target_link_libraries(smartflow_pipeline PUBLIC smartflow_legacy)
add_library(smartflow_pipeline_widgets STATIC
  vendor/pipeline-widgets/src/AbstractParameterEditor.cpp
  vendor/pipeline-widgets/src/StepDelegateNodeDelegateModel.cpp
  vendor/pipeline-widgets/src/parameter_editors/DoubleParameterEditor.cpp
  vendor/pipeline-widgets/inc/tp_qt_pipeline_widgets/AbstractParameterEditor.h
  vendor/pipeline-widgets/inc/tp_qt_pipeline_widgets/StepDelegateNodeDelegateModel.h
  vendor/pipeline-widgets/inc/tp_qt_pipeline_widgets/parameter_editors/DoubleParameterEditor.h)
target_include_directories(smartflow_pipeline_widgets PUBLIC vendor/pipeline-widgets/inc)
target_link_libraries(smartflow_pipeline_widgets PUBLIC smartflow_legacy smartflow_qtnodes)
add_library(smartflow_workspace STATIC
  app/workspace/GraphProject.cpp app/workspace/GraphProject.h
  app/workspace/PipelineCanvas.cpp app/workspace/PipelineCanvas.h
  app/workspace/ExecutionController.cpp app/workspace/ExecutionController.h
  app/workspace/WorkspaceWindow.cpp app/workspace/WorkspaceWindow.h)
target_include_directories(smartflow_workspace PUBLIC app)
target_link_libraries(smartflow_workspace PUBLIC smartflow_pipeline smartflow_pipeline_widgets)
add_executable(smartflow WIN32 app/main.cpp)
target_include_directories(smartflow PRIVATE app)
target_link_libraries(smartflow PRIVATE smartflow_workspace)
if(BUILD_TESTING)
  add_executable(workspace_tests tests/WorkspaceTests.cpp)
  target_link_libraries(workspace_tests PRIVATE smartflow_workspace Qt6::Test)
  add_test(NAME native_workspace COMMAND workspace_tests -platform offscreen -o -,txt -o workspace-results.xml,junitxml)
  set_tests_properties(native_workspace PROPERTIES TIMEOUT 60)
  add_executable(pipeline_tests tests/PipelineTests.cpp)
  target_link_libraries(pipeline_tests PRIVATE smartflow_pipeline Qt6::Test)
  add_test(NAME native_pipeline COMMAND pipeline_tests -o -,txt -o pipeline-results.xml,junitxml)
  set_tests_properties(native_pipeline PROPERTIES TIMEOUT 60)
  add_executable(native_tests tests/GraphTests.cpp app/NumberNode.h)
  target_include_directories(native_tests PRIVATE app)
  target_link_libraries(native_tests PRIVATE smartflow_qtnodes Qt6::Test)
  add_test(NAME native_graph COMMAND native_tests -platform offscreen)
endif()
