project(CFE_GPS_APP C)

set(APP_SRC_FILES
  fsw/src/gps_app.c
  fsw/src/gps_app_cmds.c
  fsw/src/gps_app_utils.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/gps_app_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/gps_app_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(gps_app ${APP_SRC_FILES})


#target_include_directories(gps_app PUBLIC fsw/inc)
# Include the public API from gps_lib to demonstrate how
# to call library-provided functions
#add_cfe_app_dependency(gps_app gps_lib)

# Add table
add_cfe_tables(gps_app fsw/tables/gps_app_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
