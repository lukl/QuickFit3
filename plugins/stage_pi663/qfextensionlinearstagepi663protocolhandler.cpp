/*
    Copyright (c) 2008-2015 Jan W. Krieger (<jan@jkrieger.de>, <j.krieger@dkfz.de>),
    German Cancer Research Center/University Heidelberg

    

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

#include "qfextensionlinearstagepi663protocolhandler.h"

QMap<QFSerialConnection*, QChar> QFExtensionLinearStagePI663ProtocolHandler::currentID=QMap<QFSerialConnection*, QChar>();

QFExtensionLinearStagePI663ProtocolHandler::QFExtensionLinearStagePI663ProtocolHandler(QFSerialConnection *com, QMutex* mutexSerial, QString name)
{
    this->com=com;
    this->log=NULL;
    this->LOG_PREFIX="";
    this->name=name;
    this->mutexSerial=mutexSerial;
    currentID[com]=QChar(0);
}

void QFExtensionLinearStagePI663ProtocolHandler::setLogging(QFPluginLogService* log, QString LOG_PREFIX) {
    this->log=log;
    this->LOG_PREFIX=LOG_PREFIX;
}

void QFExtensionLinearStagePI663ProtocolHandler::resetCurrentAxis()
{
    currentID[com]=QChar(0);
}

bool QFExtensionLinearStagePI663ProtocolHandler::checkComConnected()
{
    bool c=com->isConnectionOpen();

    return c;
}

//void QFExtensionLinearStagePI663ProtocolHandler::selectAxis(QChar ID)
//{
//    if (!currentID.contains(com) || currentID[com]!=ID) {
//        //std::cout<<"selecting motor "<<IDx<<"\n";
//        std::string s(2, ID.toLatin1());
//        s[0]=1;
//        com->write(s);
//        //com->write(s);
//        queryCommand("TB");
//        currentID[com]=ID;
//    }
//}

void QFExtensionLinearStagePI663ProtocolHandler::sendCommand(std::string command)
{
    com->write(command+"\x00a");
}

void QFExtensionLinearStagePI663ProtocolHandler::sendCommandSingleChar(std::string command)
{
    com->write(command);
}

std::string QFExtensionLinearStagePI663ProtocolHandler::queryCommand(std::string command)
{
    std::string res="";
    //std::cout<<"\n\ncommand (stage "<<currentID<<"): '"<<command<<"'";
    com->clearBuffer();
    if (com->write(command+"\x00a")) {
        res=com->readUntil(10);
        //std::cout<<" ... reading ... ";
    }
    //std::cout<<"   direct_result: '"<<toprintablestr(res)<<"' ";
    if (res.size()>=1) res=res.erase(res.size()-1, 1);
    //std::cout<<"   returned_result: '"<<toprintablestr(res)<<"'\n\n";
    return res;
}

std::string QFExtensionLinearStagePI663ProtocolHandler::queryCommandSingleChar(std::string command)
{
    std::string res="";
    //std::cout<<"\n\ncommand (stage "<<currentID<<"): '"<<command<<"'";
    com->clearBuffer();
    if (com->write(command)) {
        res=com->readUntil(10);
        //std::cout<<" ... reading ... ";
    }
    //std::cout<<"   direct_result: '"<<toprintablestr(res)<<"' ";
    //if (res.size()>=3) res=res.erase(res.size()-3, 3);
    //std::cout<<"   returned_result: '"<<toprintablestr(res)<<"'\n\n";
    return res;
}

QMutex *QFExtensionLinearStagePI663ProtocolHandler::getMutex() const
{
    return mutexSerial;
}

QFSerialConnection *QFExtensionLinearStagePI663ProtocolHandler::getCOM() const
{
    return com;
}


