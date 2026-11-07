# ---------------------------------------------------------------------------
# edu_core: database, repositories and services. No Gui or Widgets here, so
# every rule can be tested without a window.
# ---------------------------------------------------------------------------
add_library(edu_core STATIC
    core/Result.h
    core/Database.h            core/Database.cpp
    core/Transaction.h         core/Transaction.cpp
    core/SqlScript.h           core/SqlScript.cpp
    core/Migrator.h            core/Migrator.cpp
    core/Logger.h              core/Logger.cpp
    repositories/SettingsRepository.h  repositories/SettingsRepository.cpp
    services/SystemService.h   services/SystemService.cpp
)
target_include_directories(edu_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(edu_core PUBLIC Qt6::Core Qt6::Sql)
target_compile_definitions(edu_core PUBLIC EDUHUB_VERSION="${PROJECT_VERSION}")

# Migrations are compiled into the library: :/sql/migrations/NNN_name.sql
file(GLOB EDU_MIGRATIONS CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/resources/sql/migrations/*.sql")
qt_add_resources(edu_core "edu_core_sql"
    PREFIX "/"
    BASE "${PROJECT_SOURCE_DIR}/resources"
    FILES ${EDU_MIGRATIONS})

# ---------------------------------------------------------------------------
# edu_ui: windows, pages and widgets. Linked by the app and by tst_gui.
# ---------------------------------------------------------------------------
add_library(edu_ui STATIC
    ui/UiSettings.h            ui/UiSettings.cpp
    ui/Fonts.h                 ui/Fonts.cpp
    ui/AppController.h         ui/AppController.cpp
    ui/theme/Theme.h           ui/theme/Theme.cpp
    ui/theme/Icons.h           ui/theme/Icons.cpp
    ui/widgets/IconLabel.h     ui/widgets/IconLabel.cpp
    ui/widgets/NavRail.h       ui/widgets/NavRail.cpp
    ui/widgets/HeaderBar.h     ui/widgets/HeaderBar.cpp
    ui/widgets/Toast.h         ui/widgets/Toast.cpp
    ui/widgets/StatCard.h      ui/widgets/StatCard.cpp
    ui/widgets/UiHelpers.h     ui/widgets/UiHelpers.cpp
    ui/login/BrandPanel.h      ui/login/BrandPanel.cpp
    ui/login/LoginWindow.h     ui/login/LoginWindow.cpp
    ui/pages/DashboardPage.h   ui/pages/DashboardPage.cpp
    ui/pages/EmptyStatePage.h  ui/pages/EmptyStatePage.cpp
    ui/main/MainWindow.h       ui/main/MainWindow.cpp
)
target_include_directories(edu_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(edu_ui PUBLIC edu_core Qt6::Gui Qt6::Widgets Qt6::Svg)

file(GLOB EDU_ICONS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/resources/icons/*.svg")
file(GLOB EDU_FONTS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/resources/fonts/*.ttf")
qt_add_resources(edu_ui "edu_ui_assets"
    PREFIX "/"
    BASE "${PROJECT_SOURCE_DIR}/resources"
    FILES
        "${PROJECT_SOURCE_DIR}/resources/styles/app.qss"
        ${EDU_ICONS}
        ${EDU_FONTS})

# ---------------------------------------------------------------------------
# The application (EduHub.exe)
# ---------------------------------------------------------------------------
qt_add_executable(eduhub_app main.cpp)
target_link_libraries(eduhub_app PRIVATE edu_ui)
set_target_properties(eduhub_app PROPERTIES
    OUTPUT_NAME EduHub
    WIN32_EXECUTABLE ON)
