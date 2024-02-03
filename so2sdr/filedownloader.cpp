/*! Copyright 2010-2024 R. Torsten Clay N4OGW

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

    This class derived from code at https://wiki.qt.io/Download_Data_from_URL
 */
#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent)
    : QObject(parent) {
  connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply *)), this,
          SLOT(fileDownloaded(QNetworkReply *)));
  QNetworkRequest request(imageUrl);
  request.setHeader(QNetworkRequest::UserAgentHeader, QVariant("Wget/1.19"));
  m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader() {}

void FileDownloader::fileDownloaded(QNetworkReply *pReply) {
  m_DownloadedData = pReply->readAll();
  pReply->deleteLater();
  emit downloaded();
}

QByteArray FileDownloader::downloadedData() const { return m_DownloadedData; }
