/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

#include <QDebug>
#include <QDomDocument>
#include <QDeclarativeItem>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include "persistentstorage.h"


/*!
  \class PersistenStorage
  \brief Loads / stores settings to the persistent storage of the device,
         saves / loads the walked route to the KML file format.
*/

// The file name of the stored KML file.
#ifdef Q_OS_SYMBIAN
const QString PersistentStorage::m_KmlFilePath = QString("E:\\Compass.kml");
#else
const QString PersistentStorage::m_KmlFilePath = QString("/home/user/Compass.kml");
#endif


/*!
  Constructor.
*/
PersistentStorage::PersistentStorage(QObject *parent)
    : QObject(parent)
{
    m_Settings = new QSettings("Nokia", "Compass", this);

    m_DomDocument = new QDomDocument;
}


/*!
  Destructor.
*/
PersistentStorage::~PersistentStorage()
{
    if (m_DomDocument) {
        delete m_DomDocument;
        m_DomDocument = 0;
    }
}


/*!
  Saves a given key - value setting by using QSetting to devices persistent
  storage.
*/
void PersistentStorage::saveSetting(const QVariant &key, const QVariant &value)
{
    m_Settings->setValue(key.toString(), value);
}


/*!
  Loads setting from the devices persistent storage, the key defines the
  setting to load, defaultValue is the value for the key if the value
  not yet exist in the persistent storage.
*/
QVariant PersistentStorage::loadSetting(const QVariant &key,
                                        const QVariant &defaultValue)
{
    return m_Settings->value(key.toString(), defaultValue);
}


/*!
  Creates KML document with common elements.
*/
void PersistentStorage::createDocument()
{
    m_DomDocument->clear();

    QDomProcessingInstruction instr =
            m_DomDocument->createProcessingInstruction(
                "xml",
                "version=\"1.0\" encoding=\"UTF-8\"");
    m_DomDocument->appendChild(instr);

    // Create "kml"
    QDomElement kml = m_DomDocument->createElement("kml");
    kml.setAttribute("xmlns", "http://www.opengis.net/kml/2.2");
    m_DomDocument->appendChild(kml);

    // Create "Document"
    m_Document = m_DomDocument->createElement("Document");
    kml.appendChild(m_Document);

    // Create "name"
    QDomElement name = m_DomDocument->createElement("name");
    name.appendChild(m_DomDocument->createTextNode("Compass path"));
    m_Document.appendChild(name);


    // Create "Style" redLine
    QDomElement style = m_DomDocument->createElement("Style");
    style.setAttribute("id", "redLine");

    {
        // Create "Style.LineStyle"
        QDomElement lineStyle = m_DomDocument->createElement("LineStyle");
        QDomElement color = m_DomDocument->createElement("color");
        color.appendChild(m_DomDocument->createTextNode("aa0000ff"));
        lineStyle.appendChild(color);

        QDomElement width = m_DomDocument->createElement("width");
        width.appendChild(m_DomDocument->createTextNode("2"));
        lineStyle.appendChild(width);

        style.appendChild(lineStyle);
    }

    {
        // Create "Style.PolyStyle"
        QDomElement polyStyle = m_DomDocument->createElement("PolyStyle");
        QDomElement color = m_DomDocument->createElement("color");
        color.appendChild(m_DomDocument->createTextNode("aa0000bb"));
        polyStyle.appendChild(color);

        style.appendChild(polyStyle);
    }

    m_Document.appendChild(style);
}


/*!
  Loads the KML from file, sets the m_Document to refer to Document element of
  the KML. Returns false if the loading failed or the file did not exist.
*/
bool PersistentStorage::loadDocument()
{
    if (QFile::exists(m_KmlFilePath) == false) {
        qDebug() << "KML file did not exist";
        return false;
    }

    QFile file(m_KmlFilePath);
    if (file.open(QFile::ReadOnly) == false) {
        qDebug() << "Could not open KML file.";
        return false;
    }

    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (m_DomDocument->setContent(file.readAll(),
                                  &errorMsg,
                                  &errorLine,
                                  &errorColumn) == false) {
        qDebug() << "Failed to parse the KML, error: "
                 << errorMsg
                 << " on line "
                 << errorLine
                 << " on column "
                 << errorColumn;
        return false;
    }

    QDomNodeList nodeList = m_DomDocument->elementsByTagName("Document");
    if (nodeList.size() == 0) {
        qDebug() << "Could not find Document element";
        return false;
    }
    m_Document = nodeList.item(0).toElement();
    if (m_Document.isNull()) {
        qDebug() << "The m_Document was not Element node";
        return false;
    }

    return true;
}


/*!
  Saves the m_DomDocument to the m_KmlFilePath. If the file already exists
  it will be overwritten. Returns true if file was succesfully saved,
  otherwise returns false.
*/
bool PersistentStorage::saveDocument()
{
    if (QFile::exists(m_KmlFilePath)) {
        QFile::remove(m_KmlFilePath);
    }

    bool ret;

    // Write KML to file.
    QFile file(m_KmlFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false) {
        qDebug() << "Failed to open file " << m_KmlFilePath << " to write!";
        ret = false;
    }
    else {
        qint64 bytesWritten = file.write(m_DomDocument->toByteArray(2));
        qDebug() << "Wrote " << bytesWritten << " bytes to KML file "
                 << m_KmlFilePath;
        ret = true;
    }
    file.close();

    return ret;
}


/*!
  Creates new PlaceMark element for route in the DOM Document. The
  m_Coordinates element will updated to reference to the just created
  PlaceMark element.
  The m_Document must be valid before calling this method.
*/
void PersistentStorage::createRoutePlaceMark()
{
    // Create "PlaceMark"
    QDomElement placemark = m_DomDocument->createElement("Placemark");
    QDomElement styleUrl = m_DomDocument->createElement("styleUrl");
    styleUrl.appendChild(m_DomDocument->createTextNode("#redLine"));
    placemark.appendChild(styleUrl);

    // Create "PlaceMark.LineString"
    QDomElement lineString = m_DomDocument->createElement("LineString");

    // Create "PlaceMark.LineString.extrude"
    QDomElement extrude = m_DomDocument->createElement("extrude");
    extrude.appendChild(m_DomDocument->createTextNode("1"));
    lineString.appendChild(extrude);

    // Create "PlaceMark.LineString.tesselate"
    QDomElement tesselate = m_DomDocument->createElement("tesselate");
    tesselate.appendChild(m_DomDocument->createTextNode("1"));
    lineString.appendChild(tesselate);

    // Create "PlaceMark.LineString.altitudeMode"
    QDomElement altitudeMode = m_DomDocument->createElement("altitudeMode");
#ifdef Q_WS_HARMATTAN
    altitudeMode.appendChild(m_DomDocument->createTextNode("clampToGround"));
#else
    altitudeMode.appendChild(m_DomDocument->createTextNode("absolute"));
#endif
    lineString.appendChild(altitudeMode);


    // Create "PlaceMark.LineString.coordinates"
    m_Coordinates = m_DomDocument->createElement("coordinates");
    lineString.appendChild(m_Coordinates);

    placemark.appendChild(lineString);

    m_Document.appendChild(placemark);
}


/*!
  Adds route point to the QDomDocument.
*/
void PersistentStorage::addRouteCoordinate(const QVariant &varLongitude,
                                           const QVariant &varLatitude,
                                           const QVariant &varAltitude,
                                           const QVariant &varNewRoute)
{
    QString longitude = varLongitude.toString();
    QString latitude = varLatitude.toString();
    QString altitude = varAltitude.toString();
    bool newRoute = varNewRoute.toBool();

    // Make sure that the decimal separator is '.'!
    longitude.replace(',', '.');
    latitude.replace(',', '.');
    altitude.replace(',', '.');

    if (newRoute) {
        createRoutePlaceMark();
    }

    m_Coordinates.appendChild(
                m_DomDocument->createTextNode(QString("%1,%2,%3\n")
                                              .arg(longitude)
                                              .arg(latitude)
                                              .arg(altitude)));
    saveDocument();
}


/*!
  Adds waypoint to the KML file. This feature is implemented to get
  start / end positions of the route in external KML-viewer application.
*/
void PersistentStorage::createWaypoint(const QVariant &varName,
                                       const QVariant &varTimestamp,
                                       const QVariant &varLongitude,
                                       const QVariant &varLatitude,
                                       const QVariant &varAltitude)
{
    QString longitude = varLongitude.toString();
    QString latitude = varLatitude.toString();
    QString altitude = varAltitude.toString();

    // Make sure that the decimal separator is '.'!
    longitude.replace(',', '.');
    latitude.replace(',', '.');
    altitude.replace(',', '.');


    QDomElement placeMark = m_DomDocument->createElement("Placemark");
    QDomElement name = m_DomDocument->createElement("name");
    name.appendChild(m_DomDocument->createTextNode(varName.toString()));
    placeMark.appendChild(name);

    QDomElement description = m_DomDocument->createElement("description");
    description.appendChild(m_DomDocument->createTextNode(varTimestamp.toString()));
    placeMark.appendChild(description);

    QDomElement point = m_DomDocument->createElement("Point");
    QDomElement extrude = m_DomDocument->createElement("extrude");
    extrude.appendChild(m_DomDocument->createTextNode("1"));
    point.appendChild(extrude);

    QDomElement altitudeMode = m_DomDocument->createElement("altitudeMode");
#ifdef Q_WS_HARMATTAN
    altitudeMode.appendChild(m_DomDocument->createTextNode("clampToGround"));
#else
    altitudeMode.appendChild(m_DomDocument->createTextNode("absolute"));
#endif
    point.appendChild(altitudeMode);

    QDomElement coordinates = m_DomDocument->createElement("coordinates");

    coordinates.appendChild(m_DomDocument->createTextNode(
                                QString("%1,%2,%3").arg(longitude)
                                                   .arg(latitude)
                                                   .arg(altitude)));

    point.appendChild(coordinates);

    placeMark.appendChild(point);

    m_Document.appendChild(placeMark);

    saveDocument();
}


/*!
  Clears the saved route.
*/
void PersistentStorage::clearRoute()
{
    if (QFile::exists(m_KmlFilePath)) {
        QFile::remove(m_KmlFilePath);
    }

    // Create KML document with empty coordinates.
    createDocument();
}


/*!
  Loads the route from KML file, if the loading of existing route fails or
  it does not exist, create the new KML file with empty route.

  The parameter must be QML PannableMap element, which must have function
  "addRoutePointHelper" with three parameters:
        longitude (real), latitude (real) and (bool)
*/
void PersistentStorage::loadRoute(const QVariant &varPannableMap)
{
    QObject *pannableMap = varPannableMap.value<QObject*>();

    if (loadDocument() == false) {
        createDocument();
        return;
    }

    QDomNodeList nodeList = m_DomDocument->elementsByTagName("LineString");

    for (int i=0; i<nodeList.count(); i++) {
        QDomElement lineString = nodeList.item(i).toElement();
        QDomNodeList coordinatesList = lineString.elementsByTagName("coordinates");

        if (coordinatesList.count() > 0) {
            m_Coordinates = coordinatesList.item(0).toElement();

            if (m_Coordinates.childNodes().count() == 0)
                // Empty route on coordinates
                continue;
        }

        QDomNode node = m_Coordinates.childNodes().item(0);
        if(node.isText() == false) {
            qDebug() << "coordinate element in KML does not contain TextNode!";
            continue;
        }

        QStringList coordinates = node.nodeValue().split('\n',
                                                         QString::SkipEmptyParts);

        bool firstPointInRoute = true;

        foreach (const QString &coordinateString, coordinates) {
            QStringList components = coordinateString.split(',');
            if (components.size() < 2) {
                continue;
            }

            // Only read longitude and latitude, we don't know how
            // to show altitude yet in compass.
            float longitude = components[0].toFloat();
            float latitude = components[1].toFloat();

            QMetaObject::invokeMethod(pannableMap,
                                      "addRoutePointHelper",
                                      Q_ARG(QVariant, longitude),
                                      Q_ARG(QVariant, latitude),
                                      Q_ARG(QVariant, firstPointInRoute));
            firstPointInRoute = false;
        }
    }
}
