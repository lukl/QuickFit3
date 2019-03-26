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

#ifndef QFRAWDATAPROPERTYEDITOR_PRIVATE_H
#define QFRAWDATAPROPERTYEDITOR_PRIVATE_H

#include<QtGlobal>
#include <QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include "qfrdrimagemask.h"
#include <QPointer>
#include <QMap>
#include <QSortFilterProxyModel>
#include "qfproject.h"
#include "qfrawdatarecord.h"
#include "programoptions.h"
#include "qfpluginservices.h"
#include "qftools.h"
#include "qenhancedtableview.h"
#include "lib_imexport.h"
#include <QMenu>
#include <QAction>
#include <QList>
#include <QPair>
#include <QTimer>
#include <QModelIndexList>
#include "qfenhancedlineedit.h"
#include "qfstyledbutton.h"
#include "qfcompleterfromfile.h"
#include "qfpropertyitemdelegate.h"
#include "dlg_qffileeditdialog.h"
#include "qfprojecttreemodel.h"
#include "qfelidedlabel.h"

class QFRawDataPropertyEditor; // forward

class QFRawDataPropertyEditor_private : public QObject
{
        Q_OBJECT
    public:
        explicit QFRawDataPropertyEditor_private(QFRawDataPropertyEditor *parent);
        

        QFRawDataPropertyEditor* d;

        /** \brief points to the record currently displayed */
        QPointer<QFRawDataRecord> current;

        QTimer* timerResizeProps;

        /** \brief create all widgets needed to display data */
        void createWidgets();
        /** \brief QLineEdit for the name */
        QLineEdit* edtName;
        /** \brief QLineEdit for the folder */
        QLineEdit* edtFolder;
        /** \brief combobox for roles */
        QComboBox* cmbRole;
        /** \brief QPlainTextEdit for the description */
        QPlainTextEdit* pteDescription;
        /** \brief list of files associated with this rawdatarecord */
        QListWidget* lstFiles;
        /** \brief label next to buttons on top */
        QLabel* labTop;
        /** \brief label next to buttons on top */
        QLabel* labTopIcon;
        /** \brief label for the ID */
        QLabel* labID;
        /** \brief label for the type */
        QLabel* labType;
        /** \brief label for the type icon */
        QLabel* labTypeIcon;
        /** \brief Tab to change between property editor and data view */
        QTabWidget* tabMain;
        /** \brief tabel view for the properties */
        QEnhancedTableView* tvProperties;
        /** \brief button to switch to next record */
        QToolButton* btnNext;
        /** \brief button to switch to next record */
        QToolButton* btnNextSameRole;
        /** \brief button to switch to previous record */
        QToolButton* btnPreviousSameRole;
        /** \brief button to insert a new property */
        QToolButton* btnNewProperty;
        QAction* actNewProperty;
        /** \brief button to delete a property */
        QToolButton* btnDeleteProperty;
        QAction* actDeleteProperty;
        /** \brief button to switch to previous record */
        QToolButton* btnPrevious;
        /** \brief button to delete the current record */
        QToolButton* btnDeleteRecord;
        /** \brief button to delete the current record */
        QToolButton* btnDeleteRecordGroup;
        /** \brief points to a settings object that is used to store application settings */
        ProgramOptions* settings;
        /** \brief tabel display the evaluation results associated with this file  */
        QEnhancedTableView* tvResults;
        /** \brief label for averaging over selected values in tvResults */
        QLabel* labAveragedresults;
        /** \brief toolbar to access functions of tvResults */
        QToolBar* tbResults;


        QToolButton* btnCopyProperty;
        QAction* actCopyProperty;
        QToolButton* btnCutProperty;
        QAction* actCutProperty;
        QToolButton* btnPasteProperty;
        QAction* actPasteProperty;

        QAction* actExportPluginData;

        /** \brief widget that is used to display the tvResults table + opt. some more compoinents */
        QWidget* widResults;
        /** \brief button to display help */
        QToolButton* btnHelp;
        QAction* actHelp;
        QAction* actHelpPlugin;
        QAction* actHelpPluginTutorial;
        QAction* actHelpPluginCopyright;
        QAction* actHelpRDR;
        QAction* actPrevious;
        QAction* actNext;
        QAction* actDelete;
        QAction* actDeleteGroup;
        QAction* actClose;
        QAction* actPreviousSameRole;
        QAction* actNextSameRole;

        QAction* actAddFile;
        QAction* actRemoveFile;
        QAction* actEditFile;
        QAction* actCopyFile;
        QAction* actEnableFileActions;
        QAction* actMoveFilesUp;
        QAction* actMoveFilesDown;
        QAction* actOpenFileDirectory;
        QAction* actOpenFileInSystem;
        QAction* actMakePropertyEditable;



        QMenuBar* menuBar;

        QMenu* menuRDR;
        QMenu* menuHelp;
        QMenu* menuResults;
        QList<QPair<int, QMenu*> > menus;

        /** \brief action used to copy selection in tvResults to clipbord */
        QAction* actCopyValErrResults;
        QAction* actCopyValErrResultsNoHead;
        /** \brief action used to copy selection in tvResults to clipbord */
        QAction* actCopyResults;
        QAction* actCopyResultsNoHead;
        /** \brief action used to save selection in tvResults to file */
        QAction* actSaveResults;
        QAction* actSaveResultsAveraged;
        /** \brief action used to delete selection in tvResults */
        QAction* actDeleteResults;
        QAction* actDeleteAllVisibleResults;
        QAction* actCopyMedianQuantilesResults;
        QAction* actCopyMedianQuantilesNoHead;
        QAction* actStatistics;
        QAction* actStatisticsComparing;
        QAction* actCorrelation;
        QAction* actShowData;
        /** \brief action used to copy selection in tvResults to clipbord, in expanded form */
        QAction* actCopyExpanded;
        QAction* actCopyExpandedNoHead;
        QAction* actCopyExpandedNoHeadMatlab;
        QAction* actCopyExpandedFlipped;
        QAction* actCopyExpandedNoHeadFlipped;
        QAction* actCopyExpandedNoHeadMatlabFlipped;
        //QAction* actCopyResultAccessParserFunctionTable;
        QMenu* menuCopyExpanded;

        QMenu* menuCopyIDs;
        QAction* actCopyRDRID;
        QAction* actCopyEvaluationID;
        QAction* actCopyResultID;
        QAction* actCopyResultAccessParserFunction;


        QFEnhancedLineEdit* edtFilterEvaluation;
        QFEnhancedLineEdit* edtFilterResults;
        QFEnhancedLineEdit* edtFilterEvaluationNot;
        QFEnhancedLineEdit* edtFilterResultsNot;
        QFCompleterFromFile* compFilterEvaluation;
        QFCompleterFromFile* compFilterResults;
        QFCompleterFromFile* compFilterEvaluationNot;
        QFCompleterFromFile* compFilterResultsNot;
        QCheckBox* chkFilterEvaluationRegExp;
        QCheckBox* chkFilterResultsRegExp;
        QCheckBox* chkShowAvg;
        QComboBox* cmbGroup;

        QPointer<QTreeView> treeNextRecord;
        QPointer<QFProjectTreeModel> projectTree;
        QPushButton* btnSelectFromTree;


        //QSortFilterProxyModel* paramFilterProxy;

        QList<QPointer<QFRawDataEditor> > editorList;
        int id;
        /** \brief pointer that allows for access to central QuickFit services */
        QFPluginServices* services;

        QString currentSaveDir;

        QFPropertyItemDelegate* propDelegate;

    signals:
        
    public slots:
        /** \brief called when the name editor changes its contents */
        void nameChanged(const QString& text);
        /** \brief called when the folder editor changes its contents */
        void folderChanged(const QString& text);
        /** \brief called when the role editor changes its contents */
        void roleChanged(const QString& text);

        void groupChanged(int group);

        /** \brief called when the description editor changes its contents */
        void descriptionChanged();
        /** \brief this will be connected to the project to indicate when  the currently
         *         displayed record should be deleted */
        void recordAboutToBeDeleted(QFRawDataRecord* r);
        /** \brief emitted when the "next" button is pressed */
        void nextPressed();
        /** \brief emitted when the "previous" button is pressed */
        void previousPressed();
        /** \brief emitted when the "next of same role" button is pressed */
        void nextOfRolePressed();
        /** \brief emitted when the "previous of same role" button is pressed */
        void previousOfRolePressed();
        /** \brief emitted when the raw data record data changes (i.e. ID, name, description, ...) */
        //void propsChanged(const QString &property=QString(""), bool visible=true);
        void basicPropsChanged();
        /** \brief insert a new property */
        void newPropClicked();
        /** \brief delete a property */
        void deletePropClicked();
        void makePropEditable();
        void copyProperties();
        void cutProperties();
        void pasteProperties();
        /** \brief resize the cells in the properties table */
        void resizePropertiesTable();
        /** \brief remove the current record */
        void deleteRecord();
        /** \brief remove the current record and it's group */
        void deleteRecordGroup();
        void tvResultsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        /** \brief delete selected results */
        void deleteSelectedResults();

        /** \brief save results to a file */
        void saveResults();
        void saveResultsAveraged();

        /** \brief display help dialog */
        void displayHelp();

        /** \brief display help dialog with the plugin help */
        void displayHelpPlugin();

        /** \brief display help dialog with the plugin tutorial */
        void displayHelpPluginTutorial();

        /** \brief display help dialog with the plugin copyright note */
        void displayHelpPluginCopyright();

        /** \brief display help dialog with the common RDR help */
        void displayHelpRDR();

        void copyValErrResults();
        void copyValErrResultsNoHead();
        void copyMedianQuantilesResults();
        void copyMedianQuantilesResultsNoHead();

        void copyResultsExpanded();
        void copyResultsExpandedNoHead();
        void copyResultsExpandedNoHeadMatlab();
        void copyResultsExpandedFlipped();
        void copyResultsExpandedNoHeadFlipped();
        void copyResultsExpandedNoHeadMatlabFlipped();

        void copyRDRID();
        void copyEvaluationID();
        void copyResultID();
        void copyResultAccessParserFunction();

        void filterEvaluationTextChanged(const QString& text);
        void filterResultsTextChanged(const QString& text);
        void currentTabChanged(int tab);
        void checkHelpAvailable();
        void resizePropertiesLater();


        void addFile();
        void removeFile();
        void editFile();
        void showEditFilesWarning(bool activated);
        void copyFile();
        void moveFilesUp();
        void moveFilesDown();
        void openFilesDirectory();
        void openFileInSystem();
        void showAvgClicked(bool checked);

        void showStatistics();
        void showStatisticsComparing();
        void showCorrelation();

        void setCurrent(QFRawDataRecord* c);

        void reloadGroupList();

        void selectRecordFromTreeClicked();
        void selectRecordFromTreeSelected(const QModelIndex& index);

        void exportData();
        void showData();
        void deleteAllVisibleResults();
        //void copyResultAccessParserFunctionTable();
};

#endif // QFRAWDATAPROPERTYEDITOR_PRIVATE_H
