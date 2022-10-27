//============================================================================
// Product: BSP for system-testing QXK
// Last updated for version 7.1.0
// Last updated on  2022-08-27
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// https://state-machine.com
// <info@state-machine.com>
//============================================================================
#ifndef BSP_HPP
#define BSP_HPP

namespace BSP {

enum { TICKS_PER_SEC = 100 };

void init(void);
void terminate(int16_t const result);

// for testing...
void wait4PB1(void);
void ledOn(void);
void ledOff(void);

} // namespace BSP

enum TestSignals {
    TEST_SIG = QP::Q_USER_SIG,
    MAX_PUB_SIG,    // the last published signal

    TIMEOUT_SIG,
    MAX_SIG         // the last signal
};

#endif // BSP_HPP
