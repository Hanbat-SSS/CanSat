###########################################################
#
# CAM_APP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the CAM_APP configuration
set(CAM_APP_MISSION_CONFIG_FILE_LIST
  cam_app_fcncodes.h
  cam_app_interface_cfg.h
  cam_app_mission_cfg.h
  cam_app_perfids.h
  cam_app_msg.h
  cam_app_msgdefs.h
  cam_app_msgstruct.h
  cam_app_tbl.h
  cam_app_tbldefs.h
  cam_app_tblstruct.h
  cam_app_topicids.h

)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(CAM_APP_CFGFILE_SRC_cam_app_interface_cfg "cam_app_eds_designparameters.h")
  set(CAM_APP_CFGFILE_SRC_cam_app_tbldefs       "cam_app_eds_typedefs.h")
  set(CAM_APP_CFGFILE_SRC_cam_app_tblstruct     "cam_app_eds_typedefs.h")
  set(CAM_APP_CFGFILE_SRC_cam_app_msgdefs       "cam_app_eds_typedefs.h")
  set(CAM_APP_CFGFILE_SRC_cam_app_msgstruct     "cam_app_eds_typedefs.h")
  set(CAM_APP_CFGFILE_SRC_cam_app_fcncodes      "cam_app_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(CAM_APP_CFGFILE ${CAM_APP_MISSION_CONFIG_FILE_LIST})
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
