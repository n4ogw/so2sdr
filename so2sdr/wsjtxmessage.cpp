/*! Copyright 2010-2025 R. Torsten Clay N4OGW

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
#include "wsjtxmessage.h"

WsjtxMessage::WsjtxMessage(QByteArray *data, QIODevice::OpenMode mode)
    : QDataStream(data, mode) {
  schema = 2;
}

void WsjtxMessage::startMessage(WsjtxMessageType type) {
  if (schema <= 1) {
    this->setVersion(QDataStream::Qt_5_0);
  }
#if QT_VERSION >= 0x050200
  else if (schema <= 2) {
    this->setVersion(QDataStream::Qt_5_2);
  }
#endif
#if QT_VERSION >= 0x050400
  else if (schema <= 3) {
    this->setVersion(QDataStream::Qt_5_4);
  }
#endif
  *this << magic << schema << static_cast<quint32>(type)
        << QByteArray("SO2SDR");
}
