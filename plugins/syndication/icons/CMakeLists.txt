file(GLOB icons "*.png")
ecm_install_icons(
    ICONS ${icons}
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)
