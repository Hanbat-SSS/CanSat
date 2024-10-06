project(CFE_SEED_CTR_LIB C)

# cam_app의 소스 파일
set(APP_SRC_FILES
  fsw/src/seed_ctr_lib.c
)

# 애플리케이션 모듈을 생성
add_cfe_app(seed_ctr_lib ${APP_SRC_FILES})

# The API to this library (which may be invoked/referenced from other apps)
# is stored in fsw/public_inc.  Using "target_include_directories" is the 
# preferred method of indicating this (vs. directory-scope "include_directories").
target_include_directories(seed_ctr_lib PUBLIC fsw/public_inc)


