add_subdirectory(Inc)
add_library(gnss STATIC)

target_sources(gnss PUBLIC Src/GNSS.cpp Src/minmea.c)

target_link_libraries(gnss PUBLIC gnss_inc etl::etl log_common)