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

#ifndef DLGNEWPROPERTY_H
#define DLGNEWPROPERTY_H

#include <QDialog>
#include "lib_imexport.h"


namespace Ui {
    class dlgNewProperty; // forward
}

class QFLIB_EXPORT dlgNewProperty : public QDialog
{
        Q_OBJECT
    public:
        /** Default constructor */
        dlgNewProperty(QWidget* parent);
        /** Default destructor */
        virtual ~dlgNewProperty();

        QString propertyName();
        QString propertyType();
        int propertyTypeInt();
        QString propertyValue();

        void setPropertyName(QString name);
        void setPropertyValue(QString value);
        void setPropertyType(int i);
        void addPropertyNames(const QStringList& names);
    protected:
    private:
        Ui::dlgNewProperty* ui;
};

#endif // DLGNEWPROPERTY_H
