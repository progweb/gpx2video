FIND_PROGRAM(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
FIND_PROGRAM(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

set(POT_FILE "gpx2video.pot")
set(POT_SOURCE_FILES "POTFILES")
set(POT_PKGNAME "gpx2video")
set(POT_COPYRIGHT "gpx2video team")
set(POT_CONTACT "https://github.com/gpx2video")
set(LANGUAGES en fr)


if (GETTEXT_XGETTEXT_EXECUTABLE)
	add_custom_target(
		pot-update
		COMMENT "pot-update: Done."
		DEPENDS ${POT_FILE}
	)

	add_custom_command(
		TARGET pot-update
		PRE_BUILD
		COMMAND
			${GETTEXT_XGETTEXT_EXECUTABLE}
			--from-code=utf-8
			--output=${POT_FILE}
			--keyword="_"
			--files-from=${POT_SOURCE_FILES}
			--package-name="${POT_PKGNAME}"
			--copyright-holder="${POT_COPYRIGHT}"
			--msgid-bugs-address="${POT_CONTACT}"
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "pot-update: Pot file generated: ${POT_FILE}"
	)
endif (GETTEXT_XGETTEXT_EXECUTABLE)

if (GETTEXT_MSGMERGE_EXECUTABLE)
	file(GLOB PO_FILES *.po)
	message(TRACE "  PO_FILES: ${PO_FILES}")

	add_custom_target(
		pot-merge
		COMMENT "pot-merge: Done."
		DEPENDS ${POT_FILE}
	)

	foreach(PO_FILE IN ITEMS ${PO_FILES})
		message(STATUS "  Adding msgmerge for: ${PO_FILE}")
		add_custom_command(
			TARGET pot-merge
			PRE_BUILD
			COMMAND
				${GETTEXT_MSGMERGE_EXECUTABLE} 
				${PO_FILE}
				${POT_FILE}
				--output=${PO_FILE}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT "pot-merge: ${PO_FILE}"
		)
	endforeach()
endif (GETTEXT_MSGMERGE_EXECUTABLE)

if (GETTEXT_MSGFMT_EXECUTABLE)
	file(GLOB PO_FILES *.po)
	message(TRACE "  PO_FILES: ${PO_FILES}")


	add_custom_target(
		pot-build
		COMMENT "pot-build: Done."
		DEPENDS ${POT_FILE}
	)

	foreach(PO_FILE IN ITEMS ${PO_FILES})
		message(STATUS "  Adding msgfmt for: ${PO_FILE}")
		get_filename_component(LANG ${PO_FILE} NAME_WE)
		make_directory(${CMAKE_CURRENT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES)
		add_custom_command(
			TARGET pot-build
			PRE_BUILD
			COMMAND
			${GETTEXT_MSGFMT_EXECUTABLE} 
				${PO_FILE}
				--output-file=${CMAKE_CURRENT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES/${POT_PKGNAME}.mo
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMENT "pot-build: ${PO_FILE}"
		)
	endforeach()
endif (GETTEXT_MSGFMT_EXECUTABLE)

