project(CFE_MPU_APP C)

set(APP_SRC_FILES
  fsw/src/mpu_app.c
  fsw/src/mpu_app_cmds.c
  fsw/src/mpu_app_utils.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/mpu_app_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/mpu_app_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(mpu_app ${APP_SRC_FILES})

target_link_libraries(mpu_app m wiringPi)

# Add table
add_cfe_tables(mpu_app fsw/tables/mpu_app_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
