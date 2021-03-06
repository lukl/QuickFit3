/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License (LGPL) as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License (LGPL) for more details.

    You should have received a copy of the GNU Lesser General Public License (LGPL)
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qfmanyfilessettings.h"
#include <QStringList>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

QFManyFilesSettings::QFManyFilesSettings(QObject *parent):
    QObject(parent)
{
}

QFManyFilesSettings::~QFManyFilesSettings()
{
    clear();
}

void QFManyFilesSettings::addSettings(QSettings *settings, bool takeOwnership, bool readony)
{
    this->settings.append(settings);
    if (takeOwnership) settings->setParent(this);
    if (readony) this->readonly.insert(settings);
}

void QFManyFilesSettings::clear(bool deleteSettings)
{
    if (deleteSettings) {
        for (int i=0; i<settings.size(); i++) {
            if (settings[i]) delete settings[i];
        }
    }
    settings.clear();
    readonly.clear();
}

QSettings *QFManyFilesSettings::getSettings(int level)
{
    return settings.value(level, NULL);
}

QVariant QFManyFilesSettings::value(const QString &key, const QVariant &defaultValue, int* level) const
{
    for (int i=settings.size()-1; i>=0; i--) {
        //qDebug()<<"    "<<i<<" checking "<<settings[i]<<((settings[i])?settings[i]->fileName():QString("---"))<<"  for "<<key;
        if (settings[i]) {
            if (settings[i]->contains(key)) {
                //qDebug()<<"    "<<i<<" found!";
                if (level) *level=i;
                return settings[i]->value(key, defaultValue);
            }
        }
    }
    //qDebug()<<"     not found! ... returning default!";
    return defaultValue;
}

bool QFManyFilesSettings::setValue(const QString &key, const QVariant &value, int *level)
{
    int foundOnLevel=settings.size()-1;
    for (int i=settings.size()-1; i>=0; i--) {
        if (settings[i]) {
            if (settings[i]->contains(key)) {
                foundOnLevel=i;
                break;
            }
        }
    }
    if (level) *level=foundOnLevel;
    for (int i=foundOnLevel; i>=0; i--) {
        if (settings[i]) {
            if (settings[i]->isWritable() && !readonly.contains(settings[i])) {
                if (settings[i]->value(key)!=value) settings[i]->setValue(key, value);
                return true;
            }
        }
    }
    return false;
}

QStringList QFManyFilesSettings::childGroups() const
{
    QStringList sl;
    for (int i=settings.size()-1; i>=0; i--) {
        if (settings[i]) {
            sl<<settings[i]->childGroups();
        }
    }
    sl.removeDuplicates();
    return sl;
}

QStringList QFManyFilesSettings::allKeys() const
{
    QStringList sl;
    for (int i=settings.size()-1; i>=0; i--) {
        if (settings[i]) {
            sl<<settings[i]->allKeys();
        }
    }
    sl.removeDuplicates();
    return sl;

}

void QFManyFilesSettings::beginGroup(const QString &prefix)
{
    for (int i=settings.size()-1; i>=0; i--) {
        if (settings[i]) {
            settings[i]->beginGroup(prefix);
        }
    }
}

void QFManyFilesSettings::endGroup()
{
    for (int i=settings.size()-1; i>=0; i--) {
        if (settings[i]) {
            settings[i]->endGroup();
        }
    }
}

int QFManyFilesSettings::levels() const
{
    return settings.size();
}

















void jksaveWidgetGeometry(QFManyFilesSettings& settings, QWidget* widget, QString prefix) {
    settings.setValue(prefix+"pos", widget->pos());
    settings.setValue(prefix+"size", widget->size());
}

void jkloadWidgetGeometry(QFManyFilesSettings& settings, QWidget* widget, QPoint defaultPosition, QSize defaultSize, QString prefix) {
    QPoint pos = settings.value(prefix+"pos", defaultPosition).toPoint();
    QSize size = settings.value(prefix+"size", defaultSize).toSize();

    widget->resize(size.boundedTo(QApplication::desktop()->screenGeometry(widget).size()));
    if (pos.x()<0 || pos.x()>QApplication::desktop()->screenGeometry(widget).width()) pos.setX(0);
    if (pos.y()<0 || pos.y()>QApplication::desktop()->screenGeometry(widget).height()) pos.setY(0);
    widget->move(pos);
}

void jkloadWidgetGeometry(QFManyFilesSettings& settings, QWidget* widget, QString prefix) {
    jkloadWidgetGeometry(settings, widget, QPoint(10, 10), QSize(100, 100), prefix);
}

void jksaveSplitter(QFManyFilesSettings& settings, QSplitter* splitter, QString prefix) {
    /*QList<int> sizes=splitter->sizes();
    QString data="";
    for (int i=0; i<sizes.size(); i++) {
        if (!data.isEmpty()) data=data+",";
        data=data+QString::number(sizes[i]);
    }
    settings.setValue(prefix+"splitter_sizes", data);*/
    settings.setValue(prefix+"splitter_sizes", splitter->saveState());
}

void jkloadSplitter(QFManyFilesSettings& settings, QSplitter* splitter, QString prefix) {
    /*QString data=settings.value(prefix+"splitter_sizes", "").toString();
    QList<int> sizes, s1;
    QStringList sl=data.split(",");
    for (int i=0; i<sl.size(); i++) {
        sizes.append(sl[i].toInt());
    }
    s1=splitter->sizes();
    for (int i=0; i<s1.count(); i++) {
        if (i<sizes.size()) s1[i]=sizes[i];
    }*/
    splitter->restoreState(settings.value(prefix+"splitter_sizes").toByteArray());
}
