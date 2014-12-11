/*! Copyright 2010-2015 R. Torsten Clay N4OGW

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
#include "signal.h"

Signal::Signal()
{
    active = false;
    cnt    = 0;
    n      = 0;
    f      = 0;
    fcq    = 0;
    space  = 0;
    fsum   = 0;
}

void Signal::clear()
{
    active = false;
    cnt    = 0;
    f      = 0;
    fcq    = 0;
    fsum   = 0;
    n      = 0;
    space  = 0;
}

CalibSignal::CalibSignal()
{
    n       = 0;
    zsum[0] = 0.;
    zsum[1] = 0.;
    z[0]    = 0.;
    z[1]    = 0.;
    gain    = 1.0;
    phase   = 0.;
}
