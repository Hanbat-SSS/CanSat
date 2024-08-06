
# CMakeLists.txt

project(CFE_CAM_APP C)

# cam_app의 소스 파일
set(APP_SRC_FILES
  fsw/src/cam_app.c
  fsw/src/cam_app_cmds.c
  fsw/src/cam_app_utils.c
  fsw/src/common_fnc.c
  ../../libs/Security_lib/fsw/src/security.c
)

# CFE_EDS_ENABLED_BUILD에 따라 조건부 소스 파일 추가
if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/cam_app_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/cam_app_dispatch.c
  )
endif()

# 애플리케이션 모듈을 생성
add_cfe_app(cam_app ${APP_SRC_FILES})

# cam_app의 인클루드 디렉토리 설정
target_include_directories(cam_app PUBLIC fsw/inc)

# Security_lib의 인클루드 디렉토리 설정
target_include_directories(cam_app PUBLIC ../../libs/Security_lib/fsw/public_inc)
target_include_directories(cam_app PUBLIC ../../libs/Security_lib/fsw/src)
target_include_directories(cam_app PUBLIC ../../apps/cam_app/fsw/src)

# 테이블을 추가
add_cfe_tables(cam_app fsw/tables/cam_app_tbl.c)


# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.

