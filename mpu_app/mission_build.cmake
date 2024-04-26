###########################################################
#
# MPU_APP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the MPU_APP configuration
set(MPU_APP_MISSION_CONFIG_FILE_LIST
  mpu_app_fcncodes.h
  mpu_app_interface_cfg.h
  mpu_app_mission_cfg.h
  mpu_app_perfids.h
  mpu_app_msg.h
  mpu_app_msgdefs.h
  mpu_app_msgstruct.h
  mpu_app_tbl.h
  mpu_app_tbldefs.h
  mpu_app_tblstruct.h
  mpu_app_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(MPU_APP_CFGFILE_SRC_mpu_app_interface_cfg "mpu_app_eds_designparameters.h")
  set(MPU_APP_CFGFILE_SRC_mpu_app_tbldefs       "mpu_app_eds_typedefs.h")
  set(MPU_APP_CFGFILE_SRC_mpu_app_tblstruct     "mpu_app_eds_typedefs.h")
  set(MPU_APP_CFGFILE_SRC_mpu_app_msgdefs       "mpu_app_eds_typedefs.h")
  set(MPU_APP_CFGFILE_SRC_mpu_app_msgstruct     "mpu_app_eds_typedefs.h")
  set(MPU_APP_CFGFILE_SRC_mpu_app_fcncodes      "mpu_app_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(MPU_APP_CFGFILE ${MPU_APP_MISSION_CONFIG_FILE_LIST})
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
