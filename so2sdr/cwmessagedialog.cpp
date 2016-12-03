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
#include <QDialog>
#include <QFormLayout>
#include <QSettings>
#include "cwmessagedialog.h"
#include "defines.h"

CWMessageDialog::CWMessageDialog(ModeTypes modetype, QWidget *parent) : QDialog(parent)
{
    // m is an index for the mode: 0=CW, 1=SSB, 2=DIGI can all have different macros
    m=(int)modetype;
    if (m>1) m=0; // digi keys not implemented yet

    setupUi(this);
    upperValidate = new UpperValidator(this);
    connect(cwmessage_buttons, SIGNAL(rejected()), this, SLOT(rejectChanges()));
    connect(cwmessage_buttons, SIGNAL(accepted()), this, SLOT(updateCWMsg()));

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

/*!
 * \brief CWMessageDialog::initialize Initialize dialog from settings file
 * \param s  contest settings file
  */
void CWMessageDialog::initialize(QSettings *s)
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
    // function keys: exchange
    sz=settings->beginReadArray(c_ex_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        excFuncEditPtr[i]->setText(settings->value("func",c_ex_func_def[m]).toString());
        excFuncEditPtr[i]->setCursorPosition(0);
        excF[i]=excFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: shift
    sz=settings->beginReadArray(c_shift_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        shiftFuncEditPtr[i]->setText(settings->value("func",c_shift_func_def[m]).toString());
        shiftFuncEditPtr[i]->setCursorPosition(0);
        cqShiftF[i]=shiftFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // function keys: control
    sz=settings->beginReadArray(c_ctrl_func[m]);
    for (int i=0;i<sz;i++) {
        settings->setArrayIndex(i);
        ctrlFuncEditPtr[i]->setText(settings->value("func",c_ctrl_func_def[m]).toString());
        ctrlFuncEditPtr[i]->setCursorPosition(0);
        cqCtrlF[i]=ctrlFuncEditPtr[i]->text().toLatin1();
    }
    settings->endArray();
    // other special messages
    sp_exc_edit->setText(settings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    sp_exc_edit->setCursorPosition(0);
    cq_exc_edit->setText(settings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    cq_exc_edit->setCursorPosition(0);
    qsl_msg_edit->setText(settings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_msg_edit->setCursorPosition(0);
    qsl_updated_edit->setText(settings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    qsl_updated_edit->setCursorPosition(0);
    dupe_msg_edit->setText(settings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    dupe_msg_edit->setCursorPosition(0);
    quick_qsl_edit->setText(settings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
    quick_qsl_edit->setCursorPosition(0);
}


/*! slot called when cancel pressed: reset all line edits to
   stored values and close dialog */
void CWMessageDialog::rejectChanges()
{
    for (int i = 0; i < N_FUNC; i++) {
        funcEditPtr[i]->setText(cqF[i]);
        funcEditPtr[i]->setCursorPosition(0);
        ctrlFuncEditPtr[i]->setText(cqCtrlF[i]);
        ctrlFuncEditPtr[i]->setCursorPosition(0);
        shiftFuncEditPtr[i]->setText(cqShiftF[i]);
        shiftFuncEditPtr[i]->setCursorPosition(0);
        excFuncEditPtr[i]->setText(excF[i]);
        excFuncEditPtr[i]->setCursorPosition(0);
    }

    qsl_msg_edit->setText(settings->value(c_qsl_msg[m],c_qsl_msg_def[m]).toString());
    qsl_msg_edit->setCursorPosition(0);
    qsl_updated_edit->setText(settings->value(c_qsl_msg_updated[m],c_qsl_msg_updated_def[m]).toString());
    qsl_updated_edit->setCursorPosition(0);
    cq_exc_edit->setText(settings->value(c_cq_exc[m],c_cq_exc_def[m]).toString());
    cq_exc_edit->setCursorPosition(0);
    sp_exc_edit->setText(settings->value(c_sp_exc[m],c_sp_exc_def[m]).toString());
    sp_exc_edit->setCursorPosition(0);
    dupe_msg_edit->setText(settings->value(c_dupe_msg[m],c_dupe_msg_def[m]).toString());
    dupe_msg_edit->setCursorPosition(0);
    quick_qsl_edit->setText(settings->value(c_qqsl_msg[m],c_qqsl_msg_def[m]).toString());
    quick_qsl_edit->setCursorPosition(0);
    reject();
}

CWMessageDialog::~CWMessageDialog()
{
    delete upperValidate;
}

/*!
   Update messages from form
 */
void CWMessageDialog::updateCWMsg()
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
