/*! Copyright 2010-2021 R. Torsten Clay N4OGW

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
#include <QProcess>
#include <QPushButton>
#include <QDialog>
#include <QFormLayout>
#include <QSettings>
#include <QTextDocument>
#include "ssbmessagedialog.h"
#include "defines.h"

SSBMessageDialog::SSBMessageDialog(uiSize sizes, QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    upperValidate = new UpperValidator(this);
    connect(ssbmessage_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(ssbmessage_buttons, SIGNAL(accepted()), this, SLOT(updateSSBMsg()));

    recGroup.setExclusive(false);
    excRecGroup.setExclusive(false);
    otherRecGroup.setExclusive(false);
    otherPlayGroup.setExclusive(false);
    playGroup.setExclusive(false);
    excPlayGroup.setExclusive(false);

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
    for (int i=0;i<12;i++) {
        funcEditPtr[i]->setFixedWidth(qRound(sizes.width*16));
    }

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
    for (int i=0;i<12;i++) {
        funcRecEditPtr[i]->setFixedWidth(qRound(sizes.width*16));
    }
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

    funcPlayPtr[0]   = playF1;
    funcPlayPtr[1]   = playF2;
    funcPlayPtr[2]   = playF3;
    funcPlayPtr[3]   = playF4;
    funcPlayPtr[4]   = playF5;
    funcPlayPtr[5]   = playF6;
    funcPlayPtr[6]   = playF7;
    funcPlayPtr[7]   = playF8;
    funcPlayPtr[8]   = playF9;
    funcPlayPtr[9]   = playF10;
    funcPlayPtr[10]   = playF11;
    funcPlayPtr[11]   = playF12;


    for (int i=0;i<N_FUNC;i++) {
        funcRecPtr[i]->setCheckable(true);
        recGroup.addButton(funcRecPtr[i],i);
        playGroup.addButton(funcPlayPtr[i],i);
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
    for (int i=0;i<12;i++) {
        excFuncEditPtr[i]->setFixedWidth(qRound(sizes.width*16));
    }
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
    for (int i=0;i<12;i++) {
        excFuncRecEditPtr[i]->setFixedWidth(qRound(sizes.width*16));
    }
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

    excFuncPlayPtr[0]   = playExcF1;
    excFuncPlayPtr[1]   = playExcF2;
    excFuncPlayPtr[2]   = playExcF3;
    excFuncPlayPtr[3]   = playExcF4;
    excFuncPlayPtr[4]   = playExcF5;
    excFuncPlayPtr[5]   = playExcF6;
    excFuncPlayPtr[6]   = playExcF7;
    excFuncPlayPtr[7]   = playExcF8;
    excFuncPlayPtr[8]   = playExcF9;
    excFuncPlayPtr[9]   = playExcF10;
    excFuncPlayPtr[10]   = playExcF11;
    excFuncPlayPtr[11]   = playExcF12;

    for (int i=0;i<N_FUNC;i++) {
        excFuncRecPtr[i]->setCheckable(true);
        excRecGroup.addButton(excFuncRecPtr[i],i);
        excPlayGroup.addButton(excFuncPlayPtr[i],i);
    }

    recQsl->setCheckable(true);
    otherRecGroup.addButton(recQsl,0);
    otherPlayGroup.addButton(playQslMsg,0);

    recQuick->setCheckable(true);
    otherRecGroup.addButton(recQuick,1);
    otherPlayGroup.addButton(playQuickQsl,1);

    recDupe->setCheckable(true);
    otherRecGroup.addButton(recDupe,2);
    otherPlayGroup.addButton(playDupeMsg,2);

    recCallUpdate->setCheckable(true);
    otherRecGroup.addButton(recCallUpdate,3);
    otherPlayGroup.addButton(playCallUpdated,3);

    recCqExc->setCheckable(true);
    otherRecGroup.addButton(recCqExc,4);
    otherPlayGroup.addButton(playCqExch,4);

    recSpExc->setCheckable(true);
    otherRecGroup.addButton(recSpExc,5);
    otherPlayGroup.addButton(playSpExch,5);

    recCall->setCheckable(true);
    otherRecGroup.addButton(recCall,6);
    otherPlayGroup.addButton(playCall,6);

    connect(&recGroup,SIGNAL(buttonClicked(int)),this,SLOT(recButtons(int)));
    connect(&excRecGroup,SIGNAL(buttonClicked(int)),this,SLOT(excRecButtons(int)));
    connect(&playGroup,SIGNAL(buttonClicked(int)),this,SLOT(playButtons(int)));
    connect(&excPlayGroup,SIGNAL(buttonClicked(int)),this,SLOT(playExcButtons(int)));
    connect(&otherRecGroup,SIGNAL(buttonClicked(int)),this,SLOT(otherRecButtons(int)));
    connect(&otherPlayGroup,SIGNAL(buttonClicked(int)),this,SLOT(otherPlayButtons(int)));

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
    call_rec_edit->setValidator(upperValidate);
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
    message.clear();
    scriptProcess=nullptr;
    playMessageRig=0;
    recording=false;
    playing=false;
    if (!scriptProcess) scriptProcess=new QProcess();
    scriptProcess->setWorkingDirectory(userDirectory()+"/wav");
    adjustSize();
    setFixedSize(size());
}

void SSBMessageDialog::playButtons(int id)
{
    emit(sendMsg(funcEditPtr[id]->text().toLatin1(),false));
}

void SSBMessageDialog::playExcButtons(int id)
{
    emit(sendMsg(excFuncEditPtr[id]->text().toLatin1(),false));
}

void SSBMessageDialog::recButtons(int id)
{
    emit(sendMsg(funcRecEditPtr[id]->text().toLatin1(),false));
}

void SSBMessageDialog::excRecButtons(int id)
{
    emit(sendMsg(excFuncRecEditPtr[id]->text().toLatin1(),false));
}

void SSBMessageDialog::otherPlayButtons(int id)
{
    switch (id) {
    case 0:
        emit(sendMsg(qsl_msg_edit->text().toLatin1(),false));
        break;
    case 1:
        emit(sendMsg(quick_qsl_edit->text().toLatin1(),false));
        break;
    case 2:
        emit(sendMsg(dupe_msg_edit->text().toLatin1(),false));
        break;
    case 3:
        emit(sendMsg(qsl_updated_edit->text().toLatin1(),false));
        break;
    case 4:
        emit(sendMsg(cq_exc_edit->text().toLatin1(),false));
        break;
    case 5:
        emit(sendMsg(sp_exc_edit->text().toLatin1(),false));
        break;
    case 6:
        emit(sendMsg(call_edit->text().toLatin1(),false));
    }
}

void SSBMessageDialog::otherRecButtons(int id)
{
    switch (id) {
    case 0:
        emit(sendMsg(qsl_msg_rec_edit->text().toLatin1(),false));
        break;
    case 1:
        emit(sendMsg(quick_qsl_rec_edit->text().toLatin1(),false));
        break;
    case 2:
        emit(sendMsg(dupe_msg_rec_edit->text().toLatin1(),false));
        break;
    case 3:
        emit(sendMsg(qsl_updated_rec_edit->text().toLatin1(),false));
        break;
    case 4:
        emit(sendMsg(cq_exc_rec_edit->text().toLatin1(),false));
        break;
    case 5:
        emit(sendMsg(sp_exc_rec_edit->text().toLatin1(),false));
        break;
    case 6:
        emit(sendMsg(call_rec_edit->text().toLatin1(),false));
    }
}


/*!
 * \brief SSBMessageDialog::initialize Initialize dialog from settings file
 * \param s  contest csettings file
  */
void SSBMessageDialog::initialize(QSettings *cs,QSettings *s)
{
    csettings=cs;
    settings=s;

    // function keys: cq
    int sz=csettings->beginReadArray(c_cq_func[m]);
    for (int i=0;i<sz;i++) {
        csettings->setArrayIndex(i);
        funcEditPtr[i]->setText(csettings->value("func",c_cq_func_def[m]).toString());
        funcEditPtr[i]->setCursorPosition(0);
        cqF[i]=funcEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    // function keys: cq, record
    sz=csettings->beginReadArray(c_cq_rec_func);
    for (int i=0;i<sz;i++) {
        csettings->setArrayIndex(i);
        funcRecEditPtr[i]->setText(csettings->value("func",c_cq_rec_func_def).toString());
        funcRecEditPtr[i]->setCursorPosition(0);
        cqRecF[i]=funcRecEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    // function keys: exchange
    sz=csettings->beginReadArray(c_ex_func[m]);
    for (int i=0;i<sz;i++) {
        csettings->setArrayIndex(i);
        excFuncEditPtr[i]->setText(csettings->value("func",c_ex_func_def[m]).toString());
        excFuncEditPtr[i]->setCursorPosition(0);
        excF[i]=excFuncEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    // function keys: ex, record
    sz=csettings->beginReadArray(c_ex_rec_func);
    for (int i=0;i<sz;i++) {
        csettings->setArrayIndex(i);
        excFuncRecEditPtr[i]->setText(csettings->value("func",c_ex_rec_func_def).toString());
        excFuncRecEditPtr[i]->setCursorPosition(0);
        excRecF[i]=excFuncRecEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    // other special messages
    call_edit->setText(csettings->value(c_call_msg,c_call_msg_def).toString());
    call_edit->setCursorPosition(0);
    call_rec_edit->setText(csettings->value(c_call_msg_rec,c_call_msg_rec_def).toString());
    call_rec_edit->setCursorPosition(0);

    sp_exc_edit->setText(csettings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    sp_exc_edit->setCursorPosition(0);
    sp_exc_rec_edit->setText(csettings->value(c_sp_exc_rec,c_sp_exc_rec_def).toString());
    sp_exc_rec_edit->setCursorPosition(0);

    cq_exc_edit->setText(csettings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    cq_exc_edit->setCursorPosition(0);
    cq_exc_rec_edit->setText(csettings->value(c_cq_exc_rec,c_cq_exc_rec_def).toString());
    cq_exc_rec_edit->setCursorPosition(0);

    qsl_msg_edit->setText(csettings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_msg_edit->setCursorPosition(0);
    qsl_msg_rec_edit->setText(csettings->value(c_qsl_msg_rec,c_qsl_msg_rec_def).toString());
    qsl_msg_rec_edit->setCursorPosition(0);

    qsl_updated_edit->setText(csettings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    qsl_updated_edit->setCursorPosition(0);
    qsl_updated_rec_edit->setText(csettings->value(c_qsl_msg_updated_rec,c_qsl_msg_updated_rec_def).toString());
    qsl_updated_rec_edit->setCursorPosition(0);

    dupe_msg_edit->setText(csettings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    dupe_msg_edit->setCursorPosition(0);
    dupe_msg_rec_edit->setText(csettings->value(c_dupe_msg_rec,c_dupe_msg_rec_def).toString());
    dupe_msg_rec_edit->setCursorPosition(0);

    quick_qsl_edit->setText(csettings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
    quick_qsl_edit->setCursorPosition(0);
    quick_qsl_rec_edit->setText(csettings->value(c_qqsl_msg_rec,c_qqsl_msg_rec_def).toString());
    quick_qsl_rec_edit->setCursorPosition(0);

    cancel_edit->setText(csettings->value(c_ssb_cancel,c_ssb_cancel_def).toString());
    cancel_edit->setCursorPosition(0);

    // audio script csettings
    // PLAY csettings
    beforePlayLineEdit1->setText(csettings->value(s_before_play[0],s_before_play_def).toString());
    beforePlayLineEdit1->setCursorPosition(0);
    beforePlayLineEdit2->setText(csettings->value(s_before_play[1],s_before_play_def).toString());
    beforePlayLineEdit2->setCursorPosition(0);
    afterPlayLineEdit1->setText(csettings->value(s_after_play[0],s_after_play_def).toString());
    afterPlayLineEdit1->setCursorPosition(0);
    afterPlayLineEdit2->setText(csettings->value(s_after_play[1],s_after_play_def).toString());
    afterPlayLineEdit2->setCursorPosition(0);
    playLineEdit1->setText(csettings->value(s_play_command[0],s_play_command_def).toString());
    playLineEdit1->setCursorPosition(0);
    playLineEdit2->setText(csettings->value(s_play_command[1],s_play_command_def).toString());
    playLineEdit2->setCursorPosition(0);
    switchRadioLineEdit1->setText(csettings->value(s_switch_command[0],s_switch_command_def).toString());
    switchRadioLineEdit1->setCursorPosition(0);
    switchRadioLineEdit2->setText(csettings->value(s_switch_command[1],s_switch_command_def).toString());
    switchRadioLineEdit2->setCursorPosition(0);

    // REC csettings
    beforeRecLineEdit->setText(csettings->value(s_before_rec,s_before_rec_def).toString());
    beforeRecLineEdit->setCursorPosition(0);
    afterRecLineEdit->setText(csettings->value(s_after_rec,s_after_rec_def).toString());
    afterRecLineEdit->setCursorPosition(0);
    recLineEdit->setText(csettings->value(s_rec_command,s_rec_command_def).toString());
    recLineEdit->setCursorPosition(0);
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
    call_edit->setText(csettings->value(c_call_msg,c_call_msg_def).toString());
    call_edit->setCursorPosition(0);
    call_rec_edit->setText(csettings->value(c_call_msg_rec,c_call_msg_rec_def).toString());
    call_rec_edit->setCursorPosition(0);
    cancel_edit->setText(csettings->value(c_ssb_cancel,c_ssb_cancel_def).toString());
    cancel_edit->setCursorPosition(0);
    qsl_msg_edit->setText(csettings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_msg_edit->setCursorPosition(0);
    qsl_msg_rec_edit->setText(csettings->value(c_qsl_msg_rec,c_qsl_msg_rec_def).toString());
    qsl_msg_rec_edit->setCursorPosition(0);
    qsl_updated_edit->setText(csettings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    qsl_updated_edit->setCursorPosition(0);
    qsl_updated_rec_edit->setText(csettings->value(c_qsl_msg_updated_rec,c_qsl_msg_updated_rec_def).toString());
    qsl_updated_rec_edit->setCursorPosition(0);
    cq_exc_edit->setText(csettings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    cq_exc_edit->setCursorPosition(0);
    cq_exc_rec_edit->setText(csettings->value(c_cq_exc_rec,c_cq_exc_rec_def).toString());
    cq_exc_rec_edit->setCursorPosition(0);
    sp_exc_edit->setText(csettings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    sp_exc_edit->setCursorPosition(0);
    sp_exc_rec_edit->setText(csettings->value(c_sp_exc_rec,c_sp_exc_rec_def).toString());
    sp_exc_rec_edit->setCursorPosition(0);
    dupe_msg_edit->setText(csettings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    dupe_msg_edit->setCursorPosition(0);
    dupe_msg_rec_edit->setText(csettings->value(c_dupe_msg_rec,c_dupe_msg_rec_def).toString());
    dupe_msg_rec_edit->setCursorPosition(0);
    quick_qsl_edit->setText(csettings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
    quick_qsl_edit->setCursorPosition(0);
    quick_qsl_rec_edit->setText(csettings->value(c_qqsl_msg_rec,c_qqsl_msg_rec_def).toString());
    quick_qsl_rec_edit->setCursorPosition(0);
    beforePlayLineEdit1->setText(csettings->value(s_before_play[0],s_before_play_def).toString());
    beforePlayLineEdit1->setCursorPosition(0);
    beforePlayLineEdit2->setText(csettings->value(s_before_play[1],s_before_play_def).toString());
    beforePlayLineEdit2->setCursorPosition(0);
    afterPlayLineEdit1->setText(csettings->value(s_after_play[0],s_after_play_def).toString());
    afterPlayLineEdit1->setCursorPosition(0);
    afterPlayLineEdit2->setText(csettings->value(s_after_play[1],s_after_play_def).toString());
    afterPlayLineEdit2->setCursorPosition(0);
    playLineEdit1->setText(csettings->value(s_play_command[0],s_play_command_def).toString());
    playLineEdit1->setCursorPosition(0);
    playLineEdit2->setText(csettings->value(s_play_command[1],s_play_command_def).toString());
    playLineEdit2->setCursorPosition(0);
    beforeRecLineEdit->setText(csettings->value(s_before_rec,s_before_rec_def).toString());
    beforeRecLineEdit->setCursorPosition(0);
    afterRecLineEdit->setText(csettings->value(s_after_rec,s_after_rec_def).toString());
    afterRecLineEdit->setCursorPosition(0);
    recLineEdit->setText(csettings->value(s_rec_command,s_rec_command_def).toString());
    recLineEdit->setCursorPosition(0);
    switchRadioLineEdit1->setText(csettings->value(s_switch_command[0],s_switch_command_def).toString());
    switchRadioLineEdit2->setText(csettings->value(s_switch_command[1],s_switch_command_def).toString());
    reject();
}

SSBMessageDialog::~SSBMessageDialog()
{
    delete upperValidate;
    if (scriptProcess) {
        scriptProcess->close();
        delete scriptProcess;
    }
}

/*!
   Update messages from form
 */
void SSBMessageDialog::updateSSBMsg()
{
    // function keys: cq
    csettings->beginWriteArray(c_cq_func[m],N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        csettings->setArrayIndex(i);
        csettings->setValue("func",funcEditPtr[i]->text());
        cqF[i]=funcEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    // function keys: cq record
    csettings->beginWriteArray(c_cq_rec_func,N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        csettings->setArrayIndex(i);
        csettings->setValue("func",funcRecEditPtr[i]->text());
        cqRecF[i]=funcRecEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    // function keys: exchange
    csettings->beginWriteArray(c_ex_func[m],N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        csettings->setArrayIndex(i);
        csettings->setValue("func",excFuncEditPtr[i]->text());
        excF[i]=excFuncEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    // function keys: exc record
    csettings->beginWriteArray(c_ex_rec_func,N_FUNC);
    for (int i=0;i<N_FUNC;i++) {
        csettings->setArrayIndex(i);
        csettings->setValue("func",excFuncRecEditPtr[i]->text());
        excRecF[i]=excFuncRecEditPtr[i]->text().toLatin1();
    }
    csettings->endArray();

    csettings->setValue(c_call_msg,call_edit->text());
    csettings->setValue(c_call_msg_rec,call_rec_edit->text());
    csettings->setValue(c_ssb_cancel,cancel_edit->text());
    csettings->setValue(c_cq_exc[m],cq_exc_edit->text());
    csettings->setValue(c_cq_exc_rec,cq_exc_rec_edit->text());
    csettings->setValue(c_sp_exc[m],sp_exc_edit->text());
    csettings->setValue(c_sp_exc_rec,sp_exc_rec_edit->text());
    csettings->setValue(c_qsl_msg[m],qsl_msg_edit->text());
    csettings->setValue(c_qsl_msg_rec,qsl_msg_rec_edit->text());
    csettings->setValue(c_qsl_msg_updated[m],qsl_updated_edit->text());
    csettings->setValue(c_qsl_msg_updated_rec,qsl_updated_rec_edit->text());
    csettings->setValue(c_qqsl_msg[m],quick_qsl_edit->text());
    csettings->setValue(c_qqsl_msg_rec,quick_qsl_rec_edit->text());
    csettings->setValue(c_dupe_msg[m],dupe_msg_edit->text());
    csettings->setValue(c_dupe_msg_rec,dupe_msg_rec_edit->text());
    csettings->sync();
    csettings->setValue(s_play_command[0],playLineEdit1->text());
    csettings->setValue(s_rec_command,recLineEdit->text());
    csettings->setValue(s_play_command[1],playLineEdit2->text());
    csettings->setValue(s_before_play[0],beforePlayLineEdit1->text());
    csettings->setValue(s_after_play[0],afterPlayLineEdit1->text());
    csettings->setValue(s_before_play[1],beforePlayLineEdit2->text());
    csettings->setValue(s_after_play[1],afterPlayLineEdit2->text());
    csettings->setValue(s_before_rec,beforeRecLineEdit->text());
    csettings->setValue(s_after_rec,afterRecLineEdit->text());
    csettings->setValue(s_switch_command[0],switchRadioLineEdit1->text());
    csettings->setValue(s_switch_command[1],switchRadioLineEdit2->text());
    accept();
}

//
// Audio play and record functions. These launch external scripts that record and play audio
// messages.
//

/* Start script that is called when focus is changed to nrig
 *
 * this could be used for example to redirect mic audio to the correct rig
 */
void SSBMessageDialog::switchRadio(int nrig)
{
    playMessageRig=nrig;
    disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
    scriptProcess->close();
    scriptProcess->start(csettings->value(s_switch_command[nrig],s_switch_command_def).toString());
}

/* Begin playing an audio message. Intended to be called from the main program thread
 *
 * m is the filename of the audio file (without .wav extension) in ~/.so2sdr/wav
 *
 */
void SSBMessageDialog::playMessage(int nrig,QString m)
{
    message=m;
    playMessageRig=nrig;
    disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
    scriptProcess->close();
    connect(scriptProcess,SIGNAL(finished(int)),this,SLOT(playMessage2(int)));
    scriptProcess->start(csettings->value(s_before_play[nrig],s_before_play_def).toString());
}

void SSBMessageDialog::playMessage2(int signal)
{
    Q_UNUSED(signal)
    disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
    connect(scriptProcess,SIGNAL(finished(int)),this,SLOT(playMessage3(int)));

    emit(setPtt(playMessageRig,1));
    cmd=csettings->value(s_play_command[playMessageRig],s_play_command_def).toString();
    cmd=cmd.replace("$",message);
    scriptProcess->start(cmd);
    playing=true;
}

void SSBMessageDialog::playMessage3(int signal)
{
    Q_UNUSED(signal)

    playing=false;
    emit(setPtt(playMessageRig,0));
    disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
    scriptProcess->start(csettings->value(s_after_play[playMessageRig],s_after_play_def).toString());
    emit(finished());
}

void SSBMessageDialog::recMessage(QString m)
{
    if (!recording) {
        recording=true;
        message=m;
        emit(recordingStatus(true));
        disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
        scriptProcess->close();
        connect(scriptProcess,SIGNAL(finished(int)),this,SLOT(recMessage2(int)));
        scriptProcess->start(csettings->value(s_before_rec,s_before_rec_def).toString());
    } else {
        disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
        scriptProcess->close();
        recMessage3(0);
    }
}

void SSBMessageDialog::recMessage2(int signal)
{
    Q_UNUSED(signal)
    disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
    connect(scriptProcess,SIGNAL(finished(int)),this,SLOT(recMessage3(int)));
    QString t=csettings->value(s_rec_command,s_rec_command_def).toString();
    t=t.replace("$",message);
    scriptProcess->start(t);
}

void SSBMessageDialog::recMessage3(int signal)
{
    Q_UNUSED(signal)
    disconnect(scriptProcess,SIGNAL(finished(int)),nullptr,nullptr);
    scriptProcess->start(csettings->value(s_after_rec,s_after_rec_def).toString());
    emit(recordingStatus(false));
    recording=false;
}

/*! cancel a message (usually when ESC pressed). Kill the external script,
 * which moves to the "after play" message.
 * @todo need to handle case where this is called during the "before" or "after" scripts
 */
void SSBMessageDialog::cancelMessage()
{
    if (playing || recording) {
        scriptProcess->close();
    }
}
