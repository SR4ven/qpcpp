//.$file${.::history.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: history.qm
// File:  ${.::history.cpp}
//
// This code has been generated by QM 5.0.0 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::history.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include "history.hpp"

#include "safe_std.h"   // portable "safe" <stdio.h>/<string.h> facilities
#include <stdlib.h>

Q_DEFINE_THIS_FILE

//.$declare${SMs::ToastOven} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${SMs::ToastOven} .........................................................
class ToastOven : public QP::QHsm {
public:
    ToastOven();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(doorClosed);
    Q_STATE_DECL(heating);
    Q_STATE_DECL(toasting);
    Q_STATE_DECL(baking);
    Q_STATE_DECL(off);
    Q_STATE_DECL(doorOpen);
    Q_STATE_DECL(final);

protected:
    QP::QStateHandler hist_doorClosed;
};
//.$enddecl${SMs::ToastOven} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

static ToastOven l_oven; // the only instance of the ToastOven class

// global-scope definitions ------------------------------------
QP::QHsm * const the_oven = &l_oven;       // the opaque pointer

//.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//. Check for the minimum required QP version
#if (QP_VERSION < 670U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.7.0 or higher required
#endif
//.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//.$define${SMs::ToastOven} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${SMs::ToastOven} .........................................................
//.${SMs::ToastOven::ToastOven} ..............................................
ToastOven::ToastOven()
 : QHsm(Q_STATE_CAST(&ToastOven::initial))
{}

//.${SMs::ToastOven::SM} .....................................................
Q_STATE_DEF(ToastOven, initial) {
    //.${SMs::ToastOven::SM::initial}
    (void)e; /* avoid compiler warning */
    // state history attributes
    hist_doorClosed = &off;

    QS_FUN_DICTIONARY(&doorClosed);
    QS_FUN_DICTIONARY(&heating);
    QS_FUN_DICTIONARY(&toasting);
    QS_FUN_DICTIONARY(&baking);
    QS_FUN_DICTIONARY(&off);
    QS_FUN_DICTIONARY(&doorOpen);
    QS_FUN_DICTIONARY(&final);

    return tran(&doorClosed);
}
//.${SMs::ToastOven::SM::doorClosed} .........................................
Q_STATE_DEF(ToastOven, doorClosed) {
    QP::QState status_;
    switch (e->sig) {
        //.${SMs::ToastOven::SM::doorClosed}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s;", "door-Closed");
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed}
        case Q_EXIT_SIG: {
            // save deep history
            hist_doorClosed = state();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::initial}
        case Q_INIT_SIG: {
            status_ = tran(&off);
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::TERMINATE}
        case TERMINATE_SIG: {
            status_ = tran(&final);
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::OPEN}
        case OPEN_SIG: {
            status_ = tran(&doorOpen);
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::TOAST}
        case TOAST_SIG: {
            status_ = tran(&toasting);
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::BAKE}
        case BAKE_SIG: {
            status_ = tran(&baking);
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::OFF}
        case OFF_SIG: {
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
//.${SMs::ToastOven::SM::doorClosed::heating} ................................
Q_STATE_DEF(ToastOven, heating) {
    QP::QState status_;
    switch (e->sig) {
        //.${SMs::ToastOven::SM::doorClosed::heating}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s;", "heater-On");
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::heating}
        case Q_EXIT_SIG: {
            PRINTF_S("%s;", "heater-Off");
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${SMs::ToastOven::SM::doorClosed::heating::initial}
        case Q_INIT_SIG: {
            status_ = tran(&toasting);
            break;
        }
        default: {
            status_ = super(&doorClosed);
            break;
        }
    }
    return status_;
}
//.${SMs::ToastOven::SM::doorClosed::heating::toasting} ......................
Q_STATE_DEF(ToastOven, toasting) {
    QP::QState status_;
    switch (e->sig) {
        //.${SMs::ToastOven::SM::doorClosed::heating::toasting}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s;", "toasting");
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&heating);
            break;
        }
    }
    return status_;
}
//.${SMs::ToastOven::SM::doorClosed::heating::baking} ........................
Q_STATE_DEF(ToastOven, baking) {
    QP::QState status_;
    switch (e->sig) {
        //.${SMs::ToastOven::SM::doorClosed::heating::baking}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s;", "baking");
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&heating);
            break;
        }
    }
    return status_;
}
//.${SMs::ToastOven::SM::doorClosed::off} ....................................
Q_STATE_DEF(ToastOven, off) {
    QP::QState status_;
    switch (e->sig) {
        //.${SMs::ToastOven::SM::doorClosed::off}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s;", "toaster-Off");
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&doorClosed);
            break;
        }
    }
    return status_;
}
//.${SMs::ToastOven::SM::doorOpen} ...........................................
Q_STATE_DEF(ToastOven, doorOpen) {
    QP::QState status_;
    switch (e->sig) {
        //.${SMs::ToastOven::SM::doorOpen}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s;", "door-Open,lamp-On");
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${SMs::ToastOven::SM::doorOpen}
        case Q_EXIT_SIG: {
            PRINTF_S("%s;", "lamp-Off");
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${SMs::ToastOven::SM::doorOpen::CLOSE}
        case CLOSE_SIG: {
            status_ = tran_hist(hist_doorClosed);
            break;
        }
        //.${SMs::ToastOven::SM::doorOpen::TERMINATE}
        case TERMINATE_SIG: {
            status_ = tran(&final);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.${SMs::ToastOven::SM::final} ..............................................
Q_STATE_DEF(ToastOven, final) {
    QP::QState status_;
    switch (e->sig) {
        //.${SMs::ToastOven::SM::final}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s\n", "-> final\nBye!Bye!");
            QP::QF::onCleanup();
            exit(0);
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.$enddef${SMs::ToastOven} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
