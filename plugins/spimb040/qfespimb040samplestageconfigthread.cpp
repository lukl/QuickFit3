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

#include "qfespimb040samplestageconfigthread.h"

#include "qfespimb040samplestageconfig.h"

#include<QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif


#include <iostream>

#define THREAD_TIMEOUT 77


QFESPIMB040SampleStageConfigThread::QFESPIMB040SampleStageConfigThread(QFESPIMB040SampleStageConfig* parent):
    QThread(parent)
    {

    m_parent=parent;
    qRegisterMetaType<QFExtensionLinearStage::AxisState>("QFExtensionLinearStage::AxisState");
    stopped=true;
    readCounter=0;
    connX=false;
    connY=false;
    connZ=false;
    connR=false;
    InstructionMutex=new QMutex(QMutex::Recursive);
}

QFESPIMB040SampleStageConfigThread::~QFESPIMB040SampleStageConfigThread() {
    stopThread();
    delete InstructionMutex;
}

void QFESPIMB040SampleStageConfigThread::run() {
    stopped=false;
    while (!stopped) {
        nextInstruction();
        msleep(THREAD_TIMEOUT);
    }
}

bool QFESPIMB040SampleStageConfigThread::anyConnected() const {
    return connX||connY||connZ||connR;
}

void QFESPIMB040SampleStageConfigThread::start(Priority priority ) {
    QThread::start(priority);
    //qDebug()<<"started";
}

void QFESPIMB040SampleStageConfigThread::nextInstruction() {
    //qDebug()<<"QFESPIMB040SampleStageConfigThread::nextInstruction()   stopped="<<stopped<<"  readCounter="<<readCounter;
    if (stopped) {
       /* qDebug()<<"   quit";*/
        quit();
    } else {
        if (m_parent->getXStage()) connX=m_parent->getXStage()->isConnected(m_parent->getXStageAxis()); else connX=false;
        if (m_parent->getYStage()) connY=m_parent->getYStage()->isConnected(m_parent->getYStageAxis()); else connY=false;
        if (m_parent->getZStage()) connZ=m_parent->getZStage()->isConnected(m_parent->getZStageAxis()); else connZ=false;
        if (m_parent->getRStage()) connR=m_parent->getRStage()->isConnected(m_parent->getRStageAxis()); else connR=false;
        InstructionMutex->lock();
        if (instructions.isEmpty()) {
            InstructionMutex->unlock();
            if (readCounter<=0) {
                /*connX=false;
                connY=false;
                connZ=false;*/
                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getXStage();
                axis=m_parent->getXStageAxis();
                if (stage) connX=stage->isConnected(axis);
                else connX=false;

                stage=m_parent->getYStage();
                axis=m_parent->getYStageAxis();
                if (stage) connY=stage->isConnected(axis);
                else connY=false;

                stage=m_parent->getZStage();
                axis=m_parent->getZStageAxis();
                if (stage) connZ=stage->isConnected(axis);
                else connZ=false;

                stage=m_parent->getRStage();
                axis=m_parent->getRStageAxis();
                if (stage) connR=stage->isConnected(axis);
                else connR=false;


                emit stagesConnectedChanged(connX,  connY,  connZ, connR);
            } else if (readCounter==1) {
                /////////////////////////////////////////////////////////////////////////////
                // read stage x state
                /////////////////////////////////////////////////////////////////////////////
                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getXStage();
                axis=m_parent->getXStageAxis();
                if (stage) {
                    QFExtensionLinearStage::AxisState state=stage->getAxisState(axis);
                    double position=stage->getPosition(axis);
                    double speed=stage->getSpeed(axis);
                    emit stageXMoved(state, position, speed);
                }
            } else if (readCounter==2) {
                /////////////////////////////////////////////////////////////////////////////
                // read stage y state
                /////////////////////////////////////////////////////////////////////////////
                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getYStage();
                axis=m_parent->getYStageAxis();
                if (stage) {
                    QFExtensionLinearStage::AxisState state=stage->getAxisState(axis);
                    double position=stage->getPosition(axis);
                    double speed=stage->getSpeed(axis);
                    emit stageYMoved(state, position, speed);
                }
            } else if (readCounter==3) {
                /////////////////////////////////////////////////////////////////////////////
                // read stage z state
                /////////////////////////////////////////////////////////////////////////////
                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getZStage();
                axis=m_parent->getZStageAxis();
                if (stage) {
                    QFExtensionLinearStage::AxisState state=stage->getAxisState(axis);
                    double position=stage->getPosition(axis);
                    double speed=stage->getSpeed(axis);
                    emit stageZMoved(state, position, speed);
                }

            } else if (readCounter==4) {
                /////////////////////////////////////////////////////////////////////////////
                // read stage Rot state
                /////////////////////////////////////////////////////////////////////////////
                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getRStage();
                axis=m_parent->getRStageAxis();
                if (stage) {
                    QFExtensionLinearStage::AxisState state=stage->getAxisState(axis);
                    double position=stage->getPosition(axis);
                    double speed=stage->getSpeed(axis);
                    emit stageRMoved(state, position, speed);
                }

            } else if (readCounter==5) {
                /////////////////////////////////////////////////////////////////////////////
                // read joystick
                /////////////////////////////////////////////////////////////////////////////
                bool joystick=false;
                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getXStage();
                axis=m_parent->getXStageAxis();
                if (stage) joystick=joystick|stage->isJoystickActive(axis);
                stage=m_parent->getYStage();
                axis=m_parent->getYStageAxis();
                if (stage) joystick=joystick|stage->isJoystickActive(axis);
                stage=m_parent->getZStage();
                axis=m_parent->getZStageAxis();
                if (stage) joystick=joystick|stage->isJoystickActive(axis);

                emit joystickStateChanged(joystick);

            } else if (readCounter==6) {
                /////////////////////////////////////////////////////////////////////////////
                // set joystick
                /////////////////////////////////////////////////////////////////////////////
                setJoystick(m_parent->isJoystickChecked(), m_parent->joystickMaxSpeed());

            } else {
                readCounter=-1;
            }
            readCounter++;
        } else {
            Instruction inst=instructions.dequeue();
            InstructionMutex->unlock();
            if (inst.type==QFESPIMB040SampleStageConfigThread::Move) {
                /////////////////////////////////////////////////////////////////////////////
                // move
                /////////////////////////////////////////////////////////////////////////////
                double x=inst.pd1;
                double y=inst.pd2;
                double z=inst.pd3;
                double R=inst.pd4;

                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getXStage();
                axis=m_parent->getXStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, x);
                    }
                }
                stage=m_parent->getYStage();
                axis=m_parent->getYStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, y);
                    }
                }
                stage=m_parent->getZStage();
                axis=m_parent->getZStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, z);
                    }
                }
                stage=m_parent->getRStage();
                axis=m_parent->getRStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, R);
                    }
                }
            } else if (inst.type==QFESPIMB040SampleStageConfigThread::MoveRel) {
                /////////////////////////////////////////////////////////////////////////////
                // move relative
                /////////////////////////////////////////////////////////////////////////////
                double x=inst.pd1;
                double y=inst.pd2;
                double z=inst.pd3;
                double R=inst.pd4;

                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getXStage();
                axis=m_parent->getXStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, stage->getPosition(axis)+x);
                    }
                }
                stage=m_parent->getYStage();
                axis=m_parent->getYStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, stage->getPosition(axis)+y);
                    }
                }
                stage=m_parent->getZStage();
                axis=m_parent->getZStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, stage->getPosition(axis)+z);
                    }
                }
                stage=m_parent->getRStage();
                axis=m_parent->getRStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->move(axis, stage->getPosition(axis)+R);
                    }
                }
            } else if (inst.type==QFESPIMB040SampleStageConfigThread::SetJoystick) {
                /////////////////////////////////////////////////////////////////////////////
                // set the joystick state
                /////////////////////////////////////////////////////////////////////////////
                double speed=inst.pd1;
                bool enabled=inst.pb1;
                QFExtensionLinearStage* stage;
                int axis;

                stage=m_parent->getXStage();
                axis=m_parent->getXStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->setJoystickActive(axis, enabled, speed);
                    }
                }
                stage=m_parent->getYStage();
                axis=m_parent->getYStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->setJoystickActive(axis, enabled, speed);
                    }
                }
                stage=m_parent->getZStage();
                axis=m_parent->getZStageAxis();
                if (stage) {
                    if (stage->isConnected(axis)) {
                        stage->setJoystickActive(axis, enabled, speed);
                    }
                }
            }
        }
        /*qDebug()<<"   restart timer";
        QTimer::singleShot(THREAD_TIMEOUT, this, SLOT(nextInstruction()));*/
    }

}

void QFESPIMB040SampleStageConfigThread::move(double x, double y, double z, double R) {
    QFESPIMB040SampleStageConfigThread::Instruction inst;
    inst.type=QFESPIMB040SampleStageConfigThread::Move;
    inst.pd1=x;
    inst.pd2=y;
    inst.pd3=z;
    inst.pd4=R;
    InstructionMutex->lock();
    instructions.enqueue(inst);
    InstructionMutex->unlock();
}


void QFESPIMB040SampleStageConfigThread::moveRel(double x, double y, double z, double R) {
    QFESPIMB040SampleStageConfigThread::Instruction inst;
    inst.type=QFESPIMB040SampleStageConfigThread::MoveRel;
    inst.pd1=x;
    inst.pd2=y;
    inst.pd3=z;
    inst.pd4=R;
    InstructionMutex->lock();
    instructions.enqueue(inst);
    InstructionMutex->unlock();
}

void QFESPIMB040SampleStageConfigThread::setJoystick(bool enabled, double maxSpeed) {
    QFESPIMB040SampleStageConfigThread::Instruction inst;
    inst.type=QFESPIMB040SampleStageConfigThread::SetJoystick;
    inst.pd1=maxSpeed;
    inst.pb1=enabled;
    InstructionMutex->lock();
    instructions.enqueue(inst);
    InstructionMutex->unlock();
}

void QFESPIMB040SampleStageConfigThread::stopThread() {
    stopped=true;
    if (isRunning()) wait(2*THREAD_TIMEOUT); // Thread timeout if not finishing
    //qDebug()<<"thread stopped";
}

