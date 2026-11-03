cmake_minimum_required(VERSION 3.22)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Universal Binary")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS")
endif()

project(ThrowDelay VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(throw_dsp STATIC src/dsp/PhraseDetector.cpp src/dsp/ThrowEngine.cpp)
target_include_directories(throw_dsp PUBLIC src/dsp)
set_target_properties(throw_dsp PROPERTIES POSITION_INDEPENDENT_CODE ON)

enable_testing()
add_executable(throw_tests tests/throw_tests.cpp)
target_link_libraries(throw_tests PRIVATE throw_dsp)
add_test(NAME throw_tests COMMAND throw_tests)

add_subdirectory(libs/JUCE)
add_subdirectory(libs/clap-juce-extensions EXCLUDE_FROM_ALL)

juce_add_plugin(ThrowDelay
    COMPANY_NAME "beatsexuell"
    PLUGIN_MANUFACTURER_CODE Btsx
    PLUGIN_CODE Thrw
    PRODUCT_NAME "Throw Delay"
    ICON_BIG "resources/icon.png"
    BUNDLE_ID com.beatsexuell.throwdelay
    LV2URI "urn:beatsexuell:throwdelay"
    FORMATS VST3 AU LV2 Standalone
    VST3_CATEGORIES Fx Delay
    AU_MAIN_TYPE kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD FALSE)

clap_juce_extensions_plugin(TARGET ThrowDelay
    CLAP_ID "com.beatsexuell.throwdelay"
    CLAP_FEATURES audio-effect delay stereo mono)

target_sources(ThrowDelay PRIVATE src/plugin/PluginProcessor.cpp src/plugin/PluginEditor.cpp)
target_compile_definitions(ThrowDelay PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(ThrowDelay
    PRIVATE throw_dsp juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
