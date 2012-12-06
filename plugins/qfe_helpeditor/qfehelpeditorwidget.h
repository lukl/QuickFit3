#ifndef QFEHelpEditorWidget_H
#define QFEHelpEditorWidget_H


#include <QWidget>
#include <QSettings>
#include <QAction>
#include <QToolButton>
#include "qtriple.h"
#include "qrecentfilesmenu.h"
#include "finddialog.h"
#include "replacedialog.h"
#include <QStringListModel>

namespace Ui {
    class QFEHelpEditorWidget;
}



class QFEHelpEditorWidget : public QWidget {
        Q_OBJECT

    public:
        explicit QFEHelpEditorWidget(QWidget* parent=NULL);
        ~QFEHelpEditorWidget();

        QString getScript() const;

        void loadSettings(QSettings& settings, QString prefix);
        void storeSettings(QSettings& settings, QString prefix) const;


    protected slots:
        void on_btnExecute_clicked();
        void on_btnNew_clicked();
        void on_btnOpen_clicked();
        void on_btnSave_clicked();
        void on_btnOpenExample_clicked();
        void on_btnOpenTemplate_clicked();
        void edtScript_cursorPositionChanged();
        void on_btnHelp_clicked();

        void openScript(QString dir, bool saveDir=true);
        void openScriptNoAsk(QString filename);







        /** \brief slot: find the first occurence of the text */
        void findFirst();

        /** \brief slot: find the next occurence of the text */
        void findNext();

        /** \brief slot: replace the first occurence of the text */
        void replaceFirst();

        /** \brief slot: replace the next occurence of the text */
        void replaceNext();

        /** \brief slot: goto a user selectable line in the text */
        void gotoLine();

        /** \brief slot: print the SDFF document */
        void print();

        /** \brief slot: preview print of the SDFF document */
        void printPreviewClick();

        /** \brief slot: used by printPreview() */
        void printPreview(QPrinter *printer);
        /** \brief set the "find next", ... actions to disabled (this is called whenever a new file is loaded or created) */
        void clearFindActions();

    protected:
        bool maybeSave();
        void setScriptFilename(QString filename);

        QString currentScript;
    private:
        Ui::QFEHelpEditorWidget *ui;
        QRecentFilesMenu* recentHelpFiles;

        QStringList defaultWords;
        QString lastScript;

        //QFQtScriptHighlighter* highlighter;
        QAction *cutAct;
        /** \brief action object: copy the currently selected text */
        QAction *copyAct;
        /** \brief action object: paste some text */
        QAction *pasteAct;
        /** \brief action object: undo text changes */
        QAction *undoAct;
        /** \brief action object: redo text changes */
        QAction *redoAct;
        /** \brief action object: find text */
        QAction *findAct;
        /** \brief action object: find next occurence text */
        QAction *findNextAct;
        /** \brief action object: find and replace text */
        QAction *replaceAct;
        /** \brief action object: run current SDFF script */
        QAction *runSDFFAct;
        /** \brief action object: comment text */
        QAction *commentAct;
        /** \brief action object: uncomment text */
        QAction *unCommentAct;
        /** \brief action object: incease indention */
        QAction *indentAct;
        /** \brief action object: decrease indention */
        QAction *unindentAct;
        /** \brief action object: goto line */
        QAction *gotoLineAct;
        /** \brief action object: print */
        QAction *printAct;

        QCompleter* completer;


        /** \brief the "Find ..." dialog object */
        FindDialog* findDlg;
        /** \brief the "Find & Replace..." dialog object */
        ReplaceDialog* replaceDlg;
        QStringListModel* completermodel;

        /** \brief load a completer model from a file */
        QStringListModel* modelFromFile(const QString& fileName);

};









#endif // QFEHelpEditorWidget_H