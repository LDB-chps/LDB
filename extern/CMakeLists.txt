
include(FetchContent)

# Utility library for logging and versioning
FetchContent_Declare(
        tscl
        GIT_REPOSITORY https://github.com/Thukisdo/tscl.git
        GIT_TAG 8ef22ba3cd20aa618e0eeb6169e71c760a74be44 # pre-release-0.1.0.alpha
)

FetchContent_MakeAvailable(tscl)
