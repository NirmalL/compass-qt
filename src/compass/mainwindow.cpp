/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include <QtGui>
#include <QtDeclarative>
#include <QCompass>
#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>
#include <QOrientationSensor>
#include <QSensor>

#include "arc.h"
#include "compassfilter.h"
#include "declarativeview.h"
#include "mainwindow.h"
#include "orientationfilter.h"
#include "screensaverinhibiter.h"

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

QTM_USE_NAMESPACE


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_view = new DeclarativeView(this);

    m_orientationSensor = new QOrientationSensor(this);
    m_orientationFilter = new OrientationFilter(this);
    m_orientationSensor->addFilter(m_orientationFilter);
    m_orientationSensor->start();

    m_compass = new QCompass(this);
    m_compassFilter = new CompassFilter(this);
    m_compass->addFilter(m_compassFilter);

    // Try to start the compass sensor.
    bool compassStarted = m_compass->start();

    m_screenSaverInhibiter = new ScreenSaverInhibiter(this);

    m_geoPositionInfoSource = QGeoPositionInfoSource::createDefaultSource(this);
    m_geoPositionInfoSource->setUpdateInterval(5000);

    qmlRegisterType<Arc>("CustomElements", 1, 0, "Arc");
    setCentralWidget(m_view);

#ifndef QT_NO_OPENGL
    // Use QGLWidget to get the OpenGL support if available.
    QGLFormat format = QGLFormat::defaultFormat();
    format.setSampleBuffers(false);

    QGLWidget *glWidget = new QGLWidget(format);
    glWidget->setAutoFillBackground(false);
    m_view->setViewport(glWidget); // Ownership of glWidget is taken
#endif

    // Tell the QML side the path the app exist, this will be used to find out
    // the beep.wav which is used in calibration.
#ifdef Q_WS_HARMATTAN
    m_view->rootContext()->setContextProperty("appFolder", "file://"
                                              + qApp->applicationDirPath()
                                              + QDir::separator());
#else
    m_view->rootContext()->setContextProperty("appFolder",
                                              m_view->engine()->baseUrl()
                                                .toString());
#endif

    m_view->setSource(QUrl("qrc:/qml/Ui.qml"));
    m_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    QObject *rootObject = dynamic_cast<QObject*>(m_view->rootObject());

    if (!compassStarted) {
        // Failed to start the compass sensor.
        QMetaObject::invokeMethod(rootObject, "displayNoCompassNote");
    }

    // Sensor connections
    connect(m_orientationFilter, SIGNAL(orientationChanged(const QVariant&)),
            rootObject, SLOT(orientationChanged(const QVariant&)));
    connect(m_compassFilter,
            SIGNAL(azimuthChanged(const QVariant&, const QVariant&)),
            rootObject, SLOT(handleAzimuth(const QVariant&, const QVariant&)));
    connect(rootObject, SIGNAL(inhibitScreensaver(const QVariant&)),
            m_screenSaverInhibiter, SLOT(screenSaverInhibit(const QVariant&)));

    // Multitouch connections
    connect(m_view, SIGNAL(scaleFactor(const QVariant&)),
            rootObject, SLOT(scaleChanged(const QVariant&)));
    connect(m_view, SIGNAL(scaleFactorEnd(const QVariant&)),
            rootObject, SLOT(scaleChangedEnd(const QVariant&)));

    // GPS connections
    connect(m_geoPositionInfoSource,
            SIGNAL(positionUpdated(const QGeoPositionInfo&)),
            this, SLOT(positionUpdated(const QGeoPositionInfo&)));
    connect(m_geoPositionInfoSource, SIGNAL(updateTimeout()),
            rootObject, SLOT(positionTimeout()));
    connect(this, SIGNAL(position(const QVariant&, const QVariant&,
                                  const QVariant&, const QVariant&)),
            rootObject, SLOT(position(const QVariant&, const QVariant&,
                                      const QVariant&, const QVariant&)));

    // Framework connections
    connect((QObject*)m_view->engine(), SIGNAL(quit()),
            qApp, SLOT(quit()));

    // Apply the screensaver inhibiter if requested on startup
    m_screenSaverInhibiter->screenSaverInhibit(
                rootObject->property("screenSaverInhibited"));

    // Query the lask known position
    QGeoPositionInfo geoPositionInfo =
            m_geoPositionInfoSource->lastKnownPosition();

    if (geoPositionInfo.isValid()) {
        QGeoCoordinate coordinate = geoPositionInfo.coordinate();
        emit position(0, coordinate.latitude(), coordinate.longitude(),
                      geoPositionInfo.attribute(
                          QGeoPositionInfo::HorizontalAccuracy));
    }

    // Start the GPS
    m_geoPositionInfoSource->startUpdates();
}


MainWindow::~MainWindow()
{
}


void MainWindow::positionUpdated(const QGeoPositionInfo &update)
{
    qreal accuracy = update.attribute(QGeoPositionInfo::HorizontalAccuracy);

    uint secsFrom1970 = update.timestamp().toTime_t();
    QGeoCoordinate c = update.coordinate();

    emit position(secsFrom1970, c.latitude(), c.longitude(), accuracy);
}
