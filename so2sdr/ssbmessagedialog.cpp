/*! Copyright 2010-2016 R. Torsten Clay N4OGW

   This file is part of so2sdr.

    so2sdr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    so2sdr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with so2sdr.  If not, see <http://www.gnu.org/licenses/>.

 */
#include <QPushButton>
#include <QDialog>
#include <QFormLayout>
#include <QSettings>
#include "ssbmessagedialog.h"
#include "defines.h"

SSBMessageDialog::SSBMessageDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    upperValidate = new UpperValidator(this);
    connect(ssbmessage_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(ssbmessage_buttons, SIGNAL(accepted()), this, SLOT(updateSSBMsg()));

    nowRecording=0;
    recGroup.setExclusive(false);

    funcEditPtr[0]  = cq_f1_edit;
    funcEditPtr[1]  = cq_f2_edit;
    funcEditPtr[2]  = cq_f3_edit;
    funcEditPtr[3]  = cq_f4_edit;
    funcEditPtr[4]  = cq_f5_edit;
    funcEditPtr[5]  = cq_f6_edit;
    funcEditPtr[6]  = cq_f7_edit;
    funcEditPtr[7]  = cq_f8_edit;
    funcEditPtr[8]  = cq_f9_edit;
    funcEditPtr[9]  = cq_f10_edit;
    funcEditPtr[10] = cq_f11_edit;
    funcEditPtr[11] = cq_f12_edit;

    funcRecPtr[0]   = rec1;
    funcRecPtr[1]   = rec2;
    funcRecPtr[2]   = rec3;
    funcRecPtr[3]   = rec4;
    funcRecPtr[4]   = rec5;
    funcRecPtr[5]   = rec6;
    funcRecPtr[6]   = rec7;
    funcRecPtr[7]   = rec8;
    funcRecPtr[8]   = rec9;
    funcRecPtr[9]   = rec10;
    funcRecPtr[10]   = rec11;
    funcRecPtr[11]   = rec12;

    for (int i=0;i<12;i++) {
        funcRecPtr[i]->setCheckable(true);
        recGroup.addButton(funcRecPtr[i],i+1);
    }

    ctrlFuncEditPtr[0]  = cq_ctrl_f1_edit;
    ctrlFuncEditPtr[1]  = cq_ctrl_f2_edit;
    ctrlFuncEditPtr[2]  = cq_ctrl_f3_edit;
    ctrlFuncEditPtr[3]  = cq_ctrl_f4_edit;
    ctrlFuncEditPtr[4]  = cq_ctrl_f5_edit;
    ctrlFuncEditPtr[5]  = cq_ctrl_f6_edit;
    ctrlFuncEditPtr[6]  = cq_ctrl_f7_edit;
    ctrlFuncEditPtr[7]  = cq_ctrl_f8_edit;
    ctrlFuncEditPtr[8]  = cq_ctrl_f9_edit;
    ctrlFuncEditPtr[9]  = cq_ctrl_f10_edit;
    ctrlFuncEditPtr[10] = cq_ctrl_f11_edit;
    ctrlFuncEditPtr[11] = cq_ctrl_f12_edit;

    ctrlFuncRecPtr[0]   = recCtrl1;
    ctrlFuncRecPtr[1]   = recCtrl2;
    ctrlFuncRecPtr[2]   = recCtrl3;
    ctrlFuncRecPtr[3]   = recCtrl4;
    ctrlFuncRecPtr[4]   = recCtrl5;
    ctrlFuncRecPtr[5]   = recCtrl6;
    ctrlFuncRecPtr[6]   = recCtrl7;
    ctrlFuncRecPtr[7]   = recCtrl8;
    ctrlFuncRecPtr[8]   = recCtrl9;
    ctrlFuncRecPtr[9]   = recCtrl10;
    ctrlFuncRecPtr[10]   = recCtrl11;
    ctrlFuncRecPtr[11]   = recCtrl12;

    for (int i=0;i<12;i++) {
        ctrlFuncRecPtr[i]->setCheckable(true);
        recGroup.addButton(ctrlFuncRecPtr[i],i+13);
    }

    shiftFuncEditPtr[0]  = cq_shift_f1_edit;
    shiftFuncEditPtr[1]  = cq_shift_f2_edit;
    shiftFuncEditPtr[2]  = cq_shift_f3_edit;
    shiftFuncEditPtr[3]  = cq_shift_f4_edit;
    shiftFuncEditPtr[4]  = cq_shift_f5_edit;
    shiftFuncEditPtr[5]  = cq_shift_f6_edit;
    shiftFuncEditPtr[6]  = cq_shift_f7_edit;
    shiftFuncEditPtr[7]  = cq_shift_f8_edit;
    shiftFuncEditPtr[8]  = cq_shift_f9_edit;
    shiftFuncEditPtr[9]  = cq_shift_f10_edit;
    shiftFuncEditPtr[10] = cq_shift_f11_edit;
    shiftFuncEditPtr[11] = cq_shift_f12_edit;

    shiftFuncRecPtr[0]   = recShft1;
    shiftFuncRecPtr[1]   = recShft2;
    shiftFuncRecPtr[2]   = recShft3;
    shiftFuncRecPtr[3]   = recShft4;
    shiftFuncRecPtr[4]   = recShft5;
    shiftFuncRecPtr[5]   = recShft6;
    shiftFuncRecPtr[6]   = recShft7;
    shiftFuncRecPtr[7]   = recShft8;
    shiftFuncRecPtr[8]   = recShft9;
    shiftFuncRecPtr[9]   = recShft10;
    shiftFuncRecPtr[10]   = recShft11;
    shiftFuncRecPtr[11]   = recShft12;

    for (int i=0;i<12;i++) {
        shiftFuncRecPtr[i]->setCheckable(true);
        recGroup.addButton(shiftFuncRecPtr[i],i+25);
    }

    excFuncEditPtr[0]  = exc_f1_edit;
    excFuncEditPtr[1]  = exc_f2_edit;
    excFuncEditPtr[2]  = exc_f3_edit;
    excFuncEditPtr[3]  = exc_f4_edit;
    excFuncEditPtr[4]  = exc_f5_edit;
    excFuncEditPtr[5]  = exc_f6_edit;
    excFuncEditPtr[6]  = exc_f7_edit;
    excFuncEditPtr[7]  = exc_f8_edit;
    excFuncEditPtr[8]  = exc_f9_edit;
    excFuncEditPtr[9]  = exc_f10_edit;
    excFuncEditPtr[10] = exc_f11_edit;
    excFuncEditPtr[11] = exc_f12_edit;

    excFuncRecPtr[0]   = recExc1;
    excFuncRecPtr[1]   = recExc2;
    excFuncRecPtr[2]   = recExc3;
    excFuncRecPtr[3]   = recExc4;
    excFuncRecPtr[4]   = recExc5;
    excFuncRecPtr[5]   = recExc6;
    excFuncRecPtr[6]   = recExc7;
    excFuncRecPtr[7]   = recExc8;
    excFuncRecPtr[8]   = recExc9;
    excFuncRecPtr[9]   = recExc10;
    excFuncRecPtr[10]   = recExc11;
    excFuncRecPtr[11]   = recExc12;

    for (int i=0;i<12;i++) {
        excFuncRecPtr[i]->setCheckable(true);
        recGroup.addButton(excFuncRecPtr[i],i+37);
    }
    recQsl->setCheckable(true);
    recGroup.addButton(recQsl,49);

    recQuick->setCheckable(true);
    recGroup.addButton(recQuick,50);

    recDupe->setCheckable(true);
    recGroup.addButton(recDupe,51);

    recCallUpdate->setCheckable(true);
    recGroup.addButton(recCallUpdate,52);

    recCqExc->setCheckable(true);
    recGroup.addButton(recCqExc,53);

    connect(&recGroup,SIGNAL(buttonClicked(int)),this,SLOT(recButtons(int)));

    for (int i = 0; i < N_FUNC; i++) {
        funcEditPtr[i]->setValidator(upperValidate);
        ctrlFuncEditPtr[i]->setValidator(upperValidate);
        shiftFuncEditPtr[i]->setValidator(upperValidate);
        excFuncEditPtr[i]->setValidator(upperValidate);
        cqF[i].clear();
        cqCtrlF[i].clear();
        cqShiftF[i].clear();
        excF[i].clear();
    }
    qsl_msg_edit->setValidator(upperValidate);
    qsl_updated_edit->setValidator(upperValidate);
    cq_exc_edit->setValidator(upperValidate);
    sp_exc_edit->setValidator(upperValidate);
    dupe_msg_edit->setValidator(upperValidate);
    quick_qsl_edit->setValidator(upperValidate);
}

void SSBMessageDialog::recButtons(int id)
{
    int nr=id-1;
    // if this msg was already recording, stop
    if (nowRecording==id) {
        recGroup.button(nowRecording)->setChecked(false);
        nowRecording=0;
        qDebug("id=%d finished",id);
        emit(stopRecording(nr));
    } else if (nowRecording) {
        // some other was recording-cancel it
        qDebug("cancelling %d",nowRecording);
        emit(stopRecording(nowRecording-1));
        recGroup.button(nowRecording)->setChecked(false);
        nowRecording=id;
        emit(startRecording(nr));
    } else {
        qDebug("starting id=%d",id);
        nowRecording=id;
        emit(startRecording(nr));
    }
    qDebug(" ");
}

/*!
 * \brief SSBMessageDialog::initialize Initialize dialog from settings file
 * \param s  contest settings file
  */
void SSBMessageDialog::initialize(QSettings *s)
{
    settings=s;

    // function keys: cq
    int sz=settings->beginReadArray(c_cq_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        funcEditPtr[i]->setText(settings->value("func",c_cq_func_def[m]).toString());
        cqF[i]=funcEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: exchange
    sz=settings->beginReadArray(c_ex_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        excFuncEditPtr[i]->setText(settings->value("func",c_ex_func_def[m]).toString());
        excF[i]=excFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: shift
    sz=settings->beginReadArray(c_shift_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        shiftFuncEditPtr[i]->setText(settings->value("func",c_shift_func_def[m]).toString());
        cqShiftF[i]=shiftFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: control
    sz=settings->beginReadArray(c_ctrl_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        ctrlFuncEditPtr[i]->setText(settings->value("func",c_ctrl_func_def[m]).toString());
        cqCtrlF[i]=ctrlFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // other special messages
    sp_exc_edit->setText(settings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    cq_exc_edit->setText(settings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    qsl_msg_edit->setText(settings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_updated_edit->setText(settings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    dupe_msg_edit->setText(settings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    quick_qsl_edit->setText(settings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
}


/*! slot called when cancel pressed: reset all line edits to
   stored values and close dialog */
void SSBMessageDialog::rejectChanges()
{
    if (nowRecording) emit(stopRecording(nowRecording-1));
    for (int i = 0; i < N_FUNC; i++) {
        funcEditPtr[i]->setText(cqF[i]);
        ctrlFuncEditPtr[i]->setText(cqCtrlF[i]);
        shiftFuncEditPtr[i]->setText(cqShiftF[i]);
        excFuncEditPtr[i]->setText(excF[i]);
    }

    qsl_msg_edit->setText(settings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_updated_edit->setText(settings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    cq_exc_edit->setText(settings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    sp_exc_edit->setText(settings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    dupe_msg_edit->setText(settings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    quick_qsl_edit->setText(settings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
    reject();
}

SSBMessageDialog::~SSBMessageDialog()
{
    delete upperValidate;
}

/*!
   Update messages from form
 */
void SSBMessageDialog::updateSSBMsg()
{
    // function keys: cq
    settings->beginWriteArray(c_cq_func[m],N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        settings->setArrayIndex(i);
        settings->setValue("func",funcEditPtr[i]->text());
        cqF[i]=funcEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: ctrl+func
    settings->beginWriteArray(c_ctrl_func[m],N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        settings->setArrayIndex(i);
        settings->setValue("func",ctrlFuncEditPtr[i]->text());
        cqCtrlF[i]=ctrlFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: shift+func
    settings->beginWriteArray(c_shift_func[m],N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        settings->setArrayIndex(i);
        settings->setValue("func",shiftFuncEditPtr[i]->text());
            cqShiftF[i]=shiftFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: exchange
    settings->beginWriteArray(c_ex_func[m],N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        settings->setArrayIndex(i);
        settings->setValue("func",excFuncEditPtr[i]->text());
        excF[i]=excFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    settings->setValue(c_cq_exc[m],cq_exc_edit->text());
    settings->setValue(c_sp_exc[m],sp_exc_edit->text());
    settings->setValue(c_qsl_msg[m],qsl_msg_edit->text());
    settings->setValue(c_qsl_msg_updated[m],qsl_updated_edit->text());
    settings->setValue(c_qqsl_msg[m],quick_qsl_edit->text());
    settings->setValue(c_dupe_msg[m],dupe_msg_edit->text());
    settings->sync();
    accept();
}
