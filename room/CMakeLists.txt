cmake_minimum_required(VERSION 3.22)

project(RoomCorrector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# JUCE 8.0.4 recupere automatiquement (pas besoin de checkout separe)
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ---------------------------------------------------------------------------
# TRIAL / FULL : un seul code, un interrupteur.
#   cmake -B build                     → version COMPLETE  "Room Corrector"
#   cmake -B build-trial -DRC_TRIAL=ON → version D'ESSAI   "Room Corrector Trial"
# Codes plugin differents → les deux peuvent cohabiter dans le meme DAW.
# ---------------------------------------------------------------------------
option(RC_TRIAL "Build the trial (demo) version" OFF)
if(RC_TRIAL)
    set(RC_PRODUCT_NAME "Room Corrector Trial")
    set(RC_PLUGIN_CODE  RcTr)
else()
    set(RC_PRODUCT_NAME "Room Corrector")
    set(RC_PLUGIN_CODE  Rcor)
endif()

# ---------------------------------------------------------------------------
# Definition du plugin
# ---------------------------------------------------------------------------
juce_add_plugin(RoomCorrector
    PRODUCT_NAME              "${RC_PRODUCT_NAME}"
    COMPANY_NAME             "Uncertain"
    PLUGIN_MANUFACTURER_CODE  Uncr
    PLUGIN_CODE               ${RC_PLUGIN_CODE}
    FORMATS                   VST3 Standalone
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    VST3_CATEGORIES           Fx
)

if(RC_TRIAL)
    target_compile_definitions(RoomCorrector PRIVATE RC_TRIAL=1)
endif()

# Genere JuceHeader.h (OBLIGATOIRE : les sources font #include <JuceHeader.h>)
juce_generate_juce_header(RoomCorrector)

# Fichiers source (a la RACINE du repo)
target_sources(RoomCorrector PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
)

target_compile_definitions(RoomCorrector PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(RoomCorrector
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
