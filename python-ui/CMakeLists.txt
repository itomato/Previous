
INSTALL(PROGRAMS previousui
	DESTINATION ${BINDIR})

INSTALL(PROGRAMS config.py dialogs.py previous.py uihelpers.py
	previousui.py debugui.py
	DESTINATION ${DATADIR}/previousui/)

INSTALL(FILES README TODO release-notes.txt previous-icon.png previous.png
	DESTINATION ${DATADIR}/previousui/)

INSTALL(FILES previous-icon.png
	DESTINATION share/icons/hicolor/32x32/apps)

INSTALL(FILES previousui.desktop
	DESTINATION share/applications)

# if(UNIX)
	add_custom_target(previousui_man ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/previousui.1.gz)
	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/previousui.1.gz
		COMMAND gzip -c -9 ${CMAKE_CURRENT_SOURCE_DIR}/previousui.1 > ${CMAKE_CURRENT_BINARY_DIR}/previousui.1.gz
		DEPENDS previousui.1)
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/previousui.1.gz DESTINATION ${MANDIR})
# endif(UNIX)
