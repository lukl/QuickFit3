#include "optionswidget.h"
#include "ui_optionswidget.h"
#include "qfstyledbutton.h"
#include <QFile>
#include "programoptions.h"
#include "qfstyledbutton.h"
#include "qfpluginservices.h"

OptionsWidget::OptionsWidget(QObject* pluginObject, QWidget *parent) :
    QFPluginOptionsWidget(pluginObject, parent),
    ui(new Ui::OptionsWidget)
{
    ui->setupUi(this);
    QFStyledButton* btn=new QFStyledButton(QFStyledButton::SelectFile, ui->edtUserDatabase, ui->edtUserDatabase);
    btn->setFilter("user database files (*.ini)");
    ui->edtUserDatabase->addButton(btn);
}

OptionsWidget::~OptionsWidget()
{
    delete ui;
}

void OptionsWidget::readSettings(ProgramOptions *options)
{
    ui->edtUserDatabase->setText(options->getConfigValue("calc_diffcoeff/user_database", QFPluginServices::getInstance()->getConfigFileDirectory()+"/plugins/calc_diffcoeff/mysamples.ini").toString());
}

void OptionsWidget::writeSettings(ProgramOptions *options)
{

    options->setConfigValue("calc_diffcoeff/user_database", ui->edtUserDatabase->text());
}