file(GLOB icons "*.png")
ecm_install_icons(
    ICONS ${icons}
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)
