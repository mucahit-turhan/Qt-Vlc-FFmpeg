QT       += core gui quick qml network positioning location

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# Workaround for QTBUG-38735
QT_FOR_CONFIG += location-private
qtConfig(geoservices_mapboxgl): QT += sql
qtConfig(geoservices_osm): QT += concurrent

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#FFmpeg icin lazim olabilir
#QMAKE_CXXFLAGS += -D__STDC_CONSTANT_MACROS

LIBS += -L$$PWD/3rdparty/VLC/bin
LIBS += -laxvlc -llibvlc -llibvlccore -lnpvlc
LIBS += -L$$PWD/3rdparty/FFMPEG/lib -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale
INCLUDEPATH += $$PWD/3rdparty/VLC/include
INCLUDEPATH += $$PWD/3rdparty/FFMPEG/include

SOURCES += \
    ffmpeg.cpp \
    main.cpp \
    qtplayer.cpp

HEADERS += \
    ffmpeg.h \
    qtplayer.h

FORMS += \
    qtplayer.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    doc/images/mapviewer.png \
    doc/src/mapviewer.qdoc \
    forms/Geocode.qml \
    forms/GeocodeForm.ui.qml \
    forms/Locale.qml \
    forms/LocaleForm.ui.qml \
    forms/Message.qml \
    forms/MessageForm.ui.qml \
    forms/ReverseGeocode.qml \
    forms/ReverseGeocodeForm.ui.qml \
    forms/RouteAddress.qml \
    forms/RouteAddressForm.ui.qml \
    forms/RouteCoordinate.qml \
    forms/RouteCoordinateForm.ui.qml \
    forms/RouteList.qml \
    forms/RouteListDelegate.qml \
    forms/RouteListHeader.qml \
    helper.js \
    map/CircleItem.qml \
    map/ImageItem.qml \
    map/MapComponent.qml \
    map/MapSliders.qml \
    map/Marker.qml \
    map/MiniMap.qml \
    map/PolygonItem.qml \
    map/PolylineItem.qml \
    map/RectangleItem.qml \
    mapviewer.qml \
    menus/ItemPopupMenu.qml \
    menus/MainMenu.qml \
    menus/MapPopupMenu.qml \
    menus/MarkerPopupMenu.qml \
    resources/icon.png \
    resources/marker.png \
    resources/scale.png \
    resources/scale_end.png

RESOURCES += \
    mapviewer.qrc
