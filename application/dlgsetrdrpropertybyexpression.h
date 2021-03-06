/*
Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    last modification: $LastChangedDate: 2014-09-02 10:24:20 +0200 (Di, 02 Sep 2014) $  (revision $Rev: 3422 $)

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

#ifndef DlgSetRDRPropertyByExpression_H
#define DlgSetRDRPropertyByExpression_H

#include "qfdialog.h"
#include "qfproject.h"
#include "qfrawdatarecord.h"
#include <QRegExp>
#include <QList>
#include <QPointer>
#include "dlgsetrdrpropertybyexpressioneditor.h"
#include "qfmathparser.h"

namespace Ui {
    class DlgSetRDRPropertyByExpression;
}


class DlgSetRDRPropertyByExpression : public QFDialog
{
        Q_OBJECT
        
    public:
        explicit DlgSetRDRPropertyByExpression(QWidget *parent = 0);
        ~DlgSetRDRPropertyByExpression();
        void setProject(QFProject* project);

        QList<QPointer<QFRawDataRecord> > getSelectedRDRs() const;

        QVariant getResult(const QString& expression, QFRawDataRecord* rdr, bool *apply=NULL, QString *error=NULL);
        void applyResult(DlgSetRDRPropertyByExpressionEditor *wid, QFRawDataRecord* rdr);
        QFRawDataRecord* currentRDR() const;
        int getEditorCount() const;
        void setEditorCount(int cnt);
        void addEditor();
        void removeEditor();
        QList<DlgSetRDRPropertyByExpressionEditor*> getEditors() const;
    protected slots:
        void on_btnSelectNone_clicked();
        void on_btnSelectAll_clicked();
        void on_btnToggled_clicked();

        void on_btnAdd_clicked();
        void showHelp();
        void removeEditor(DlgSetRDRPropertyByExpressionEditor* edt);
        void emitWidgetContentsChanged();
        void updateTest();
        void on_btnDeselSel_clicked();
        void on_btnSelSel_clicked();
    signals:
        void widgetContentsChanged();
    private:
        Ui::DlgSetRDRPropertyByExpression *ui;
        QFProject* project;
        QList<QPointer<QFRawDataRecord> > rdrs;

        QList<DlgSetRDRPropertyByExpressionEditor*> editors;

        QVBoxLayout* layItems;
        QFMathParser mp;
        QString lastExpression;
        QFMathParser::qfmpNode* lastNode;
};

#endif // DlgSetRDRPropertyByExpression_H
