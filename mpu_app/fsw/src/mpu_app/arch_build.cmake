###########################################################
#
# MPU_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the MPU_APP configuration
set(MPU_APP_PLATFORM_CONFIG_FILE_LIST
  mpu_app_internal_cfg.h
  mpu_app_platform_cfg.h
  mpu_app_perfids.h
  mpu_app_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(MPU_APP_CFGFILE ${MPU_APP_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${MPU_APP_CFGFILE}" NAME_WE)
  if (DEFINED MPU_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${MPU_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${MPU_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${MPU_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
