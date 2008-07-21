#!/bin/sh
cat <<EOF
[Desktop Entry]
Version=1.0
Type=Application
Name=QTM
GenericName=Blog editor
Comment=Weblog management application
TryExec=$1/bin/qtm
Exec=$1/bin/qtm &
Categories=Application;Network;X-MandrivaLinux-Internet-Other;
Icon=$1/share/icons/qtm-logo1.png
# MimeType=image/x-foo;
# X-KDE-Library=libfooview
# X-KDE-FactoryName=fooviewfactory
# X-KDE-ServiceType=FooService
EOF
