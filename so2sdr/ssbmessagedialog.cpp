/*! Copyright 2010-2017 R. Torsten Clay N4OGW

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

    recGroup.setExclusive(false);
    excRecGroup.setExclusive(false);
    otherRecGroup.setExclusive(false);

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

    funcRecEditPtr[0]  = cq_f1_rec_edit;
    funcRecEditPtr[1]  = cq_f2_rec_edit;
    funcRecEditPtr[2]  = cq_f3_rec_edit;
    funcRecEditPtr[3]  = cq_f4_rec_edit;
    funcRecEditPtr[4]  = cq_f5_rec_edit;
    funcRecEditPtr[5]  = cq_f6_rec_edit;
    funcRecEditPtr[6]  = cq_f7_rec_edit;
    funcRecEditPtr[7]  = cq_f8_rec_edit;
    funcRecEditPtr[8]  = cq_f9_rec_edit;
    funcRecEditPtr[9]  = cq_f10_rec_edit;
    funcRecEditPtr[10] = cq_f11_rec_edit;
    funcRecEditPtr[11] = cq_f12_rec_edit;

    funcRecPtr[0]   = recF1;
    funcRecPtr[1]   = recF2;
    funcRecPtr[2]   = recF3;
    funcRecPtr[3]   = recF4;
    funcRecPtr[4]   = recF5;
    funcRecPtr[5]   = recF6;
    funcRecPtr[6]   = recF7;
    funcRecPtr[7]   = recF8;
    funcRecPtr[8]   = recF9;
    funcRecPtr[9]   = recF10;
    funcRecPtr[10]   = recF11;
    funcRecPtr[11]   = recF12;

    for (int i=0;i<N_FUNC;i++) {
        funcRecPtr[i]->setCheckable(true);
        recGroup.addButton(funcRecPtr[i],i);
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

    excFuncRecEditPtr[0]  = exc_f1_rec_edit;
    excFuncRecEditPtr[1]  = exc_f2_rec_edit;
    excFuncRecEditPtr[2]  = exc_f3_rec_edit;
    excFuncRecEditPtr[3]  = exc_f4_rec_edit;
    excFuncRecEditPtr[4]  = exc_f5_rec_edit;
    excFuncRecEditPtr[5]  = exc_f6_rec_edit;
    excFuncRecEditPtr[6]  = exc_f7_rec_edit;
    excFuncRecEditPtr[7]  = exc_f8_rec_edit;
    excFuncRecEditPtr[8]  = exc_f9_rec_edit;
    excFuncRecEditPtr[9]  = exc_f10_rec_edit;
    excFuncRecEditPtr[10] = exc_f11_rec_edit;
    excFuncRecEditPtr[11] = exc_f12_rec_edit;

    excFuncRecPtr[0]   = recExcF1;
    excFuncRecPtr[1]   = recExcF2;
    excFuncRecPtr[2]   = recExcF3;
    excFuncRecPtr[3]   = recExcF4;
    excFuncRecPtr[4]   = recExcF5;
    excFuncRecPtr[5]   = recExcF6;
    excFuncRecPtr[6]   = recExcF7;
    excFuncRecPtr[7]   = recExcF8;
    excFuncRecPtr[8]   = recExcF9;
    excFuncRecPtr[9]   = recExcF10;
    excFuncRecPtr[10]   = recExcF11;
    excFuncRecPtr[11]   = recExcF12;

    for (int i=0;i<N_FUNC;i++) {
        excFuncRecPtr[i]->setCheckable(true);
        excRecGroup.addButton(excFuncRecPtr[i],i);
    }

    recQsl->setCheckable(true);
    otherRecGroup.addButton(recQsl,0);

    recQuick->setCheckable(true);
    otherRecGroup.addButton(recQuick,1);

    recDupe->setCheckable(true);
    otherRecGroup.addButton(recDupe,2);

    recCallUpdate->setCheckable(true);
    otherRecGroup.addButton(recCallUpdate,3);

    recCqExc->setCheckable(true);
    otherRecGroup.addButton(recCqExc,4);

    recSpExc->setCheckable(true);
    otherRecGroup.addButton(recSpExc,5);

    recCall->setCheckable(true);
    otherRecGroup.addButton(recCall,6);

    connect(&recGroup,SIGNAL(buttonClicked(int)),this,SLOT(recButtons(int)));
    connect(&excRecGroup,SIGNAL(buttonClicked(int)),this,SLOT(excRecButtons(int)));
    connect(&otherRecGroup,SIGNAL(buttonClicked(int)),this,SLOT(otherRecButtons(int)));

    for (int i = 0; i < N_FUNC; i++) {
        funcEditPtr[i]->setValidator(upperValidate);
        funcRecEditPtr[i]->setValidator(upperValidate);
        excFuncEditPtr[i]->setValidator(upperValidate);
        excFuncRecEditPtr[i]->setValidator(upperValidate);
        cqF[i].clear();
        excF[i].clear();
        cqRecF[i].clear();
        excRecF[i].clear();
    }

    call_edit->setValidator(upperValidate);
    qsl_msg_edit->setValidator(upperValidate);
    qsl_msg_rec_edit->setValidator(upperValidate);
    qsl_updated_edit->setValidator(upperValidate);
    qsl_updated_rec_edit->setValidator(upperValidate);
    cq_exc_edit->setValidator(upperValidate);
    cq_exc_rec_edit->setValidator(upperValidate);
    sp_exc_edit->setValidator(upperValidate);
    sp_exc_rec_edit->setValidator(upperValidate);
    dupe_msg_edit->setValidator(upperValidate);
    dupe_msg_rec_edit->setValidator(upperValidate);
    quick_qsl_edit->setValidator(upperValidate);
    quick_qsl_rec_edit->setValidator(upperValidate);
    cancel_edit->setValidator(upperValidate);
}

void SSBMessageDialog::recButtons(int id)
{
    emit(recordMsg(funcRecEditPtr[id]->text().toLatin1(),false));
}

void SSBMessageDialog::excRecButtons(int id)
{
    emit(recordMsg(excFuncRecEditPtr[id]->text().toLatin1(),false));
}

void SSBMessageDialog::otherRecButtons(int id)
{
    switch (id) {
    case 0:
        emit(recordMsg(qsl_msg_rec_edit->text().toLatin1(),false));
        break;
    case 1:
        emit(recordMsg(quick_qsl_rec_edit->text().toLatin1(),false));
        break;
    case 2:
        emit(recordMsg(dupe_msg_rec_edit->text().toLatin1(),false));
        break;
    case 3:
        emit(recordMsg(qsl_updated_rec_edit->text().toLatin1(),false));
        break;
    case 4:
        emit(recordMsg(cq_exc_rec_edit->text().toLatin1(),false));
        break;
    case 5:
        emit(recordMsg(sp_exc_rec_edit->text().toLatin1(),false));
        break;
    case 6:
        emit(recordMsg(call_rec_edit->text().toLatin1(),false));
    }
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
        funcEditPtr[i]->setCursorPosition(0);
        cqF[i]=funcEditPtr[i]->text().toLatin1();
    }
    settings->endArray();

    // function keys: cq, record
    sz=settings->beginReadArray(c_cq_rec_func);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        funcRecEditPtr[i]->setText(settings->value("func",c_cq_rec_func_def).toString());
        funcRecEditPtr[i]->setCursorPosition(0);
        cqRecF[i]=funcRecEditPtr[i]->text().toLatin1();
    }
    settings->endArray();

    // function keys: exchange
    sz=settings->beginReadArray(c_ex_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        excFuncEditPtr[i]->setText(settings->value("func",c_ex_func_def[m]).toString());
        excFuncEditPtr[i]->setCursorPosition(0);
        excF[i]=excFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();

    // function keys: ex, record
    sz=settings->beginReadArray(c_ex_rec_func);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        excFuncRecEditPtr[i]->setText(settings->value("func",c_ex_rec_func_def).toString());
        excFuncRecEditPtr[i]->setCursorPosition(0);
        excRecF[i]=excFuncRecEditPtr[i]->text().toLatin1();
    }
    settings->endArray();

    // other special messages
    call_edit->setText(settings->value(c_call_msg,c_call_msg_def).toString());
    call_edit->setCursorPosition(0);
    call_rec_edit->setText(settings->value(c_call_msg_rec,c_call_msg_rec_def).toString());
    call_rec_edit->setCursorPosition(0);

    sp_exc_edit->setText(settings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    sp_exc_edit->setCursorPosition(0);
    sp_exc_rec_edit->setText(settings->value(c_sp_exc_rec,c_sp_exc_rec_def).toString());
    sp_exc_rec_edit->setCursorPosition(0);

    cq_exc_edit->setText(settings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    cq_exc_edit->setCursorPosition(0);
    cq_exc_rec_edit->setText(settings->value(c_cq_exc_rec,c_cq_exc_rec_def).toString());
    cq_exc_rec_edit->setCursorPosition(0);

    qsl_msg_edit->setText(settings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_msg_edit->setCursorPosition(0);
    qsl_msg_rec_edit->setText(settings->value(c_qsl_msg_rec,c_qsl_msg_rec_def).toString());
    qsl_msg_rec_edit->setCursorPosition(0);

    qsl_updated_edit->setText(settings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    qsl_updated_edit->setCursorPosition(0);
    qsl_updated_rec_edit->setText(settings->value(c_qsl_msg_updated_rec,c_qsl_msg_updated_rec_def).toString());
    qsl_updated_rec_edit->setCursorPosition(0);

    dupe_msg_edit->setText(settings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    dupe_msg_edit->setCursorPosition(0);
    dupe_msg_rec_edit->setText(settings->value(c_dupe_msg_rec,c_dupe_msg_rec_def).toString());
    dupe_msg_rec_edit->setCursorPosition(0);

    quick_qsl_edit->setText(settings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
    quick_qsl_edit->setCursorPosition(0);
    quick_qsl_rec_edit->setText(settings->value(c_qqsl_msg_rec,c_qqsl_msg_rec_def).toString());
    quick_qsl_rec_edit->setCursorPosition(0);

    cancel_edit->setText(settings->value(c_ssb_cancel,c_ssb_cancel_def).toString());
    cancel_edit->setCursorPosition(0);
}


/*! slot called when cancel pressed: reset all line edits to
   stored values and close dialog */
void SSBMessageDialog::rejectChanges()
{
    for (int i = 0; i < N_FUNC; i++) {
        funcEditPtr[i]->setText(cqF[i]);
        funcEditPtr[i]->setCursorPosition(0);
        funcRecEditPtr[i]->setText(cqRecF[i]);
        funcRecEditPtr[i]->setCursorPosition(0);
        excFuncEditPtr[i]->setText(excF[i]);
        excFuncEditPtr[i]->setCursorPosition(0);
        excFuncRecEditPtr[i]->setText(excRecF[i]);
        excFuncRecEditPtr[i]->setCursorPosition(0);
    }
    call_edit->setText(settings->value(c_call_msg,c_call_msg_def).toString());
    call_edit->setCursorPosition(0);
    call_rec_edit->setText(settings->value(c_call_msg_rec,c_call_msg_rec_def).toString());
    call_rec_edit->setCursorPosition(0);
    cancel_edit->setText(settings->value(c_ssb_cancel,c_ssb_cancel_def).toString());
    cancel_edit->setCursorPosition(0);
    qsl_msg_edit->setText(settings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_msg_edit->setCursorPosition(0);
    qsl_msg_rec_edit->setText(settings->value(c_qsl_msg_rec,c_qsl_msg_rec_def).toString());
    qsl_msg_rec_edit->setCursorPosition(0);
    qsl_updated_edit->setText(settings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    qsl_updated_edit->setCursorPosition(0);
    qsl_updated_rec_edit->setText(settings->value(c_qsl_msg_updated_rec,c_qsl_msg_updated_rec_def).toString());
    qsl_updated_rec_edit->setCursorPosition(0);
    cq_exc_edit->setText(settings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    cq_exc_edit->setCursorPosition(0);
    cq_exc_rec_edit->setText(settings->value(c_cq_exc_rec,c_cq_exc_rec_def).toString());
    cq_exc_rec_edit->setCursorPosition(0);
    sp_exc_edit->setText(settings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    sp_exc_edit->setCursorPosition(0);
    sp_exc_rec_edit->setText(settings->value(c_sp_exc_rec,c_sp_exc_rec_def).toString());
    sp_exc_rec_edit->setCursorPosition(0);
    dupe_msg_edit->setText(settings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    dupe_msg_edit->setCursorPosition(0);
    dupe_msg_rec_edit->setText(settings->value(c_dupe_msg_rec,c_dupe_msg_rec_def).toString());
    dupe_msg_rec_edit->setCursorPosition(0);
    quick_qsl_edit->setText(settings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
    quick_qsl_edit->setCursorPosition(0);
    quick_qsl_rec_edit->setText(settings->value(c_qqsl_msg_rec,c_qqsl_msg_rec_def).toString());
    quick_qsl_rec_edit->setCursorPosition(0);
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

    // function keys: cq record
    settings->beginWriteArray(c_cq_rec_func,N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        settings->setArrayIndex(i);
        settings->setValue("func",funcRecEditPtr[i]->text());
        cqRecF[i]=funcRecEditPtr[i]->text().toLatin1();
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

    // function keys: exc record
    settings->beginWriteArray(c_ex_rec_func,N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        settings->setArrayIndex(i);
        settings->setValue("func",excFuncRecEditPtr[i]->text());
        excRecF[i]=excFuncRecEditPtr[i]->text().toLatin1();
    }
    settings->endArray();

    settings->setValue(c_call_msg,call_edit->text());
    settings->setValue(c_call_msg_rec,call_rec_edit->text());
    settings->setValue(c_ssb_cancel,cancel_edit->text());
    settings->setValue(c_cq_exc[m],cq_exc_edit->text());
    settings->setValue(c_cq_exc_rec,cq_exc_rec_edit->text());
    settings->setValue(c_sp_exc[m],sp_exc_edit->text());
    settings->setValue(c_sp_exc_rec,sp_exc_rec_edit->text());
    settings->setValue(c_qsl_msg[m],qsl_msg_edit->text());
    settings->setValue(c_qsl_msg_rec,qsl_msg_rec_edit->text());
    settings->setValue(c_qsl_msg_updated[m],qsl_updated_edit->text());
    settings->setValue(c_qsl_msg_updated_rec,qsl_updated_rec_edit->text());
    settings->setValue(c_qqsl_msg[m],quick_qsl_edit->text());
    settings->setValue(c_qqsl_msg_rec,quick_qsl_rec_edit->text());
    settings->setValue(c_dupe_msg[m],dupe_msg_edit->text());
    settings->setValue(c_dupe_msg_rec,dupe_msg_rec_edit->text());
    settings->sync();
    accept();
}
