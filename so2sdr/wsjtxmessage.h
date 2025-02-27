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
#ifndef WSJTXMESSAGE_H
#define WSJTXMESSAGE_H

#include "udpreader.h"
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

// for sending UDP data stream to wsjtx
class WsjtxMessage : public QDataStream {
public:
  explicit WsjtxMessage(QByteArray *data, QIODevice::OpenMode mode);
  void startMessage(WsjtxMessageType type);
  void setSchema(quint32 s) { schema = s; }

private:
  quint32 schema;
};

#endif // WSJTXMESSAGE_H
