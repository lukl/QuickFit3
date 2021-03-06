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

#include "csvtools.h"
#include <iostream>
#include <math.h>
#include <QDebug>
#include <QChar>
#include "qftools.h"
#include <QApplication>
#include <QMimeData>
#include <QClipboard>
#include "datatools.h"
#include "qfdlgcsvparameters.h"
#include "programoptions.h"
#include <QVariant>

QList<QVector<double> > importCSVAskUser(const QString& filename, const QString& configprefix, QStringList* colNames) {
    QFDlgCSVParameters* csvDlg=new QFDlgCSVParameters(NULL, ProgramOptions::getConfigValue(configprefix+"column_separator_save", ",").toString(),
                                                  ProgramOptions::getConfigValue(configprefix+"decimal_separator_save", ".").toString(),
                                                  ProgramOptions::getConfigValue(configprefix+"comment_start_save", "#").toString(),
                                                  ProgramOptions::getConfigValue(configprefix+"header_start_save", "#!").toString());

    csvDlg->setFileContents(filename);
    QList<QVector<double> > res;
    if (csvDlg->exec()==QDialog::Accepted) {
        ProgramOptions::setConfigValue(configprefix+"column_separator_save", QString(csvDlg->get_column_separator()));
        ProgramOptions::setConfigValue(configprefix+"decimal_separator_save", QString(csvDlg->get_decimal_separator()));
        ProgramOptions::setConfigValue(configprefix+"comment_start_save", QString(csvDlg->get_comment_start()));
        ProgramOptions::setConfigValue(configprefix+"header_start_save", QString(csvDlg->get_header_start()));

        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            res=readCSV(in, csvDlg->get_column_separator(), csvDlg->get_decimal_separator(), csvDlg->get_comment_start(), csvDlg->get_header_start(), NAN, QString("\n"), QString("\r"), colNames);
            file.close();
        }
    }
    delete csvDlg;
    return res;
}

QList<QVector<double> > readCSV(QTextStream& f, QChar separator_char, QChar /*decimalsep_char*/, QChar comment_char, const QString& /*headerComment*/, double non_value, const QString& eolChars, const QString& ignoreChars, QStringList* colNames) {
    QList<QVector<double> > res;
    while (!f.atEnd())  {
        QVector<double> d=csvReadline(f,separator_char, comment_char, non_value, eolChars, ignoreChars);
        if (d.size()>0) res.append(d);
    }
    int rows=0;
    for (int i=0; i<res.size(); i++) {
        rows=qMax(rows, res[i].size());
    }
    for (int i=0; i<res.size(); i++) {
        while (res[i].size()<rows) res[i].append(non_value);
    }
    if (colNames) {
        for (int i=0; i<rows; i++) {
            (*colNames)<<QString("col %1").arg(i+1);
        }
    }
    return dataRotate(res);
}

QVector<double> csvReadline(QString f, QChar separator_char, QChar comment_char, double non_value, const QString& eolChars, const QString& ignoreChars, QString* lineOut) {
    QTextStream ff(&f);
    return csvReadline(ff, separator_char, comment_char,  non_value,  eolChars,  ignoreChars, lineOut);
}
QVector<double> csvReadline(QTextStream& f, QChar separator_char, QChar comment_char, double non_value, const QString& eolChars, const QString& ignoreChars, QString* lineOut) {
    //const QString line=f.readLine();
    QString line="";
    QString ch=f.read(1);
    while (!f.atEnd() && !eolChars.contains(ch)) {
        line.append(ch);
        ch=f.read(1);
    }
    if (!eolChars.contains(ch)) line.append(ch);
    QLocale loc=QLocale::c();
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    //qDebug()<<"line='"<<line;
    QVector<double> result;
    bool done=false;
    QString num="";
    bool isComment=false;
    bool ok=false;
    double d=0;
    if (lineOut) *lineOut=line;
    //for (int i=0; i<line.size(); i++) {
    for (QString::const_iterator i=line.begin(); i!=line.end(); ++i) {
        const QChar ch=*i;//line[i];
        const char cc=ch.toLatin1();
        if (!isComment) {
            switch(cc) {

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case 'e':
                case '-':
                case '+':
                case 'n':
                case 'a':
                case 'i':
                case 'f':
                    num=num+ch;
                    break;
                /*case '\r': break;
                case '\n':
                    done=true;
                    if (num.size()>0) {
                        result.append(loc.toDouble(num, &ok));
                    }
                    num="";
                    break;*/
                case 'N':
                    num=num+'n';
                    break;
                case 'A':
                    num=num+'a';
                    break;
                case 'I':
                    num=num+'i';
                    break;
                case 'F':
                    num=num+'f';
                    break;
                case 'E':
                    num=num+'e';
                    break;
                default:
                    if (ignoreChars.contains(ch)) {
                         // ignore this character ;-)
                    } else if (eolChars.contains(ch)) {
                        // found an end of line character
                        done=true;
                        if (num.size()>0) {
                            result.append(loc.toDouble(num, &ok));
                        }
                        num="";

                    } else if (ch=='.' && separator_char!='.') {
                        num=num+'.';
                    } else if (ch==',' && separator_char!=',') {
                        num=num+'.';
                    } else if (ch==separator_char) {
                        if (num.size()>0) {
                            ok=true;
                            if (num=="nan") {
                                result.append(NAN);
                            } else if (num=="inf") {
                                result.append(INFINITY);
                            } else{
                                d=loc.toDouble(num, &ok);
                                if (ok) result.append(d);
                                else result.append(non_value);
                            }
                        }
                        num="";
                    } else if (ch==comment_char) {
                        isComment=true;
                        if (num.size()>0) {
                            ok=true;
                            if (num=="nan") {
                                result.append(NAN);
                            } else{
                                d=num.toDouble(&ok);
                                if (ok) result.append(d);
                                else result.append(non_value);
                            }
                        }
                        num="";
                        done=true;
                    }
                    break;
            }
        } else {
            if (eolChars.contains(ch)) {
                done=true;
            }
            break;
        }

        if (done) break;
    }
    if ((!done) && num.size()>0) {
        result.append(loc.toDouble(num, &ok));
    }
    if (result.size()<=0 && isComment) return csvReadline(f, separator_char, comment_char, non_value);
    return result;
}




void csvCopy(const QList<QList<double> >& data, const QStringList& columnsNames, const QStringList& rowNames, bool copyWithLocalePriority) {

    QString csv, csvLocale, excel;
    QLocale loc;
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    int datas=0;
    for (int r=0; r<data.size(); r++) {
        if (data[r].size()>datas) datas=data[r].size();
    }

    if (columnsNames.size()>0) {
        csv+="#! ";
        if (rowNames.size()>0) {
            csv+=csv+QObject::tr("\"row name\", ");
            csvLocale+=csvLocale+"; ";
            excel+=excel+"\t";
        }
        for (int i=0; i<columnsNames.size(); i++) {
            if (i>0) {
                csv+=", ";
                csvLocale+="; ";
                excel+="\t";
            }
            if (!columnsNames.value(i, "").isEmpty()) {

                QString strans=columnsNames.value(i, "");
                strans=strans.replace("\"", "").replace("'", "").replace("\n", " ").replace("\r", " ");
                strans=QString("\"%1\"").arg(strans);
                csv+=strans;
                csvLocale+=strans;
                excel+=strans;
            }
        }
        csv+="\n";
        csvLocale+="\n";
        excel+="\n";
    }
    for (int r=0; r<datas; r++) {
        if (rowNames.size()>0) {
            QString strans=rowNames.value(r, "");
            strans=strans.replace("\"", "\\\"").replace("'", "\\\'").replace("\n", " ").replace("\r", " ").replace("\t", " ");
            strans=QString("\"%1\"").arg(strans);
            QString strans_nc=strans;
            strans_nc=strans_nc.replace(',', ';');
            QString strans_ns=strans;
            strans_ns=strans_ns.replace(';', ',');
            csv+=strans_nc+", ";
            csvLocale+=strans_ns+"; ";
            excel+=strans+"\t";
        }
        for (int c=0; c<data.size(); c++) {
            if (c>0) {
                csv+=", ";
                csvLocale+="; ";
                excel+="\t";
            }
            if (r<data[c].size()) {
                double d=data[c].at(r);

                excel+=loc.toString(d);
                csvLocale+=loc.toString(d);
                csv+=CDoubleToQString(d);
            }
        }
        csv+="\n";
        csvLocale+="\n";
        excel+="\n";
    }



    QClipboard *clipboard = QApplication::clipboard();
    QMimeData* mime=new QMimeData();
    if (copyWithLocalePriority) {
        mime->setText(excel);
    } else {
        mime->setText(csv);
    }
    mime->setData("jkqtplotter/csv", csv.toUtf8());
    mime->setData("application/vnd.ms-excel", excel.toLocal8Bit());
    mime->setData("quickfit/csv", csv.toUtf8());
    mime->setData("text/csv", csv.toLocal8Bit());
    mime->setData("text/comma-separated-values", csv.toLocal8Bit());
    mime->setData("quickfit3/qfrdrtable", toQFTableModelXML(dataToVariant(data), columnsNames, rowNames).toUtf8());
    clipboard->setMimeData(mime);
}

void csvCopy(const QList<QList<QVariant> >& data, const QStringList& columnsNames, const QStringList& rowNames, bool copyWithLocalePriority) {

    QString csv, csvLocale, excel;
    QString stringDelimiter="\"";
    QLocale loc;
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    int datas=0;
    for (int r=0; r<data.size(); r++) {
        if (data[r].size()>datas) datas=data[r].size();
    }

    if (columnsNames.size()>0) {
        csv+="#! ";
        if (rowNames.size()>0) {
            csv+=csv+QObject::tr("\"row name\", ");
            csvLocale+=csvLocale+"; ";
            excel+=excel+"\t";
        }
        for (int i=0; i<columnsNames.size(); i++) {
            if (i>0) {
                csv+=", ";
                csvLocale+="; ";
                excel+="\t";
            }
            if (!columnsNames.value(i, "").isEmpty()) {

                QString strans=columnsNames.value(i, "");
                strans=strans.replace("\"", "").replace("'", "").replace("\n", " ").replace("\r", " ");
                strans=QString("\"%1\"").arg(strans);
                csv+=strans;
                csvLocale+=strans;
                excel+=strans;
            }
        }
        csv+="\n";
        csvLocale+="\n";
        excel+="\n";
    }

    //qDebug()<<datas<<data.size();
    for (int r=0; r<datas; r++) {
        if (rowNames.size()>0) {
            QString strans=rowNames.value(r, "");
            strans=strans.replace("\"", "\\\"").replace("'", "\\\'").replace("\n", " ").replace("\r", " ").replace("\t", " ");
            strans=QString("\"%1\"").arg(strans);
            QString strans_nc=strans;
            strans_nc=strans_nc.replace(',', ';');
            QString strans_ns=strans;
            strans_ns=strans_ns.replace(';', ',');
            csv+=strans_nc+", ";
            csvLocale+=strans_ns+"; ";
            excel+=strans+"\t";
        }
        for (int c=0; c<data.size(); c++) {
            if (c>0) {
                csv+=", ";
                csvLocale+="; ";
                excel+="\t";
            }
            if (r<data[c].size()) {
                const QVariant& v=data[c].at(r);

                //qDebug()<<r<<c<<v;

                switch(v.type()) {
                    case QVariant::Int:
                    case QVariant::UInt:
                    case QVariant::LongLong:
                    case QVariant::ULongLong:
                    case QVariant::Double:  {
                            double d=v.toDouble();
                            excel+=loc.toString(d);
                            csvLocale+=loc.toString(d);
                            csv+=CDoubleToQString(d);
                        } break;
                    case QVariant::Invalid:  break;
                    default:
                        if (v.isValid()) {
                            QString h=v.toString();
                            h=h.replace(',', "_").replace(';', "_").replace('\t', " ").replace('\n', "\\n").replace('\r', "\\r").replace('\'', "_").replace('\"', "_");
                            excel+=stringDelimiter+h+stringDelimiter;
                            csvLocale+=stringDelimiter+h+stringDelimiter;
                            csv+=stringDelimiter+h+stringDelimiter;
                        }
                        break;

                }

            }
        }
        csv+="\n";
        csvLocale+="\n";
        excel+="\n";
    }



    QClipboard *clipboard = QApplication::clipboard();
    QMimeData* mime=new QMimeData();
    if (copyWithLocalePriority) {
        mime->setText(excel);
    } else {
        mime->setText(csv);
    }
    mime->setData("jkqtplotter/csv", csv.toUtf8());
    mime->setData("application/vnd.ms-excel", excel.toLocal8Bit());
    mime->setData("quickfit/csv", csv.toUtf8());
    mime->setData("text/csv", csv.toLocal8Bit());
    mime->setData("text/comma-separated-values", csv.toLocal8Bit());
    mime->setData("quickfit3/qfrdrtable", toQFTableModelXML(data, columnsNames, rowNames).toUtf8());
    clipboard->setMimeData(mime);
}

void csvCopy(const QList<QVector<double> >& data, const QStringList& columnsNames, const QStringList& rowNames, bool withLocalePriority) {
    QList<QList<double> > d;
    for (int i=0; i<data.size(); i++) {
        d.append(data[i].toList());
    }
    csvCopy(d, columnsNames, rowNames, withLocalePriority);
}

QList<QVector<double> > csvDataRotate(const QList<QVector<double> >& data) {
    QList<QVector<double> > result;
    int cols=0;
    for (int i=0; i<data.size(); i++) {
        if (cols<data[i].size()) cols=data[i].size();
    }
    QVector<double> dEmpty(data.size(), NAN);
    for (int i=0; i<cols; i++) {
        result<<dEmpty;
    }
    for (int i=0; i<data.size(); i++) {
        for (int r=0; r<data[i].size(); r++) {
            result[r].operator[](i)=data[i].at(r);
        }
    }
    return result;
}

QList<QList<double> > csvDataRotate(const QList<QList<double> >& data) {
    QList<QList<double> > result;
    int cols=0;
    for (int i=0; i<data.size(); i++) {
        if (cols<data[i].size()) cols=data[i].size();
    }
    QList<double> dEmpty;
    for (int i=0; i<data.size(); i++) dEmpty<<NAN;
    for (int i=0; i<cols; i++) {
        result<<dEmpty;
    }
    for (int i=0; i<data.size(); i++) {
        for (int r=0; r<data[i].size(); r++) {
            result[r].operator[](i)=data[i].at(r);
        }
    }
    return result;
}

QList<QList<QVariant> > csvDataRotate(const QList<QList<QVariant> >& data) {
    QList<QList<QVariant> > result;
    int cols=0;
    for (int i=0; i<data.size(); i++) {
        if (cols<data[i].size()) cols=data[i].size();
    }
    QList<QVariant> dEmpty;
    for (int i=0; i<data.size(); i++) dEmpty<<QVariant();
    for (int i=0; i<cols; i++) {
        result<<dEmpty;
    }
    for (int i=0; i<data.size(); i++) {
        for (int r=0; r<data[i].size(); r++) {
            result[r].operator[](i)=data[i].at(r);
        }
    }
    return result;
}

QString toCSV(const QList<QVector<double> >& data, const QStringList& columnNames, const QStringList& rowNames, QChar decimalSep, const QString colSep, bool withHeaders, QChar stringDelimiter, const QString& headerSep, int precision, int fieldWidth) {
    QString res;
    int cols=data.size();
    int rows=0;
    for (int i=0; i<data.size(); i++) {
        rows=qMax(rows, data[i].size());
    }
    if (columnNames.size()>0 && withHeaders) {
        res+=headerSep;
        for (int i=0; i<cols; i++) {
            if (i>0 || rowNames.size()>0) res+=colSep;
            QString h=columnNames.value(i, "");
            h=h.replace(colSep, "_").replace(',', "_").replace(';', "_").replace('\t', " ").replace('\n', "\\n").replace('\r', "\\r").replace(stringDelimiter, "_");
            res+=qfLeftPaddedString(stringDelimiter+h+stringDelimiter, fieldWidth);
        }
        res+="\n";
    }

    for (int r=0; r<rows; r++) {
        if (rowNames.size()>0 && withHeaders) {
            QString h=rowNames.value(r, "");
            h=h.replace(colSep, "_").replace(',', "_").replace(';', "_").replace('\t', " ").replace('\n', "\\n").replace('\r', "\\r").replace(stringDelimiter, "_");
            res+=qfLeftPaddedString(stringDelimiter+h+stringDelimiter+colSep, fieldWidth);
        }
        for (int c=0; c<cols; c++) {
            if (c>0 || rowNames.size()>0) res+=colSep;
            if (r<data[c].size()) res+=qfLeftPaddedString(doubleToQString(data[c].value(r, 0), precision, 'g', decimalSep), fieldWidth);
        }
        res+="\n";
    }
    return res;
}


QString toCSV(const QList<QList<QVariant> >& data, const QStringList& columnNames, const QStringList& rowNames, QChar decimalSep, const QString colSep, bool withHeaders, QChar stringDelimiter, const QString& headerSep, int precision, int fieldWidth) {
    QString res;
    int cols=data.size();
    int rows=0;
    for (int i=0; i<data.size(); i++) {
        rows=qMax(rows, data[i].size());
    }
    if (columnNames.size()>0 && withHeaders) {
        res+=headerSep;
        for (int i=0; i<cols; i++) {
            if (i>0 || rowNames.size()>0) res+=colSep;
            QString h=columnNames.value(i, "");
            h=h.replace(colSep, "_").replace(',', "_").replace(';', "_").replace('\t', " ").replace('\n', "\\n").replace('\r', "\\r").replace(stringDelimiter, "_");
            res+=qfLeftPaddedString(stringDelimiter+h+stringDelimiter, fieldWidth);
        }
        res+="\n";
    }

    for (int r=0; r<rows; r++) {
        if (rowNames.size()>0 && withHeaders) {
            QString h=rowNames.value(r, "");
            h=h.replace(colSep, "_").replace(',', "_").replace(';', "_").replace('\t', " ").replace('\n', "\\n").replace('\r', "\\r").replace(stringDelimiter, "_");
            res+=qfLeftPaddedString(stringDelimiter+h+stringDelimiter+colSep, fieldWidth);
        }
        for (int c=0; c<cols; c++) {
            if (c>0 || rowNames.size()>0) res+=colSep;
            if (r<data[c].size()) {
                const QVariant& v=data[c].value(r, 0);
                switch(v.type()) {
                    case QVariant::Int:
                    case QVariant::UInt:
                    case QVariant::LongLong:
                    case QVariant::ULongLong:
                    case QVariant::Double:
                        res+=qfLeftPaddedString(doubleToQString(v.toDouble(), precision, 'g', decimalSep), fieldWidth);
                        break;
                    default:
                        if (v.isValid()) {
                            QString h=v.toString();
                            h=h.replace(colSep, "_").replace(',', "_").replace(';', "_").replace('\t', " ").replace('\n', "\\n").replace('\r', "\\r").replace(stringDelimiter, "_");
                            res+=qfLeftPaddedString(stringDelimiter+h+stringDelimiter, fieldWidth);
                        }
                        break;
                }
            }
        }
        res+="\n";
    }
    return res;
}









bool guessCSVParameters(QString data, char* out_sep, char* out_dec, char* out_comment, QString* out_headercomment) {
    QLocale loc;
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
    //QTextStream in(&data);
    QStringList sl=data.split("\n");
    char sep='\t';
    char dec=loc.decimalPoint().toLatin1();
    char comment='#';
    QString headercomment="";
    if (sl.size()>0) {
        QString d;
        int i=0;
        int cntl=0;
        QStringList lines;
        while (i<sl.size() && cntl<20) {
            QString line=sl[i].trimmed();
            if (line.size()>0) {
                lines.append(line);
                QChar c0=line.at(0);
                QChar c1='\0';
                if (line.size()>1) c1=line.at(1);
                if (c0.isDigit() || c0.isLetter() || QString::fromLatin1("!\xA7$&?\xB0#~*+-\\/<>\"\'").contains(c0)) {
                    d=d+sl[i]+"\n";
                    cntl++;
                }
                if (c0=='#') { comment='#'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='%') { comment='%'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='&') { comment='&'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0==';') { comment=';'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0==QChar::fromLatin1('\xA7')) { comment='\xA7'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='!') { comment='!'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='/') { comment='/'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='\\') { comment='\\'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='*') { comment='*'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='~') { comment='~'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='\'' && !line.contains('\'')) { comment='\''; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='\"' && !line.contains('\"')) { comment='\"'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='?') { comment='?'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='!') { comment='!'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0==QChar::fromLatin1('\xB0')) { comment='\xB0'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='<') { comment='\xB0'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='>') { comment='\xB0'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
                else if (c0=='=') { comment='\xB0'; if (QString::fromLatin1("!\xA7$&?\xB0#~*+\\/<>").contains(c1)) headercomment=QString(comment)+QString(c1); }
            }
            i++;
        }

        // remove all quotes from the string
        QString dd=d;
        d="";
        QChar quot='\0';
        for (int i=0; i<dd.size(); i++) {
            bool read=true;
            if (dd[i]=='"' || dd[i]=='\'') {
                if (quot!='\0' && quot==dd[i]) {
                    quot='\0';
                    read=false;
                } else if (quot!='\0' && quot!=dd[i]) {
                    read=false;
                } else if (quot=='\0') {
                    read=false;
                    quot=dd[i];
                }
            } else {
                read=(quot=='\0');
            }
            if (read) d.append(dd[i]);
        }
        //qDebug()<<data<<"\n\n"<<dd<<"\n\n"<<d;

        QRegExp rxNumbers("\\d([,.])\\d");
        if (rxNumbers.indexIn(d)>=0) {
            if (rxNumbers.cap(1).size()>0) dec=rxNumbers.cap(1).at(0).toLatin1();
        }

        double icntDot=d.count('.');
        double icntCom=d.count(',');
        d=d.remove(dec);
        int cnt=d.count('\n');
        int cntSem=d.count(';');
        int cntCom=d.count(',');
        int cntTab=d.count('\t');
        int cntSPACE=d.trimmed().count(' ');
        //qDebug()<<"icntDot="<<icntDot<<"  icntCom="<<icntCom<<"  cnt="<<cnt<<"  cntSem="<<cntSem<<"  cntCom="<<cntCom<<"  cntTab="<<cntTab;
        if (cntSem>=cnt && ';'!=dec) sep=';';
        if (cntCom>=cnt && ','!=dec) sep=',';
        if (cntTab>=cnt && cntSem<=0) sep='\t';
        if (cntTab>=cnt && cntSem<=0 && (cntCom<=0 || (cntCom>0 && icntDot>0))) sep='\t';
        if (dec==',' && sep==',') dec='.';
        if (sep!=',' && dec==',' && icntCom==0 && icntDot>0) dec='.';
        if (sep!=',' && dec=='.' && icntCom>0 && icntDot==0) dec=',';
        if (dec==',' && icntCom<=0 && icntDot>0 ) dec='.';
        if (icntDot>0 && icntCom>0) { dec='.'; sep=',';}
        if (sep==';' && comment==';') { comment='#'; headercomment=""; }
        if (headercomment.size()<=0) { headercomment=comment; headercomment+="!"; }
        if (sep=='\t' && cntTab<=0 && cntSPACE>0) sep=' ';
        //qDebug()<<"out_comment="<<QChar(comment);
        //qDebug()<<"out_dec="<<QChar(dec);
        //qDebug()<<"out_sep="<<QChar(sep);
        //qDebug()<<"out_headercomment="<<headercomment;
        //qDebug()<<"icntDot="<<icntDot;
        //qDebug()<<"icntCom="<<icntCom;
        //qDebug()<<"cnt="<<cnt;
        //qDebug()<<"cntSem="<<cntSem;
        //qDebug()<<"cntCom="<<cntCom;
        //qDebug()<<"cntTab="<<cntTab;
        if (out_comment) *out_comment=comment;
        if (out_dec) *out_dec=dec;
        if (out_sep) *out_sep=sep;
        if (out_headercomment) *out_headercomment=headercomment;
        return true;

    }
    return false;
}
