#include "mytelnetwidget.h"

MyTelnetWidget::MyTelnetWidget(QWidget *parent) : QPlainTextEdit(parent), t(new QtTelnet())
{
    // Disable text zone while not connected
    QPlainTextEdit::setEnabled(false);

    QPlainTextEdit::setUndoRedoEnabled(false);

    // Set a fixed width for carachters
    // TODO utiliser l'algorithm qui cherche une police qui correspond au caract?re set au QFont
    QFont font = QFont("Bitstream Vera Sans Mono");
    QPlainTextEdit::setFont(font);

    connect(t->socket(), SIGNAL(connected()), this, SLOT(sockConnected()));
    connect(t->socket(), SIGNAL(disconnected()), this, SLOT(sockDisconnected()));
}

MyTelnetWidget::~MyTelnetWidget()
{
    delete t;
}

void MyTelnetWidget::connectToHost(const QString &host, const quint16 port)
{
    t->connectToHost(host,port);
}

void MyTelnetWidget::closeSession()
{
    t->close();
}

void MyTelnetWidget::sockConnected()
{
    qDebug() << "Connected";
    connect(t, SIGNAL(message(const QString &)), this, SLOT(processIncomingData(const QString &)));

    QPlainTextEdit::setEnabled(true);
    emit connected();
}

void MyTelnetWidget::sockDisconnected()
{
    qDebug() << "Disconnected";
    //QPlainTextEdit::appendPlainText("\n");
    QPlainTextEdit::setEnabled(false);
    emit disconnected();
}

void MyTelnetWidget::keyPressEvent(QKeyEvent * ke)
{
    if(!t->socket()->isValid())
    {
        ke->ignore();
        return;
    }

    qDebug("%d",ke->key());

    if (ke->modifiers() == Qt::NoModifier)
    {
        switch (ke->key())
        {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                t->sendData("\r\n");
                break;
            case Qt::Key_Left:
                t->sendData("\033[D");
                break;
            case Qt::Key_Right:
                t->sendData("\033[C");
                break;
            case Qt::Key_Up:
                t->sendData("\033[A");
                break;
            case Qt::Key_Down:
                t->sendData("\033[B");
                break;
            case Qt::Key_Delete:
                t->sendData(QChar(0x04));
                break;
            case Qt::Key_Home:
                t->sendData(QChar(0x01));
                break;
            case Qt::Key_End:
                t->sendData(QChar(0x05));
                break;
            default:
                t->sendData(ke->text());
        }
    }
    else if (ke->modifiers() & Qt::ControlModifier)
    {
        switch (ke->key())
        {
            default:
                t->sendData(ke->text());
        }
    }
    else
        // Send Event I didn't know ... for the moment !
        //QPlainTextEdit::keyPressEvent(ke);
        t->sendData(ke->text());
}

void MyTelnetWidget::processIncomingData(const QString &text)
{
    qDebug() << "receiving data";
    QString reformatedText;

    if (text == NULL)
        return;


    reformatedText = text;
    reformatedText.remove('\r');

    // '\a' == 0x07 (bell)
    if (reformatedText.contains('\a'))
    {
        reformatedText.remove('\a');
        emit alert();
    }

    // '\b' == 0x08 (backspace)     but for telnet it s go left
    if ( reformatedText.contains('\b') )
    {
        if (reformatedText == "\b")
        {
            this->moveCursor(QTextCursor::Left);
            return;
        }
    }

    if ( reformatedText.startsWith("\n") )
    {
        this->moveCursor(QTextCursor::End);
        this->insertPlainText("\n");
        reformatedText.remove(0,1);
    }

    //reformatedText.remove(QRegExp("\033\\[[0-9;]*[A-Za-z]")); // Also remove terminal control codes

    simulateOverWriteMode(reformatedText);

    // AutoScroll
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}

void MyTelnetWidget::simulateOverWriteMode(QString text)    // simulate overwrite mode + interpret '\b'
{
    for (int i=0; i<text.length(); i++)
    {
        if (text.at(i) == '\b')
            this->moveCursor(QTextCursor::Left);
        else
        {
            this->textCursor().deleteChar();
            this->insertPlainText(text.at(i));
        }
    }
}


void MyTelnetWidget::mousePressEvent(QMouseEvent *me)
{
}


