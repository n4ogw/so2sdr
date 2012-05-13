/*! Copyright 2010-2011 R. Torsten Clay N4OGW

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
#include <QDir>
#include <QErrorMessage>
#include <QMessageBox>
#include <QStringList>
#include "so2sdr.h"

// ///// Read/write config files

/*!
   read station configuration file

 */
bool So2sdr::readConfig()
{
    const QByteArray token_names[] = { "winkey port",
                                       "my call",
                                       "grid",
                                       "cq zone",
                                       "itu zone",
                                       "winkey pot min",
                                       "winkey pot max",
                                       "use winkey pot",
                                       "wpm1",
                                       "winkey paddle",
                                       "winkey sidetone freq",
                                       "paddle swap",
                                       "rig1 port",
                                       "rig1 poll time",
                                       "rig1 baud",
                                       "rig2 port",
                                       "rig2 poll time",
                                       "rig2 baud",
                                       "my name",
                                       "my state",
                                       "my section",
                                       "cty file",
                                       "winkey ct space",
                                       "winkey paddle mode",
                                       "radio focus pin",
                                       "stereo pin",
                                       "contest directory",
                                       "wpm2",
                                       "rig1 type",
                                       "rig2 type",
                                       "rig1 audio device",
                                       "rig2 audio device",
                                       "rig1 offset",
                                       "rig2 offset",
                                       "telnet address",
                                       "spot timeout",
                                       "cabrillo name",
                                       "address",
                                       "cabrillo city",
                                       "cabrillo state",
                                       "cabrillo postal",
                                       "cabrillo country",
                                       "rig1 bits",
                                       "rig2 bits",
                                       "parallel_port",
                                       "rig1 audio api",
                                       "rig2 audio api",
                                       "rig1 correct iq",
                                       "rig2 correct iq",
                                       "rig1 iq data",
                                       "rig2 iq data",
                                       "rig1 click filter",
                                       "rig2 click filter",
                                       "rig1 mark signals",
                                       "rig2 mark signals",
                                       "rig1 poll tx",
                                       "rig2 poll tx",
                                       "radio focus pin invert",
                                       "find cq time",
                                       "data directory",
                                       "user directory",
                                       "enable bandmap 1",
                                       "enable bandmap 2",
                                       "cabrillo club" };
    const int        n_token_names = 64;

    QFile            file;//(stationConfigFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox *msg;
        msg = new QMessageBox(this);
        msg->setWindowTitle("Error");
        //msg->setText(stationConfigFile + " not found.");

        msg->exec();
        msg->deleteLater();

        return(false);
    }
   // station->CabrilloAddressEdit->clear();
    while (!file.atEnd()) {
        QByteArray buffer;
        buffer = file.readLine();
        buffer = buffer.trimmed();
        if (buffer.startsWith("#")) continue;  // comment lines
        int        j = buffer.indexOf('=');
        if (j == -1) continue;                 // no = sign, skip line
        QByteArray token = buffer.left(j);
        token = token.trimmed();               // remove initial/final whitespace
        token = token.toLower();
        QByteArray val = buffer.right(buffer.size() - j - 1);

        // check for quotes,
        int k, l;
        if ((k = val.indexOf('\"')) != -1) {
            if ((l = val.lastIndexOf('\"'))) {
                val = val.mid(k + 1, l - k - 1);
            }
        } else {
            // otherwise zap whitespace
            val = val.trimmed();
        }
        int p;
        for (int i = 0; i < n_token_names; i++) {
            if (token == token_names[i]) {
                switch (i) {
                case 0: // winkey port
                    //winkeyDialog->setDevice(val);
                    break;
                case 1: // my call
                    //station->setCall(val);
                    //setWindowTitle("SO2SDR:" + station->Call);
                    break;
                case 2: // grid
                    //station->setGrid(val);
                    break;
                case 3: // cq zone
                    //station->setCQZone(val);
                    break;
                case 4: // itu zone
                    //station->setITUZone(val);
                    break;
                case 5: // winkey pot min
                    //winkeyDialog->setPotMin(val.toInt());
                    break;
                case 6: // winkey pot max
                    //winkeyDialog->setPotMax(val.toInt());
                    break;
                case 7: // use winkey pot
                    //if (val.toInt()) winkeyDialog->setUsePot(true);
                    //else winkeyDialog->setUsePot(false);
                    break;
                case 8: // wpm1
                    //wpm[0] = val.toInt();
                    //wpmLineEditPtr[0]->setText(QString::number(wpm[0]));
                    break;
                case 9: // winkey paddle
                    //if (val.toInt()) winkeyDialog->setSidetonePaddle(true);
                    //else winkeyDialog->setSidetonePaddle(false);
                    break;
                case 10: // winkey sidetone freq
                    //winkeyDialog->setSidetone(val.toInt());
                    break;
                case 11: // paddle swap
                    //if (val.toInt()) winkeyDialog->setPaddleSwap(true);
                    //else winkeyDialog->setPaddleSwap(false);
                    break;
                case 12: // rig1 port
//                    radios->setDevice(0, val);
                    break;
                case 13: // rig1 poll time
  //                  radios->setPollTime(0, val.toInt());
    //                radios->Rig1PollEdit->setText(val);
                    break;
                case 14: // rig1 baud
      //              radios->setBaud(0, val.toInt());
                    break;
                case 15: // rig2 port
        //            radios->setDevice(1, val);
                    break;
                case 16: // rig2 poll time
          //          radios->setPollTime(1, val.toInt());
            //        radios->Rig2PollEdit->setText(val);
                    break;
                case 17: // rig2 baud
              //      radios->setBaud(1, val.toInt());
                    break;
                case 18: // my name
                    //station->setName(val);
                    break;
                case 19: // my state
                    //station->setState(val);
                    break;
                case 20: // my section
                    //station->setSection(val);
                    break;
                case 21: // cty file (default)
                    // option removed v0.82
                    break;
                case 22: // winky ct space
                    //if (val.toInt()) winkeyDialog->setCTSpace(true);
                    //else winkeyDialog->setCTSpace(false);
                    break;
                case 23: // winkey paddle mode
                    //if (val.toInt()) winkeyDialog->setPaddleMode(val.toInt());
                    break;
                case 24: // radio focus pin
                //    p = val.toInt();
                 //   radios->setRadioPin(p);
                  //  pport->setRadioPin(p);
                    break;
                case 25: // stereo pin
                  //  p = val.toInt();
                  //  radios->setStereoPin(p);
                  //  pport->setStereoPin(p);
                    break;
                case 26: // contest directory
                    contestDirectory = val;
                    break;
                case 27: // wpm2
                    //wpm[1] = val.toInt();
                    //wpmLineEditPtr[1]->setText(QString::number(wpm[1]));
                    break;
                case 28:  // rig1 type
                   // radios->setModel(0, val.toInt());
                    break;
                case 29:  // rig2 type
                   // radios->setModel(1, val.toInt());
                    break;
                case 30: // rig1 audio device
                   // sdr->setDevice(0, val.toInt());
                    break;
                case 31: // rig2 audio device
                   // sdr->setDevice(1, val.toInt());
                    break;
                case 32: // rig1 offset
                   // sdr->setOffset(0, val.toInt());
                    break;
                case 33: // rig2 offset
                   // sdr->setOffset(1, val.toInt());
                    break;
                case 34: // telnet address
                    val = val.trimmed();
                    telnetAddress.append(val);
                    break;
                case 35: // spot timeout
                   // sdr->setSpotTimeout(val.toInt());
                    break;
                case 36: // cabrilloname
                    //val = val.simplified();
                    //station->setCabName(val);
                    break;
                case 37: // address
                    //val = val.simplified();
                    //station->setCabAddress(val);
                    break;
                case 38: // cabrillocity
                    //station->setCabCity(val);
                    break;
                case 39: // cabrillostate
                    //station->setCabState(val);
                    break;
                case 40:  // cabrillopostal
                    //station->setCabZip(val);
                    break;
                case 41: // cabrillocountry
                    //station->setCabCountry(val);
                    break;
                case 42: // rig 1 bits
                   /*
                    switch (val.toInt()) {
                    case 16: sdr->setBits(0, 0); break;
                    case 24: sdr->setBits(0, 1); break;
                    case 32: sdr->setBits(0, 2); break;
                    }*/
                    break;
                case 43: // rig 2 bits
                    /*
                    switch (val.toInt()) {
                    case 16: sdr->setBits(1, 0); break;
                    case 24: sdr->setBits(1, 1); break;
                    case 32: sdr->setBits(1, 2); break;
                    }*/
                    break;
                case 44: // parallel_port
                    //radios->setParallelPortBox(val);
                    break;
                case 45: // rig1 audio api
                    //sdr->setAPI(0, val.toInt());
                    break;
                case 46: // rig2 audio api
                    //sdr->setAPI(1, val.toInt());
                    break;
                case 47: // rig1 correct iq balance
                    //iqCorrect[0] = val.toInt();
                    break;
                case 48: // rig2 correct iq balance
                    //iqCorrect[1] = val.toInt();
                    break;
                case 49: // rig1 collect IQ data
                    //iqData[0] = val.toInt();
                    break;
                case 50: // rig2 collect IQ data
                    //iqData[1] = val.toInt();
                    break;
                case 51: // rig1 click filter
                    //clickFilter[0] = val.toInt();
                    break;
                case 52: // rig2 click filter
                    //clickFilter[1] = val.toInt();
                    break;
                case 53: // rig1 mark signals
                    //mark[0] = val.toInt();
                    break;
                case 54: // rig2 mark signals
                    //mark[1] = val.toInt();
                    break;
                case 55: // rig1 poll tx
                    // depreciated option: now handled via winkey
                    break;
                case 56: // rig2 poll tx
                    // depreciated option: now handled via winkey
                    break;
                case 57: // radio focus pin invert
                //    radios->setRadioInvert(val.toInt());
                  //  pport->setRadioPinInvert(val.toInt());
                    break;
                case 58: // find cq integration time (in seconds)
                   // sdr->lineEditIntegTime->setText(val);
                    break;
                case 59: // data directory
                    //dataDirectory = val;
                    break;
                case 60: // user directory
                //    userDirectory = val;
                    break;
                case 61: // enable bandmap 1
                   /* if (val.toInt()) {
                        sdr->setEnabled(0, true);
                    } else {
                        sdr->setEnabled(0, false);
                    }*/
                    break;
                case 62: // enable bandmap 2
                   /* if (val.toInt()) {
                        sdr->setEnabled(1, true);
                    } else {
                        sdr->setEnabled(1, false);
                    }*/
                    break;
                case 63: // cabrillo club
              //      cabrillo->lineEditClub->setText(val);
                    break;
                }
            }
        }
    }
    file.close();
    return(true);
}

bool So2sdr::readContestConfig()
{
    const QByteArray token_names[] = { "cq memory f1",
                                       "cq memory f2",
                                       "cq memory f3",
                                       "cq memory f4",
                                       "cq memory f5",
                                       "cq memory f6",
                                       "cq memory f7",
                                       "cq memory f8",
                                       "cq memory f9",
                                       "cq memory f10",
                                       "cq memory f11",
                                       "cq memory f12",
                                       "my call",
                                       "cq exchange",
                                       "qsl message",
                                       "dupe message",
                                       "quick qsl message",
                                       "cq memory ctrl f1",
                                       "cq memory ctrl f2",
                                       "cq memory ctrl f3",
                                       "cq memory ctrl f4",
                                       "cq memory ctrl f5",
                                       "cq memory ctrl f6",
                                       "cq memory ctrl f7",
                                       "cq memory ctrl f8",
                                       "cq memory ctrl f9",
                                       "cq memory ctrl f10",
                                       "cq memory ctrl f11",
                                       "cq memory ctrl f12",
                                       "cq memory shift f1",
                                       "cq memory shift f2",
                                       "cq memory shift f3",
                                       "cq memory shift f4",
                                       "cq memory shift f5",
                                       "cq memory shift f6",
                                       "cq memory shift f7",
                                       "cq memory shift f8",
                                       "cq memory shift f9",
                                       "cq memory shift f10",
                                       "cq memory shift f11",
                                       "cq memory shift f12",
                                       "exc memory f1",
                                       "exc memory f2",
                                       "exc memory f3",
                                       "exc memory f4",
                                       "exc memory f5",
                                       "exc memory f6",
                                       "exc memory f7",
                                       "exc memory f8",
                                       "exc memory f9",
                                       "exc memory f10",
                                       "exc memory f11",
                                       "exc memory f12",
                                       "sp exchange",
                                       "my state",
                                       "my name",
                                       "my section",
                                       "contest",
                                       "sprint mode",
                                       "band mults",
                                       "qsl message changed",
                                       "mult file 1",
                                       "contest name",
                                       "master filename",
                                       "use master",
                                       "nmulttypes",
                                       "mult file 2",
                                       "qso type 1",
                                       "qso type 2",
                                       "zone type",
                                       "exclude mults 1",
                                       "show mults",
                                       "mult name 1",
                                       "mult name 2",
                                       "bandmap geometry 1",
                                       "bandmap geometry 2",
                                       "geometry w",
                                       "geometry h",
                                       "spot calls",
                                       "cabrillo1",
                                       "cabrillo2",
                                       "cabrillo3",
                                       "cabrillo4",
                                       "cabrillo5",
                                       "cabrillo6",
                                       "cabrillo7",
                                       "cty file",
                                       "cq limits 160",
                                       "cq limits 80",
                                       "cq limits 40",
                                       "cq limits 20",
                                       "cq limits 15",
                                       "cq limits 10",
                                       "sent exchange",
                                       "work dupes",
                                       "cabrillo name",
                                       "exclude mults 2",
                                       "cabrillo version",
                                       "geometry",
                                       "dupesheet geometry 1",
                                       "dupesheet geometry 2",
                                       "cabrillo location"};
    const int        n_token_names = 102;
    int              i, j, i1;
    unsigned int     f1, f2;
    bool             ok1, ok2;

    QFile            file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorBox->showMessage("Can't open contest config file <" + fileName + ">");
        return(false);
    }
/*
    cabrillo->comboBoxCabrillo1->clear();
    cabrillo->comboBoxCabrillo1->setEnabled(false);
    cabrillo->comboBoxCabrillo2->clear();
    cabrillo->comboBoxCabrillo2->setEnabled(false);
    cabrillo->comboBoxCabrillo3->clear();
    cabrillo->comboBoxCabrillo3->setEnabled(false);
    cabrillo->comboBoxCabrillo4->clear();
    cabrillo->comboBoxCabrillo4->setEnabled(false);
    cabrillo->comboBoxCabrillo5->clear();
    cabrillo->comboBoxCabrillo5->setEnabled(false);
    cabrillo->comboBoxCabrillo6->clear();
    cabrillo->comboBoxCabrillo6->setEnabled(false);
    cabrillo->comboBoxCabrillo7->clear();
    cabrillo->comboBoxCabrillo7->setEnabled(false);
*/
    while (!file.atEnd()) {
        QByteArray buffer;
        buffer = file.readLine();
        buffer = buffer.trimmed();
        if (buffer.startsWith("#")) continue;  // comment lines

        j = buffer.indexOf('=');
        if (j == -1) continue;  // no = sign, skip line

        QByteArray token = buffer.left(j);
        token = token.trimmed(); // remove initial/final whitespace
        token = token.toLower();
        QByteArray val = buffer.right(buffer.size() - j - 1);

        // check for quotes,
        if ((i = val.indexOf('\"')) != -1) {
            if ((j = val.lastIndexOf('\"'))) {
                val = val.mid(i + 1, j - i - 1);
            }
        } else {
            // otherwise zap whitespace
            val = val.trimmed();
        }
        QByteArray valu = val.toUpper();
        for (int i = 0; i < n_token_names; i++) {
            if (token == token_names[i]) {
                switch (i) {
                case 0:
                //    cwMessage->setFunc(0, valu);
                    break;
                case 1:
                  //  cwMessage->setFunc(1, valu);
                    break;
                case 2:
                    //cwMessage->setFunc(2, valu);
                    break;
                case 3:
                  //  cwMessage->setFunc(3, valu);
                    break;
                case 4:
                   // cwMessage->setFunc(4, valu);
                    break;
                case 5:
                   // cwMessage->setFunc(5, valu);
                    break;
                case 6:
                   // cwMessage->setFunc(6, valu);
                    break;
                case 7:
                   // cwMessage->setFunc(7, valu);
                    break;
                case 8:
                   // cwMessage->setFunc(8, valu);
                    break;
                case 9:
                    //cwMessage->setFunc(9, valu);
                    break;
                case 10:
                   // cwMessage->setFunc(10, valu);
                    break;
                case 11:
                    //cwMessage->setFunc(11, valu);
                    break;
                case 12:
                 //   station->setCall(valu);
                    break;
                case 13:
                  //  cwMessage->cqExc = valu;
                   // cwMessage->cq_exc_edit->setText(cwMessage->cqExc);
                    break;
                case 14:
                   // cwMessage->qslMsg = valu;
                   // cwMessage->qsl_msg_edit->setText(cwMessage->qslMsg);
                    break;
                case 15:
                   // cwMessage->dupeMsg = valu;
                   // cwMessage->dupe_msg_edit->setText(cwMessage->dupeMsg);
                    break;
                case 16:
                   // cwMessage->qqslMsg = valu;
                   // cwMessage->quick_qsl_edit->setText(cwMessage->qqslMsg);
                    break;
                case 17:
                   // cwMessage->setCtrlFunc(0, valu);
                    break;
                case 18:
                   // cwMessage->setCtrlFunc(1, valu);
                    break;
                case 19:
                   // cwMessage->setCtrlFunc(2, valu);
                    break;
                case 20:
                    //cwMessage->setCtrlFunc(3, valu);
                    break;
                case 21:
                    //cwMessage->setCtrlFunc(4, valu);
                    break;
                case 22:
                    //cwMessage->setCtrlFunc(5, valu);
                    break;
                case 23:
                    //cwMessage->setCtrlFunc(6, valu);
                    break;
                case 24:
                    //cwMessage->setCtrlFunc(7, valu);
                    break;
                case 25:
                    //cwMessage->setCtrlFunc(8, valu);
                    break;
                case 26:
                    //cwMessage->setCtrlFunc(9, valu);
                    break;
                case 27:
                    //cwMessage->setCtrlFunc(10, valu);
                    break;
                case 28:
                    //cwMessage->setCtrlFunc(11, valu);
                    break;
                case 29:
                   // cwMessage->setShiftFunc(0, valu);
                    break;
                case 30:
                   // cwMessage->setShiftFunc(1, valu);
                    break;
                case 31:
                   // cwMessage->setShiftFunc(2, valu);
                    break;
                case 32:
                    //cwMessage->setShiftFunc(3, valu);
                    break;
                case 33:
                    //cwMessage->setShiftFunc(4, valu);
                    break;
                case 34:
                   // cwMessage->setShiftFunc(5, valu);
                    break;
                case 35:
                   // cwMessage->setShiftFunc(6, valu);
                    break;
                case 36:
                   // cwMessage->setShiftFunc(7, valu);
                    break;
                case 37:
                  //  cwMessage->setShiftFunc(8, valu);
                    break;
                case 38:
                  //  cwMessage->setShiftFunc(9, valu);
                    break;
                case 39:
                  //  cwMessage->setShiftFunc(10, valu);
                    break;
                case 40:
                  //  cwMessage->setShiftFunc(11, valu);
                    break;
                case 41:
                   // cwMessage->setExcFunc(0, valu);
                    break;
                case 42:
                  //  cwMessage->setExcFunc(1, valu);
                    break;
                case 43:
                   // cwMessage->setExcFunc(2, valu);
                    break;
                case 44:
                   // cwMessage->setExcFunc(3, valu);
                    break;
                case 45:
                   // cwMessage->setExcFunc(4, valu);
                    break;
                case 46:
                   // cwMessage->setExcFunc(5, valu);
                    break;
                case 47:
                   // cwMessage->setExcFunc(6, valu);
                    break;
                case 48:
                  //  cwMessage->setExcFunc(7, valu);
                    break;
                case 49:
                   // cwMessage->setExcFunc(8, valu);
                    break;
                case 50:
                   // cwMessage->setExcFunc(9, valu);
                    break;
                case 51:
                   // cwMessage->setExcFunc(10, valu);
                    break;
                case 52:
                  //  cwMessage->setExcFunc(11, valu);
                    break;
                case 53:
                //    cwMessage->spExc = valu;
                  //  cwMessage->sp_exc_edit->setText(cwMessage->spExc);
                    break;
                case 54:
                  //  station->setState(valu);
                    break;
                case 55:
                   // station->setName(valu);
                    break;
                case 56:
                  //  station->setSection(valu);
                    break;
                case 57:
                    //selectContest(valu);
                    //contest->setContestName(valu);
                    break;
                case 58:
                    if (valu.toInt()) {
                        //options->setSprintMode(true);
                    } else {
                        //options->setSprintMode(false);
                    }
                    break;
                case 59:
                    if (valu.toInt()) {
                        //contest->setmultsByBand(true);
                        //options->setmultsByBand(true);
                    } else {
                       // contest->setmultsByBand(false);
                        //options->setmultsByBand(false);
                    }
                    break;
                case 60:
                   // cwMessage->qslMsgChanged = valu;
                   // cwMessage->qsl_updated_edit->setText(valu);
                    break;
                case 61:
                  //  multFile[0] = val;
                    break;
                case 62:
                    //contestNameDisplayed = val;
                    break;
                case 63:
                    //options->setMasterFile(val);
                    break;
                case 64:
                    if (val.toInt()) {
                      //  options->setMasterMode(true);
                    } else {
                       // options->setMasterMode(false);
                    }
                    break;
                case 65:
                  //  contest->setNMultTypes(val.toInt());
                    break;
                case 66:
                 //   multFile[1] = val;
                    break;
                case 67:
                //    contest->addQsoType(val,0);
                    break;
                case 68:
                 //   contest->addQsoType(val,1);
                    break;
                case 69:
//                    zoneType = val.toInt();
                    break;
                case 70: // exclude mults 1
                    //excludeMultsFile[0] = val;
                    //readExcludeMults(0, val);
                    break;
                case 71:
                    if (val.toInt()) {
                       // options->setShowMults(true);
                    } else {
                        //options->setShowMults(false);
                    }
                    break;
                case 72:
                   // multNameLabel[0]->setText(val);
                    break;
                case 73:
                  //  multNameLabel[1]->setText(val);
                    break;
                case 74: // bandmap geometry 1
                   // bandmapGeometry[0] = val;
                   // bandmapGeometry[0].replace(255, '\n');
                    break;
                case 75: // bandmap geometry 2
                   // bandmapGeometry[1] = val;
                   // bandmapGeometry[1].replace(255, '\n');
                    break;
                case 76:
                    break;
                case 77:
                    break;
                case 78:
                   /* if (val.toInt()) {
                        sdr->setSpotCalls(true);
                    } else {
                        sdr->setSpotCalls(false);
                    }*/
                    break;
                case 79: // cabrillo1
                    // fill in category name with first string
                    /*
                    if (!cabrillo->comboBoxCabrillo1->isEnabled()) {
                        cabrillo->labelCabrillo1->setText(val);
                        cabrillo->comboBoxCabrillo1->setEnabled(true);
                    } else {
                        cabrillo->comboBoxCabrillo1->addItem(val);
                    }
                    */
                    break;
                case 80: // cabrillo2
                    /*
                    if (!cabrillo->comboBoxCabrillo2->isEnabled()) {
                        cabrillo->labelCabrillo2->setText(val);
                        cabrillo->comboBoxCabrillo2->setEnabled(true);
                    } else {
                        cabrillo->comboBoxCabrillo2->addItem(val);
                    }*/
                    break;
                case 81: // cabrillo3
                    // fill in category name
                    /*
                    if (!cabrillo->comboBoxCabrillo3->isEnabled()) {
                        cabrillo->labelCabrillo3->setText(val);
                        cabrillo->comboBoxCabrillo3->setEnabled(true);
                    } else {
                        cabrillo->comboBoxCabrillo3->addItem(val);
                    }*/
                    break;
                case 82: // cabrillo4
                    // fill in category name
                    /*
                    if (!cabrillo->comboBoxCabrillo4->isEnabled()) {
                        cabrillo->labelCabrillo4->setText(val);
                        cabrillo->comboBoxCabrillo4->setEnabled(true);
                    } else {
                        cabrillo->comboBoxCabrillo4->addItem(val);
                    }*/
                    break;
                case 83: // cabrillo5
                    // fill in category name
                    /*
                    if (!cabrillo->comboBoxCabrillo5->isEnabled()) {
                        cabrillo->labelCabrillo5->setText(val);
                        cabrillo->comboBoxCabrillo5->setEnabled(true);
                    } else {
                        cabrillo->comboBoxCabrillo5->addItem(val);
                    }*/
                    break;
                case 84: // cabrillo6
                    // fill in category name
                    /*
                    if (!cabrillo->comboBoxCabrillo6->isEnabled()) {
                        cabrillo->labelCabrillo6->setText(val);
                        cabrillo->comboBoxCabrillo6->setEnabled(true);
                    } else {
                        cabrillo->comboBoxCabrillo6->addItem(val);
                    }*/
                    break;
                case 85: // cabrillo7
                    // fill in category name
                    /*
                    if (!cabrillo->comboBoxCabrillo7->isEnabled()) {
                        cabrillo->labelCabrillo7->setText(val);
                        cabrillo->comboBoxCabrillo7->setEnabled(true);
                    } else {
                        cabrillo->comboBoxCabrillo7->addItem(val);
                    }*/
                    break;
                case 86: // cty file
                    //options->lineEditCtyFile->setText(val);
                    break;
                case 87: // cq limits 160
                    /*
                    i1 = val.indexOf(",");
                    f1 = val.mid(0, i1).toUInt(&ok1, 10);
                    f2 = val.mid(i1 + 1, val.length() - i1 - 1).toUInt(&ok2, 10);
                    if (ok1 && ok2) {
                        cqLimits[0][0] = f1;
                        cqLimits[0][1] = f2;
                    }*/
                    break;
                case 88: // cq limits 80
                    /*
                    i1 = val.indexOf(",");
                    f1 = val.mid(0, i1).toUInt(&ok1, 10);
                    f2 = val.mid(i1 + 1, val.length() - i1 - 1).toUInt(&ok2, 10);
                    if (ok1 && ok2) {
                        cqLimits[1][0] = f1;
                        cqLimits[1][1] = f2;
                    }*/
                    break;
                case 89: // cq limits 40
                    /*
                    i1 = val.indexOf(",");
                    f1 = val.mid(0, i1).toUInt(&ok1, 10);
                    f2 = val.mid(i1 + 1, val.length() - i1 - 1).toUInt(&ok2, 10);
                    if (ok1 && ok2) {
                        cqLimits[2][0] = f1;
                        cqLimits[2][1] = f2;
                    }*/
                    break;
                case 90: // cq limits 20
                    /*
                    i1 = val.indexOf(",");
                    f1 = val.mid(0, i1).toUInt(&ok1, 10);
                    f2 = val.mid(i1 + 1, val.length() - i1 - 1).toUInt(&ok2, 10);
                    if (ok1 && ok2) {
                        cqLimits[3][0] = f1;
                        cqLimits[3][1] = f2;
                    }*/
                    break;
                case 91: // cq limits 15
                    /*
                    i1 = val.indexOf(",");
                    f1 = val.mid(0, i1).toUInt(&ok1, 10);
                    f2 = val.mid(i1 + 1, val.length() - i1 - 1).toUInt(&ok2, 10);
                    if (ok1 && ok2) {
                        cqLimits[4][0] = f1;
                        cqLimits[4][1] = f2;
                    }*/
                    break;
                case 92: // cq limits 10
                    /*
                    i1 = val.indexOf(",");
                    f1 = val.mid(0, i1).toUInt(&ok1, 10);
                    f2 = val.mid(i1 + 1, val.length() - i1 - 1).toUInt(&ok2, 10);
                    if (ok1 && ok2) {
                        cqLimits[5][0] = f1;
                        cqLimits[5][1] = f2;
                    }*/
                    break;
                case 93: // sent exchange
                {
                    QList<QByteArray> tmplist = val.split(' ');
                    if (tmplist.size() > 0) {
                        //options->setSentExch(0, tmplist.at(0));
                    }
                    if (tmplist.size() > 1) {
                        //options->setSentExch(1, tmplist.at(1));
                    }
                    if (tmplist.size() > 2) {
                        //options->setSentExch(2, tmplist.at(2));
                    }
                    if (tmplist.size() > 3) {
                        //options->setSentExch(3, tmplist.at(3));
                    }
                }
                break;
                case 94: // work dupes
                    //options->setDupeMode(val.toInt());
                    break;
                case 95: // cabrillo name
                   // cabrillo->labelContest->setText(val);
                    break;
                case 96: // exclude mults 2
                    //excludeMultsFile[1] = val;
                    //readExcludeMults(1, val);
                    break;
                case 97: // cabrillo version
                    //cabrillo->cabrilloVersion = val;
                    break;
                case 98:
               //     mainGeometry = val;    // geometry
               //     restoreGeometry(val);
                    break;
                case 99: // dupesheet geometry 1
                   // dupesheetGeometry[0] = val;
                    break;
                case 100: // dupesheet geometry 2
                    //dupesheetGeometry[1] = val;
                    break;
                case 101: // cabrillo location
                 //   cabrillo->lineEditLocation->setText(val);
                    break;
                }
            }
        }
    }
    file.close();
    int     indx = fileName.lastIndexOf("/");
    QString tmp  = fileName.mid(indx + 1, fileName.size() - indx);
    if (contestNameDisplayed.isEmpty()) {
        contestNameDisplayed = contest->contestName();
    }
   // setWindowTitle(station->Call + " : " + tmp + " : " + contestNameDisplayed.toUpper());
  //  So2sdrStatusBar->showMessage("Read " + fileName, 3000);

  //  sdr->setCQLimit(cqLimits);

    return(true); // successful
}

/*!
   save station config to file
 */
bool So2sdr::saveDotfileConfig()
{
    directory->setCurrent(QDir::homePath());
#ifdef Q_OS_WIN
    directory->setCurrent("so2sdr");
#endif
#ifdef Q_OS_LINUX
    directory->setCurrent(".so2sdr");
#endif
    QFile file("so2sdr.cfg");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return(false);
    }
    file.write("#SO2SDR version " + Version + "\n");
    file.write("#\n#Files\n#\n");
    if (!contestDirectory.isEmpty()) {
        file.write("contest directory=\"" + contestDirectory.toAscii() + "\"\n");
    }
    if (!userDirectory.isEmpty()) {
        file.write("user directory=\"" + userDirectory.toAscii() + "\"\n");
    }

    file.write("#\n#Station information\n#\n");
  //  file.write("my call=" + station->Call.toAscii() + "\n");
  ////  file.write("grid=" + station->Grid.toAscii() + "\n");
 //   QString tmp = "cq zone=" + QString::number(station->CQZone) + "\n";
  //  file.write(tmp.toAscii());
  //  tmp = "itu zone=" + QString::number(station->ITUZone) + "\n";
 //   file.write(tmp.toAscii());
 //   file.write("my name=" + station->Name.toAscii() + "\n");
 //   file.write("my state=" + station->State.toAscii() + "\n");
  //  file.write("my section=" + station->Section.toAscii() + "\n");
/*
    file.write("#\n#Winkey setup\n#\n");
    file.write("winkey port=" + winkeyDialog->Device() + "\n");
    tmp = "winkey pot min=" + QString::number(winkeyDialog->params()->WinkeyPotMin) + "\n";
    file.write(tmp.toAscii());
    tmp = "winkey pot max=" + QString::number(winkeyDialog->params()->WinkeyPotMax) + "\n";
    file.write(tmp.toAscii());
    tmp = "use winkey pot=" + QString::number(winkeyDialog->params()->WinkeyUsePot) + "\n";
    file.write(tmp.toAscii());
    tmp = "wpm1=" + QString::number(wpm[0]) + "\n";
    file.write(tmp.toAscii());
    tmp = "wpm2=" + QString::number(wpm[1]) + "\n";
    file.write(tmp.toAscii());
    if (winkeyDialog->params()->WinkeySidetonePaddle == true) {
        tmp = "winkey paddle=1\n";
    } else {
        tmp = "winkey paddle=0\n";
    }
    file.write(tmp.toAscii());
    tmp = "winkey paddle mode=" + QString::number(winkeyDialog->params()->PaddleMode) + "\n";
    file.write(tmp.toAscii());
    tmp = "winkey sidetone freq=" + QString::number(winkeyDialog->params()->WinkeySidetone) + "\n";
    file.write(tmp.toAscii());
    tmp = "paddle swap=" + QString::number(winkeyDialog->params()->WinkeyPaddleSwap) + "\n";
    file.write(tmp.toAscii());
    tmp = "winkey ct space=" + QString::number(winkeyDialog->params()->CTSpace) + "\n";
    file.write(tmp.toAscii());
*/
    /*
    file.write("#\n#Radio configuration\n#\n");
    QString tmp = "rig1 type=" + QString::number(radios->model(0)) + "\n";
    file.write(tmp.toAscii());
    file.write("rig1 port=" + radios->device(0) + "\n");
    tmp = "rig1 poll time=" + QString::number(radios->params(0)->polltime) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig1 baud=" + QString::number(radios->params(0)->baud) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig2 type=" + QString::number(radios->model(1)) + "\n";
    file.write(tmp.toAscii());
    file.write("rig2 port=" + radios->device(1) + "\n");
    tmp = "rig2 poll time=" + QString::number(radios->params(1)->polltime) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig2 baud=" + QString::number(radios->params(1)->baud) + "\n";
    file.write(tmp.toAscii());

    file.write("#\n#Parallel port so2r switching\n#\n");
    tmp = "parallel_port=" + radios->parallelPort() + "\n";
    file.write(tmp.toAscii());
    tmp = "radio focus pin=" + QString::number(radios->radioPin()) + "\n";
    file.write(tmp.toAscii());
    tmp = "radio focus pin invert=" + QString::number(radios->radioInvert()) + "\n";
    file.write(tmp.toAscii());
    tmp = "stereo pin=" + QString::number(radios->stereoPin()) + "\n";
    file.write(tmp.toAscii());
*/
    /*
    file.write("#\n#Bandscope configuration\n#\n");
    tmp = "enable bandmap 1=";
    if (sdr->enabled(0)) tmp = tmp + "1\n";
    else tmp = tmp + "0\n";
    file.write(tmp.toAscii());
    tmp = "enable bandmap 2=";
    if (sdr->enabled(1)) tmp = tmp + "1\n";
    else tmp = tmp + "0\n";
    file.write(tmp.toAscii());
    tmp = "rig1 audio api=" + QString::number(sdr->selectedDeviceApi(0)) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig1 audio device=" + QString::number(sdr->selectedDeviceNum(0)) + "\n";
    file.write(tmp.toAscii());

    tmp = "rig1 bits=";
    switch (sdr->bits(0)) {
    case 0: tmp = tmp + "16\n"; break;
    case 1: tmp = tmp + "24\n"; break;
    case 2: tmp = tmp + "32\n"; break;
    }
    file.write(tmp.toAscii());
    tmp = "rig1 offset=" + QString::number(sdr->offset(0)) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig1 correct iq=" + QString::number(iqCorrect[0]) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig1 iq data=" + QString::number(iqData[0]) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig1 click filter=" + QString::number(clickFilter[0]) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig1 mark signals=" + QString::number(mark[0]) + "\n";
    file.write(tmp.toAscii());

    tmp = "rig2 audio api=" + QString::number(sdr->selectedDeviceApi(1)) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig2 audio device=" + QString::number(sdr->selectedDeviceNum(1)) + "\n";
    file.write(tmp.toAscii());

    tmp = "rig2 bits=";
    switch (sdr->bits(1)) {
    case 0: tmp = tmp + "16\n"; break;
    case 1: tmp = tmp + "24\n"; break;
    case 2: tmp = tmp + "32\n"; break;
    }
    file.write(tmp.toAscii());
    tmp = "rig2 offset=" + QString::number(sdr->offset(1)) + "\n";
    file.write(tmp.toAscii());

    tmp = "rig2 correct iq=" + QString::number(iqCorrect[1]) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig2 iq data=" + QString::number(iqData[1]) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig2 click filter=" + QString::number(clickFilter[1]) + "\n";
    file.write(tmp.toAscii());
    tmp = "rig2 mark signals=" + QString::number(mark[1]) + "\n";
    file.write(tmp.toAscii());
    tmp = "find cq time=" + sdr->lineEditIntegTime->text() + "\n";
    file.write(tmp.toAscii());

    file.write("#\n#DX cluster setup\n#\n");
    for (int i = 0; i < telnetAddress.size(); i++) {
        tmp = "telnet address=" + telnetAddress[i] + "\n";
        file.write(tmp.toAscii());
    }
    tmp = "spot timeout=" + QString::number(sdr->spotTimeOut()) + "\n";
    file.write(tmp.toAscii());
*/
    /*
    file.write("#\n#Cabrillo generation\n#\n");
    tmp = "cabrillo name=" + station->cabrilloName + "\n";
    file.write(tmp.toAscii());
    QString     str     = station->cabrilloAddress;
    QStringList strlist = str.split("\n");
    for (int i = 0; i < strlist.size(); i++) {
        tmp = "address=" + strlist.at(i) + "\n";
        file.write(tmp.toAscii());
    }
    tmp = "cabrillo city=" + station->cabrilloCity + "\n";
    file.write(tmp.toAscii());
    tmp = "cabrillo state=" + station->cabrilloState + "\n";
    file.write(tmp.toAscii());
    tmp = "cabrillo postal=" + station->cabrilloZip + "\n";
    file.write(tmp.toAscii());
    tmp = "cabrillo country=" + station->cabrilloCountry + "\n";
    file.write(tmp.toAscii());
    tmp = "cabrillo club=" + cabrillo->lineEditClub->text() + "\n";
    file.write(tmp.toAscii());
    */
    file.close();
    return(true);
}

/*!
   save current contest setting to file
 */
bool So2sdr::saveContestConfig()
{
    return true;

    directory->setCurrent(contestDirectory);
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return(false);
    }
    file.write("#SO2SDR version " + Version + "\n#\n");
    if (!contestNameDisplayed.isEmpty()) file.write("contest name=" + contestNameDisplayed + "\n");
    file.write("contest=" + contest->contestName() + "\n");

    file.write("\n#These will override settings in so2sdr.cfg\n");
    /*
    file.write("my call=" + station->Call.toAscii() + "\n");

    if (!station->Section.isEmpty()) {
        file.write("my section=" + station->Section.toAscii() + "\n");
    }
    if (!station->Name.isEmpty()) {
        file.write("my name=" + station->Name.toAscii() + "\n");
    }
    if (!station->State.isEmpty()) {
        file.write("my state=" + station->State.toAscii() + "\n");
    }
    file.write("cty file=" + options->lineEditCtyFile->text().toAscii() + "\n");

    file.write("\n#Zone type: 0=CQ 1=ITU\n");
    file.write("zone type=" + QByteArray::number(zoneType) + "\n");

    file.write("\n#Contest/program options\n");
    if (options->sprintMode()) {
        file.write("sprint mode=1\n");
    } else {
        file.write("sprint mode=0\n");
    }

    if (sdr->spotCalls()) {
        file.write("spot calls=1\n");
    } else {
        file.write("spot calls=0\n");
    }

    file.write("work dupes=" + QByteArray::number(options->dupeMode()) + "\n");
*/
    file.write("\n#Multiplier setup\n");
   // file.write("nmulttypes=" + QByteArray::number(contest->nMultTypes()) + "\n");
   // if (!multFile[0].isEmpty()) file.write("mult file 1=" + multFile[0] + "\n");
 //   if (!multNameLabel[0]->text().isEmpty()) file.write("mult name 1=" + multNameLabel[0]->text().toAscii() + "\n");
   // if (!multFile[1].isEmpty()) file.write("mult file 2=" + multFile[1] + "\n");
   // if (!multNameLabel[1]->text().isEmpty()) file.write("mult name 2=" + multNameLabel[1]->text().toAscii() + "\n");
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < contest->qsoType(j).size(); i++) {
            file.write("qso type " + QByteArray::number(j + 1) + "=" + contest->qsoType(j).at(i) + "\n");
        }
    }
    //file.write("sent exchange=" + options->sentExch(0) + " "
      //         + options->sentExch(1) + " " + options->sentExch(2) + " " + options->sentExch(3) + "\n");
/*
    if (contest->multsByBand()) {
        file.write("band mults=1\n");
    } else {
        file.write("band mults=0\n");
    }
    file.write("\n# files for mults excluded from screen list\n");
    for (int i = 0; i < MMAX; i++) {
        if (!excludeMultsFile[i].isEmpty()) {
            file.write("exclude mults " + QByteArray::number(i + 1) + "=" + excludeMultsFile[i].toAscii() + "\n\n");
        }
    }

    if (options->showMults()) {
        file.write("show mults=1\n");
    } else {
        file.write("show mults=0\n");
    }

    file.write("\n#CQ limits\n");
    file.write("cq limits 160=" + QByteArray::number(sdr->cqLimit(0, 0)) + "," + QByteArray::number(sdr->cqLimit(0, 1)) + "\n");
    file.write("cq limits 80=" + QByteArray::number(sdr->cqLimit(1, 0)) + "," + QByteArray::number(sdr->cqLimit(1, 1)) + "\n");
    file.write("cq limits 40=" + QByteArray::number(sdr->cqLimit(2, 0)) + "," + QByteArray::number(sdr->cqLimit(2, 1)) + "\n");
    file.write("cq limits 20=" + QByteArray::number(sdr->cqLimit(3, 0)) + "," + QByteArray::number(sdr->cqLimit(3, 1)) + "\n");
    file.write("cq limits 15=" + QByteArray::number(sdr->cqLimit(4, 0)) + "," + QByteArray::number(sdr->cqLimit(4, 1)) + "\n");
    file.write("cq limits 10=" + QByteArray::number(sdr->cqLimit(5, 0)) + "," + QByteArray::number(sdr->cqLimit(5, 1)) + "\n");

    file.write("\n#CQ memories\n");
    for (int i = 0; i < 12; i++) {
        if (!cwMessage->cqF[i].isEmpty()) {
            QString tmp = "cq memory f" + QString::number(i + 1) + "=" + cwMessage->cqF[i] + "\n";
            file.write(tmp.toAscii());
        }
    }
    file.write("\n#Exchange memories\n");
    for (int i = 0; i < 12; i++) {
        if (!cwMessage->excF[i].isEmpty()) {
            QString tmp = "exc memory f" + QString::number(i + 1) + "=" + cwMessage->excF[i] + "\n";
            file.write(tmp.toAscii());
        }
    }
    file.write("\n#Ctrl+F memories\n");
    for (int i = 0; i < 12; i++) {
        if (!cwMessage->cqCtrlF[i].isEmpty()) {
            QString tmp = "cq memory ctrl f" + QString::number(i + 1) + "=" + cwMessage->cqCtrlF[i] + "\n";
            file.write(tmp.toAscii());
        }
    }
    file.write("\n#Shift+F memories\n");
    for (int i = 0; i < 12; i++) {
        if (!cwMessage->cqShiftF[i].isEmpty()) {
            QString tmp = "cq memory shift f" + QString::number(i + 1) + "=" + cwMessage->cqShiftF[i] + "\n";
            file.write(tmp.toAscii());
        }
    }
    file.write("\n#other messages\n");
    if (!cwMessage->cqExc.isEmpty()) {
        file.write("cq exchange=" + cwMessage->cqExc + "\n");
    }
    if (!cwMessage->spExc.isEmpty()) {
        file.write("sp exchange=" + cwMessage->spExc + "\n");
    }
    if (!cwMessage->qslMsg.isEmpty()) {
        file.write("qsl message=" + cwMessage->qslMsg + "\n");
    }
    if (!cwMessage->qqslMsg.isEmpty()) {
        file.write("quick qsl message=" + cwMessage->qqslMsg + "\n");
    }
    if (!cwMessage->qslMsgChanged.isEmpty()) {
        file.write("qsl message changed=" + cwMessage->qslMsgChanged + "\n");
    }
    if (!cwMessage->dupeMsg.isEmpty()) {
        file.write("dupe message=" + cwMessage->dupeMsg + "\n");
    }

    file.write("\n#supercheck partial\n");
    if (!options->masterFile().isEmpty()) {
        QString tmp = "master filename=" + options->masterFile() + "\n";
        file.write(tmp.toAscii());
    }
    if (options->masterMode()) {
        QString tmp = "use master=1\n";
        file.write(tmp.toAscii());
    } else {
        QString tmp = "use master=0\n";
        file.write(tmp.toAscii());
    }

    file.write("\n#Window geometry\n");
    file.write("geometry=\"" + saveGeometry() + "\"\n");
    for (int i = 0; i < NRIG; i++) {
        if (bandmapOn[i]) {
            bandmapGeometry[i] = bandmap[i]->saveGeometry();
            bandmapGeometry[i].replace('\n', 255);
        }
        if (!bandmapGeometry[i].isEmpty()) {
            file.write("bandmap geometry " + QByteArray::number(i + 1) + "=\"" + bandmapGeometry[i] + "\"\n");
        }
    }

    for (int i = 0; i < 2; i++) {
        if (dupesheet[i]) {
            dupesheetGeometry[i] = dupesheet[i]->saveGeometry();
            if (!dupesheetGeometry[i].isEmpty()) {
                file.write("dupesheet geometry " + QByteArray::number(i + 1) + "=\"" + dupesheetGeometry[i] + "\"\n");
            }
        }
    }

    file.write("\n#Cabrillo categories\n");
    file.write("cabrillo version=" + cabrillo->cabrilloVersion + "\n");
    if (cabrillo->comboBoxCabrillo1->isEnabled()) {
        QString tmp;
        tmp = "cabrillo1=" + cabrillo->labelCabrillo1->text() + "\n";
        file.write(tmp.toAscii());
        for (int i = 0; i < cabrillo->comboBoxCabrillo1->count(); i++) {
            tmp = "cabrillo1=" + cabrillo->comboBoxCabrillo1->itemText(i) + "\n";
            file.write(tmp.toAscii());
        }
    }
    if (cabrillo->comboBoxCabrillo2->isEnabled()) {
        QString tmp;
        tmp = "cabrillo2=" + cabrillo->labelCabrillo2->text() + "\n";
        file.write(tmp.toAscii());
        for (int i = 0; i < cabrillo->comboBoxCabrillo2->count(); i++) {
            tmp = "cabrillo2=" + cabrillo->comboBoxCabrillo2->itemText(i) + "\n";
            file.write(tmp.toAscii());
        }
    }
    if (cabrillo->comboBoxCabrillo3->isEnabled()) {
        QString tmp;
        tmp = "cabrillo3=" + cabrillo->labelCabrillo3->text() + "\n";
        file.write(tmp.toAscii());
        for (int i = 0; i < cabrillo->comboBoxCabrillo3->count(); i++) {
            tmp = "cabrillo3=" + cabrillo->comboBoxCabrillo3->itemText(i) + "\n";
            file.write(tmp.toAscii());
        }
    }
    if (cabrillo->comboBoxCabrillo4->isEnabled()) {
        QString tmp;
        tmp = "cabrillo4=" + cabrillo->labelCabrillo4->text() + "\n";
        file.write(tmp.toAscii());
        for (int i = 0; i < cabrillo->comboBoxCabrillo4->count(); i++) {
            tmp = "cabrillo4=" + cabrillo->comboBoxCabrillo4->itemText(i) + "\n";
            file.write(tmp.toAscii());
        }
    }
    if (cabrillo->comboBoxCabrillo5->isEnabled()) {
        QString tmp;
        tmp = "cabrillo5=" + cabrillo->labelCabrillo5->text() + "\n";
        file.write(tmp.toAscii());
        for (int i = 0; i < cabrillo->comboBoxCabrillo5->count(); i++) {
            tmp = "cabrillo5=" + cabrillo->comboBoxCabrillo5->itemText(i) + "\n";
            file.write(tmp.toAscii());
        }
    }
    if (cabrillo->comboBoxCabrillo6->isEnabled()) {
        QString tmp;
        tmp = "cabrillo6=" + cabrillo->labelCabrillo6->text() + "\n";
        file.write(tmp.toAscii());
        for (int i = 0; i < cabrillo->comboBoxCabrillo6->count(); i++) {
            tmp = "cabrillo6=" + cabrillo->comboBoxCabrillo6->itemText(i) + "\n";
            file.write(tmp.toAscii());
        }
    }
    if (cabrillo->comboBoxCabrillo7->isEnabled()) {
        QString tmp;
        tmp = "cabrillo7=" + cabrillo->labelCabrillo7->text() + "\n";
        file.write(tmp.toAscii());
        for (int i = 0; i < cabrillo->comboBoxCabrillo7->count(); i++) {
            tmp = "cabrillo7=" + cabrillo->comboBoxCabrillo7->itemText(i) + "\n";
            file.write(tmp.toAscii());
        }
    }
    if (!cabrillo->lineEditLocation->text().isEmpty()) {
        file.write("cabrillo location=" + cabrillo->lineEditLocation->text().toAscii()+"\n");
    }
*/
    file.close();

    So2sdrStatusBar->showMessage("Saved " + fileName, 3000);
    return(true);
}

