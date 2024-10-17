add_subdirectory(Inc)
add_library(gnss STATIC)

target_sources(gnss PRIVATE Src/GNSS.cpp)

target_link_libraries(gnss PUBLIC gnss_inc etl::etl)