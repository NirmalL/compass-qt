/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef ORIENTATIONFILTER_H
#define ORIENTATIONFILTER_H

#include <QOrientationFilter>

QTM_USE_NAMESPACE

class OrientationFilter : public QObject, public QOrientationFilter
{
    Q_OBJECT
public:
    OrientationFilter(QObject *parent = NULL)
        : QObject(parent)
    {
    }

    bool filter(QOrientationReading *reading) {
        emit orientationChanged(reading->orientation());

        // don't store the reading in the sensor
        return false;
    }

signals:
    void orientationChanged(const QVariant &orientation);
};

#endif // ORIENTATIONFILTER_H
