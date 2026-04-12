#!/usr/bin/env fish

function usage
    set script_name (basename (status current-filename))
    echo "Uso: fish $script_name [--prefix PATH] [--build-dir PATH] [--deps-dir PATH] [--run]"
    echo
    echo "  --prefix     Prefijo de instalacion (por defecto: ~/.local/ktorrent-jackett-test)"
    echo "  --build-dir  Directorio de build de CMake (por defecto: <repo>/build)"
    echo "  --deps-dir   Donde guardar ECM y Boost locales (por defecto: <build-dir>/_deps)"
    echo "  --run        Ejecuta KTorrent al terminar la instalacion"
end

function fail --argument-names message
    echo "Error: $message" >&2
    exit 1
end

function require_command --argument-names cmd
    if not type -q $cmd
        fail "Falta el comando requerido: $cmd"
    end
end

function expand_path --argument-names raw_path
    if test -z "$raw_path"
        echo $raw_path
        return
    end

    if string match -qr '^~($|/)' -- $raw_path
        echo (string replace -r '^~' $HOME -- $raw_path)
    else
        echo $raw_path
    end
end

argparse 'prefix=' 'build-dir=' 'deps-dir=' run help -- $argv
or begin
    usage
    exit 1
end

if set -q _flag_help
    usage
    exit 0
end

set script_dir (cd (dirname (status current-filename)); and pwd)
or fail "No se pudo determinar el directorio del script"

set repo_root (cd "$script_dir/.."; and pwd)
or fail "No se pudo determinar la raiz del repositorio"

set prefix (expand_path "$HOME/.local/ktorrent-jackett-test")
if set -q _flag_prefix
    set prefix (expand_path "$_flag_prefix")
end

set build_dir (expand_path "$repo_root/build")
if set -q _flag_build_dir
    set build_dir (expand_path "$_flag_build_dir")
end

set deps_dir (expand_path "$build_dir/_deps")
if set -q _flag_deps_dir
    set deps_dir (expand_path "$_flag_deps_dir")
end

set run_after_install 0
if set -q _flag_run
    set run_after_install 1
end

for cmd in git cmake curl tar
    require_command $cmd
end

set jobs 2
if type -q nproc
    set jobs (nproc)
end

set ecm_prefix "$deps_dir/ecm-prefix"
set ecm_source_dir "$deps_dir/extra-cmake-modules"
set ecm_build_dir "$deps_dir/extra-cmake-modules-build"

if not test -f "$ecm_prefix/share/ECM/cmake/ECMConfig.cmake"
    mkdir -p "$deps_dir"
    or fail "No se pudo crear $deps_dir"

    if not test -d "$ecm_source_dir/.git"
        echo "Clonando extra-cmake-modules..."
        git clone --depth 1 https://github.com/KDE/extra-cmake-modules.git "$ecm_source_dir"
        or fail "Fallo al clonar extra-cmake-modules"
    end

    echo "Compilando extra-cmake-modules..."
    cmake -S "$ecm_source_dir" -B "$ecm_build_dir" -DCMAKE_INSTALL_PREFIX="$ecm_prefix"
    or fail "Fallo al configurar extra-cmake-modules"
    cmake --build "$ecm_build_dir" -j $jobs
    or fail "Fallo al compilar extra-cmake-modules"
    cmake --install "$ecm_build_dir"
    or fail "Fallo al instalar extra-cmake-modules"
end

set boost_dir "$deps_dir/boost_1_88_0"
set boost_archive "$deps_dir/boost_1_88_0.tar.bz2"

if not test -f "$boost_dir/boost/version.hpp"
    mkdir -p "$deps_dir"
    or fail "No se pudo crear $deps_dir"

    if not test -f "$boost_archive"
        echo "Descargando Boost..."
        curl -L --fail -o "$boost_archive" https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.bz2
        or fail "Fallo al descargar Boost"
    end

    echo "Extrayendo Boost..."
    tar -xf "$boost_archive" -C "$deps_dir"
    or fail "Fallo al extraer Boost"
end

set cmake_prefix_path "$ecm_prefix"
if set -q CMAKE_PREFIX_PATH
    set cmake_prefix_path "$ecm_prefix;$CMAKE_PREFIX_PATH"
end

echo "Configurando KTorrent..."
cmake -S "$repo_root" -B "$build_dir" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_PREFIX_PATH="$cmake_prefix_path" \
    -DBoost_INCLUDE_DIR="$boost_dir"
or fail "Fallo al configurar KTorrent"

echo "Compilando KTorrent..."
cmake --build "$build_dir" -j $jobs
or fail "Fallo al compilar KTorrent"

echo "Instalando KTorrent en $prefix ..."
cmake --install "$build_dir" --prefix "$prefix"
or fail "Fallo al instalar KTorrent"

set launcher "$prefix/bin/ktorrent-dev"
set escaped_prefix (string escape --style=script -- "$prefix")

mkdir -p "$prefix/bin"
or fail "No se pudo crear $prefix/bin"

begin
    echo '#!/usr/bin/env fish'
    echo "set prefix $escaped_prefix"
    echo 'set -gx PATH "$prefix/bin" $PATH'
    echo 'set -gx LD_LIBRARY_PATH "$prefix/lib" $LD_LIBRARY_PATH'
    echo 'set -gx XDG_DATA_DIRS "$prefix/share" $XDG_DATA_DIRS'
    echo 'set -gx XDG_CONFIG_DIRS "$prefix/etc/xdg" $XDG_CONFIG_DIRS'
    echo 'set -gx QT_PLUGIN_PATH "$prefix/lib/plugins" $QT_PLUGIN_PATH'
    echo 'set -gx QML2_IMPORT_PATH "$prefix/lib/qml" $QML2_IMPORT_PATH'
    echo 'set -gx QT_QUICK_CONTROLS_STYLE_PATH "$prefix/lib/qml/QtQuick/Controls.2" $QT_QUICK_CONTROLS_STYLE_PATH'
    echo 'exec "$prefix/bin/ktorrent" $argv'
end > "$launcher"
or fail "No se pudo escribir el lanzador $launcher"

chmod +x "$launcher"
or fail "No se pudo marcar el lanzador como ejecutable"

set xdg_data_home "$HOME/.local/share"
if set -q XDG_DATA_HOME
    if test -n "$XDG_DATA_HOME"
        set xdg_data_home (expand_path "$XDG_DATA_HOME")
    end
end

set desktop_dir "$xdg_data_home/applications"
set desktop_file "$desktop_dir/org.kde.ktorrent-jackett.desktop"
set icon_path "$prefix/share/icons/hicolor/128x128/apps/ktorrent.png"
if not test -f "$icon_path"
    set icon_path "$prefix/share/icons/hicolor/64x64/apps/ktorrent.png"
end

mkdir -p "$desktop_dir"
or fail "No se pudo crear $desktop_dir"

begin
    echo '[Desktop Entry]'
    echo 'Version=1.0'
    echo 'Type=Application'
    echo 'Name=KTorrent Jackett'
    echo 'GenericName=BitTorrent Client'
    echo 'Comment=KTorrent with Jackett/Torznab integration'
    echo "Exec=$launcher %U"
    echo "TryExec=$launcher"
    echo "Icon=$icon_path"
    echo 'Terminal=false'
    echo 'StartupNotify=true'
    echo 'StartupWMClass=ktorrent'
    echo 'Categories=Qt;KDE;Network;FileTransfer;P2P;'
    echo 'MimeType=application/x-bittorrent;application/x-torrent;x-scheme-handler/magnet;'
end > "$desktop_file"
or fail "No se pudo escribir el lanzador de escritorio $desktop_file"

if type -q update-desktop-database
    update-desktop-database "$desktop_dir" >/dev/null 2>/dev/null
end

echo
echo "Listo."
echo "Instalado en: $prefix"
echo "Para abrirlo: $launcher"
echo "Lanzador de GNOME: $desktop_file"

if test $run_after_install -eq 1
    exec "$launcher"
end
