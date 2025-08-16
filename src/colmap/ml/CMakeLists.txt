set(ML_SOURCES
    ml_manager.cc
    superpoint_detector.cc
    superglue_matcher.cc
    loftr_matcher.cc
    disk_detector.cc
)

set(ML_HEADERS
    ml_manager.h
    superpoint_detector.h
    superglue_matcher.h
    loftr_matcher.h
    disk_detector.h
)

# Create the ML library
add_library(colmap_ml STATIC ${ML_SOURCES} ${ML_HEADERS})

# Set target properties
set_target_properties(colmap_ml PROPERTIES
    COMPILE_FLAGS "${COLMAP_COMPILE_FLAGS}"
    POSITION_INDEPENDENT_CODE ON
)

# Link dependencies
target_link_libraries(colmap_ml
    colmap_util
    colmap_feature
    colmap_sensor
    ${EIGEN3_LIBRARIES}
)

# Include directories
target_include_directories(colmap_ml PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
    $<INSTALL_INTERFACE:include>
)

# Install headers
install(FILES ${ML_HEADERS}
    DESTINATION include/colmap/ml
)
