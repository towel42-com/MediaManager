set(_PROJECT_NAME MediaManager)
set(FOLDER_NAME Apps)

set(qtproject_SRCS
    main.cpp    
)

set(qtproject_H
)

set(project_H
)

set(qtproject_UIS
)


set(qtproject_QRC
)

set( project_pub_DEPS
        Towel42Utils
        Towel42MediaUtils
        UI
        PreferencesUI
        PreferencesCore
        Core
        Models
)

SET( project_pri_DEPS
    Qt6::Core
    ${project_pri_DEPS}
)