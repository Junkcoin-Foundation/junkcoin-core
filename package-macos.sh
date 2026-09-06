#!/usr/bin/env bash
#
# Package junkcoin-qt into a self-contained, signed macOS .app + .dmg.
#
# Uses Qt's official `macdeployqt` (the in-tree macdeployqtplus cannot resolve
# modern Qt @rpath deps), then manually resolves the Boost @loader_path closure
# that macdeployqt misses (e.g. libboost_atomic).
#
set -euo pipefail

eval "$(/opt/homebrew/bin/brew shellenv)"
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"

REPO="$(cd "$(dirname "$0")" && pwd)"
cd "$REPO"

APP="Junkcoin-Qt.app"
APP_BIN="src/qt/junkcoin-qt"
VOLNAME="Junkcoin Core"
DMG="Junkcoin-Qt.dmg"
BOOST_LIBDIR="/opt/homebrew/opt/boost/lib"

[ -x "$APP_BIN" ] || { echo "error: $APP_BIN not built; run make first" >&2; exit 1; }

echo "==> Assembling $APP"
rm -rf "$APP" dist dmgstage "$DMG"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
cp share/qt/Info.plist "$APP/Contents/Info.plist"
printf 'APPL????' > "$APP/Contents/PkgInfo"
cp src/qt/res/icons/bitcoin.icns "$APP/Contents/Resources/bitcoin.icns"
cp "$APP_BIN" "$APP/Contents/MacOS/Junkcoin-Qt"

echo "==> Running macdeployqt (bundling Qt frameworks + plugins)"
macdeployqt "$APP" -verbose=1

echo "==> Resolving Boost @loader_path dependency closure"
FW="$APP/Contents/Frameworks"
for _ in 1 2 3 4 5 6; do
  added=0
  for f in "$FW"/*.dylib; do
    while IFS= read -r ref; do
      base="$(basename "$ref")"
      if [ ! -f "$FW/$base" ]; then
        if [ -f "$BOOST_LIBDIR/$base" ]; then
          cp "$BOOST_LIBDIR/$base" "$FW/$base"
          chmod u+w "$FW/$base"
          install_name_tool -id "@loader_path/$base" "$FW/$base"
          echo "    + added $base"
          added=1
        else
          echo "    ! missing and not in brew: $base" >&2
        fi
      fi
    done < <(otool -L "$f" 2>/dev/null | grep -o '@loader_path/libboost_[^ ]*' || true)
  done
  [ "$added" -eq 0 ] && break
done

echo "==> Ad-hoc code signing"
find "$FW" -name '*.dylib' -exec codesign --force --sign - {} \;
codesign --force --deep --sign - "$APP"
codesign --verify --deep "$APP" && echo "    signature OK"

echo "==> Building $DMG"
mkdir dmgstage
cp -R "$APP" dmgstage/
ln -s /Applications dmgstage/Applications
hdiutil create -volname "$VOLNAME" -srcfolder dmgstage -ov -format UDZO "$DMG" >/dev/null
rm -rf dmgstage
echo "==> Done: $REPO/$DMG"
ls -la "$DMG"
