#include "call.h"
#include <QByteArray>
Call::Call()
{
    call.clear();
    freq=0;
    rgbCall[0]=0; // call black by default
    rgbCall[1]=0;
    rgbCall[2]=0;
    markRgb[0]=false;
    markRgb[1]=false;
    markRgb[2]=false;
    mark=false;
}
