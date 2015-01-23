/*
    Copyright (c) 2008-2014 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>), German Cancer Research Center (DKFZ) & IWR, University of Heidelberg

    last modification: $LastChangedDate$  (revision $Rev$)

    This file is part of QuickFit 3 (http://www.dkfz.de/Macromol/quickfit).

    This software is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qfrawdataeditor.h"
#include "qfrawdatarecord.h"
#include "qfrawdatapropertyeditor.h"
#include <QCloseEvent>

QFRawDataEditor::QFRawDataEditor(QFPluginServices* services,  QFRawDataPropertyEditor* propEditor, QWidget* parent):
    QWidget(parent)
{
    current=NULL;
    peID=0;
    this->services=services;
    this->propertyEditor=propEditor;

    actSaveReport=new QAction(QIcon(":/lib/savereport.png"), tr("Save &Report"), this);
    actSaveReport->setToolTip(tr("save the evaluation results as a report in PDF or PostScript format"));
    connect(actSaveReport, SIGNAL(triggered()), this, SLOT(saveReport()));
    actPrintReport=new QAction(QIcon(":/lib/printreport.png"), tr("&Print Report"), this);
    actPrintReport->setToolTip(tr("print the evaluation results as a report"));
    connect(actPrintReport, SIGNAL(triggered()), this, SLOT(printReport()));

    actPrintReport->setToolTip(tr("print a report which contains all data on the current screen"));

    actSaveReport->setToolTip(tr("save a report which contains all data on the current screen as PDF or PostScript file"));
}

QFRawDataEditor::~QFRawDataEditor()
{
}

void QFRawDataEditor::setCurrent(QFRawDataRecord* c, int peID) {
    this->peID=peID;
    QFRawDataRecord* old=current;
    if (current) {
        writeSettings();
        disconnect(current, SIGNAL(rawDataChanged()), this, SLOT(rawDataChanged()));
    }
    current=c;
    //std::cout<<"connecting widgets ... \n";
    connectWidgets(current, old);
    //std::cout<<"connecting widgets ... done\n";
    if (current) {
        connect(current, SIGNAL(rawDataChanged()), this, SLOT(rawDataChanged()));
        rawDataChanged();
    }
}

void QFRawDataEditor::closeEvent( QCloseEvent * event ) {
    if (settings) writeSettings();
    event->accept();
}

void QFRawDataEditor::setSettings(ProgramOptions* s, int peID) {
    std::cout<<"QFRawDataEditor::setSettings("<<s<<", "<<peID<<")\n";
    settings=s;
    this->peID=peID;
    if (settings) readSettings();
}

void QFRawDataEditor::sendEditorCommand(const QString &command, const QVariant &param1, const QVariant &param2, const QVariant &param3, const QVariant &param4, const QVariant &param5)
{
}

void QFRawDataEditor::saveReport()
{

    /* it is often a good idea to have a possibility to save or print a report about the fit results.
       This is implemented in a generic way here.    */

    QString currentSaveDirectory="";
    if (settings && current) currentSaveDirectory=settings->getQSettings()->value(QString("%2/evaluationeditor%1/last_report_directory").arg(peID).arg(current->getType()), currentSaveDirectory).toString();

    QString fn = QFileDialog::getSaveFileName(this, tr("Save Report"),
                                currentSaveDirectory,
                            #if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
                                tr("PDF File (*.pdf);;PostScript File (*.ps)")
                            #else
                                tr("PDF File (*.pdf)")
                            #endif
                            );


    if (!fn.isEmpty()) {
        settings->getQSettings()->setValue(QString("%2/evaluationeditor%1/last_report_directory").arg(peID).arg(current->getType()), currentSaveDirectory);
        currentSaveDirectory=QFileInfo(fn).absolutePath();
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QProgressDialog progress(tr("Exporting ..."), "", 0, 100, NULL);
        progress.setWindowModality(Qt::WindowModal);
        //progress.setHasCancel(false);
        progress.setLabelText(tr("saving report <br> to '%1' ...").arg(fn));
        progress.setValue(50);
        progress.show();

        QFileInfo fi(fn);
        QPrinter* printer=new QPrinter();//QPrinter::HighResolution);
        printer->setPaperSize(QPrinter::A4);
        printer->setPageMargins(15,15,15,15,QPrinter::Millimeter);
        printer->setOrientation(QPrinter::Portrait);
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setColorMode(QPrinter::Color);
        QTextDocument* doc=new QTextDocument();
        doc->setTextWidth(printer->pageRect().size().width());
        createReportDoc(doc);
        if (fi.suffix().toLower()=="ps" || fi.suffix().toLower()=="pdf") {
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
            if (fi.suffix().toLower()=="ps") printer->setOutputFormat(QPrinter::PostScriptFormat);
#endif
            printer->setColorMode(QPrinter::Color);
            printer->setOutputFileName(fn);
            doc->print(printer);
        } else if (fi.suffix().toLower()=="odf") {
            QTextDocumentWriter writer(fn, "odf");
            writer.write(doc);
        } else if ((fi.suffix().toLower()=="html")||(fi.suffix().toLower()=="htm")) {
            QTextDocumentWriter writer(fn, "html");
            writer.write(doc);
        }
        //qDebug()<<doc->toHtml();
        delete doc;
        delete printer;
        progress.accept();
        QApplication::restoreOverrideCursor();
    }
}

void QFRawDataEditor::printReport()
{
    /* it is often a good idea to have a possibility to save or print a report about the fit results.
       This is implemented in a generic way here.    */
    QPrinter* p=new QPrinter();

    p->setPageMargins(15,15,15,15,QPrinter::Millimeter);
    p->setOrientation(QPrinter::Portrait);
    QPrintDialog *dialog = new QPrintDialog(p, this);
    dialog->setWindowTitle(tr("Print Report"));
    if (dialog->exec() != QDialog::Accepted) {
        delete p;
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QTextDocument* doc=new QTextDocument();
    doc->setTextWidth(p->pageRect().size().width());
    createReportDoc(doc);
    doc->print(p);
    delete p;
    delete doc;
    QApplication::restoreOverrideCursor();
}

void QFRawDataEditor::createReportDoc(QTextDocument *doc) {
    doc->setHtml(tr("<center>THIS PLUGIN DOES NOT CREATE REPORTS!</center>"));
}



