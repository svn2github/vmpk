SET( TRANSLATIONS_FILES 
     vmpk_cs.ts 
     vmpk_de.ts 
     vmpk_es.ts 
     vmpk_fr.ts
     vmpk_ru.ts
     vmpk_tr.ts 
)

QT4_ADD_TRANSLATION(QM_FILES ${TRANSLATIONS_FILES})
ADD_CUSTOM_TARGET(translations ALL DEPENDS ${QM_FILES})

IF (UNIX AND NOT APPLE)
    INSTALL( FILES ${QM_FILES}
             DESTINATION share/locale )
ENDIF (UNIX AND NOT APPLE)

IF (WIN32)
    INSTALL( FILES ${QM_FILES}
             DESTINATION . )
ENDIF (WIN32)
