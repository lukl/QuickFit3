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

#ifndef RENAMEGROUPSDIALOG_H
#define RENAMEGROUPSDIALOG_H

#include "qfdialog.h"
#include <QStringListModel>

namespace Ui {
    class RenameGroupsDialog;
}

class RenameGroupsDialog : public QFDialog
{
        Q_OBJECT
        
    public:
        explicit RenameGroupsDialog(QWidget *parent = 0);
        ~RenameGroupsDialog();

        void setStrings(const QStringList& list);
        QStringList getStrings() const;
    protected slots:
        void showHelp();
    private:
        Ui::RenameGroupsDialog *ui;

        QStringListModel model;
};

#endif // RENAMEGROUPSDIALOG_H
