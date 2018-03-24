
set(SOURCES
  ${SOURCES_PREFIX}/boot_board_config.c
  ${SOURCES_PREFIX}/board_config.h
	${SOURCES_PREFIX}/../src/board_arch_config.c
	${SOURCES_PREFIX}/../src/board_arch_config.h
  ${SOURCES_PREFIX}/link_transport.c
  ${SOURCES_PREFIX}/link_transport.h
  PARENT_SCOPE)
