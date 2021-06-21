set( BOOST_ROOT "C:/Tools/Boost/" CACHE PATH "Boost library path" )

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost REQUIRED COMPONENTS filesystem)

if (Boost_FOUND)
    list(APPEND vkcv_includes ${Boost_INCLUDE_DIR})
    list(APPEND vkcv_libraries ${Boost_LIBRARIES})

    message(${vkcv_config_msg} " Boost  -   ")
endif ()
