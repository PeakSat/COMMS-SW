add_subdirectory(inc)

add_library(ecss)

target_include_directories(ecss PRIVATE inc)

# Collect all the .cpp files in the specified directories
file(GLOB SOURCES
        "src/*.cpp"
        "src/Helpers/*.cpp"
        "src/Services/*.cpp"
        "src/Time/*.cpp"
)

# Add the collected source files to the target
target_sources(ecss PUBLIC ${SOURCES})

# Link the necessary libraries
target_link_libraries(ecss PUBLIC ecss_headers etl::etl log_common platform)
