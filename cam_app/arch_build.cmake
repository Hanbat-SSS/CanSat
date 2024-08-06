###########################################################
#
# arch_build.cmake
#
# CAM_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the CAM_APP configuration
set(CAM_APP_PLATFORM_CONFIG_FILE_LIST
  cam_app_internal_cfg.h
  cam_app_platform_cfg.h
  cam_app_perfids.h
  cam_app_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(CAM_APP_CFGFILE ${CAM_APP_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${CAM_APP_CFGFILE}" NAME_WE)
  if (DEFINED CAM_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${CAM_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${CAM_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${CAM_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
