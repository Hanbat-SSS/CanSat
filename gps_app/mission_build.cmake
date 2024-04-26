###########################################################
#
# GPS_APP mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the GPS_APP configuration
set(GPS_APP_MISSION_CONFIG_FILE_LIST
  gps_app_fcncodes.h
  gps_app_interface_cfg.h
  gps_app_mission_cfg.h
  gps_app_perfids.h
  gps_app_msg.h
  gps_app_msgdefs.h
  gps_app_msgstruct.h
  gps_app_tbl.h
  gps_app_tbldefs.h
  gps_app_tblstruct.h
  gps_app_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(GPS_APP_CFGFILE_SRC_gps_app_interface_cfg "gps_app_eds_designparameters.h")
  set(GPS_APP_CFGFILE_SRC_gps_app_tbldefs       "gps_app_eds_typedefs.h")
  set(GPS_APP_CFGFILE_SRC_gps_app_tblstruct     "gps_app_eds_typedefs.h")
  set(GPS_APP_CFGFILE_SRC_gps_app_msgdefs       "gps_app_eds_typedefs.h")
  set(GPS_APP_CFGFILE_SRC_gps_app_msgstruct     "gps_app_eds_typedefs.h")
  set(GPS_APP_CFGFILE_SRC_gps_app_fcncodes      "gps_app_eds_cc.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(GPS_APP_CFGFILE ${GPS_APP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${GPS_APP_CFGFILE}" NAME_WE)
  if (DEFINED GPS_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${GPS_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${GPS_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${GPS_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
