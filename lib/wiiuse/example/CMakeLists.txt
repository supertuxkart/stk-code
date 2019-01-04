include_directories(../src)
add_executable(wiiuseexample example.c)
target_link_libraries(wiiuseexample wiiuse)

if(INSTALL_EXAMPLES)
	install(TARGETS wiiuseexample
		RUNTIME DESTINATION bin COMPONENT examples)
endif()