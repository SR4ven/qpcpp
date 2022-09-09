//$file${.::blinky.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: blinky.qm
// File:  ${.::blinky.cpp}
//
// This code has been generated by QM 5.2.1 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${.::blinky.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include <iostream>
#include <cstdlib> // for exit()

using namespace std;
using namespace QP;

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif

enum { BSP_TICKS_PER_SEC = 100 };

void BSP_ledOff(void) {
    cout << "LED OFF" << endl;
}
void BSP_ledOn(void) {
    cout << "LED ON" << endl;
}
extern "C" void Q_onAssert(char const * const module, int loc) {
    cout << "Assertion failed in " << module << ':' << loc << endl;
    exit(-1);
}
void QF::onStartup(void) {}
void QF::onCleanup(void) {}
void QF::onClockTick(void) {
    QTimeEvt::TICK_X(0U, nullptr);  // perform the QF clock tick processing
}

enum BlinkySignals {
    TIMEOUT_SIG = Q_USER_SIG,
    MAX_SIG
};

//=============== ask QM to declare the Blinky class ==================
//$declare${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Blinky} .............................................................
class Blinky : public QP::QActive {
private:
    QP::QTimeEvt m_timeEvt;

public:
    Blinky();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(off);
    Q_STATE_DECL(on);
}; // class Blinky
//$enddecl${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

static Blinky l_blinky;
QActive * const AO_Blinky = &l_blinky;

int main() {
    // statically allocate event queue buffer for the Blinky AO
    static QEvt const *blinky_queueSto[10];

    QF::init(); // initialize the framework
    AO_Blinky->start(1U, // priority
                     blinky_queueSto, Q_DIM(blinky_queueSto),
                     nullptr, 0U); // no stack
    return QF::run(); // run the QF application
}

//================ ask QM to define the Blinky class ==================
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Blinky} .............................................................

//${AOs::Blinky::Blinky} .....................................................
Blinky::Blinky()
: QActive(Q_STATE_CAST(&Blinky::initial)),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{}

//${AOs::Blinky::SM} .........................................................
Q_STATE_DEF(Blinky, initial) {
    //${AOs::Blinky::SM::initial}
    m_timeEvt.armX(BSP_TICKS_PER_SEC/2, BSP_TICKS_PER_SEC/2);
    (void)e; // unused parameter

    QS_FUN_DICTIONARY(&Blinky::off);
    QS_FUN_DICTIONARY(&Blinky::on);

    return tran(&off);
}

//${AOs::Blinky::SM::off} ....................................................
Q_STATE_DEF(Blinky, off) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Blinky::SM::off}
        case Q_ENTRY_SIG: {
            BSP_ledOff();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Blinky::SM::off::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&on);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

//${AOs::Blinky::SM::on} .....................................................
Q_STATE_DEF(Blinky, on) {
    QP::QState status_;
    switch (e->sig) {
        //${AOs::Blinky::SM::on}
        case Q_ENTRY_SIG: {
            BSP_ledOn();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${AOs::Blinky::SM::on::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&off);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//$enddef${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
