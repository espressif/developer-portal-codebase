idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")

#
# ULP support additions to the component CMakeLists.txt
#
set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_sources "ulp/main.c")

ulp_embed_binary(${ulp_app_name} "${ulp_sources}" "")