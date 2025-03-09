# Add subdirectories for Helpers, Services, and Time
add_subdirectory(Helpers)
add_subdirectory(Services)
add_subdirectory(Time)

# Create the ecss_headers interface library
add_library(ecss_headers INTERFACE)

# Include the current directory in the interface library
target_include_directories(ecss_headers INTERFACE .)


target_link_libraries(ecss_headers INTERFACE ecss_time_inc ecss_services_inc ecss_helpers_inc)
