//.$file${.::philo.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp.qm
// File:  ${.::philo.cpp}
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
//.$endhead${.::philo.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include "dpp.hpp"
#include "bsp.hpp"

Q_DEFINE_THIS_FILE

// Active object class -------------------------------------------------------
//.$declare${AOs::Philo} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace DPP {

//.${AOs::Philo} .............................................................
class Philo : public QP::QActive {
private:
    QP::QTimeEvt m_timeEvt;

public:
    Philo();

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(thinking);
    Q_STATE_DECL(hungry);
    Q_STATE_DECL(eating);
};

} // namespace DPP
//.$enddecl${AOs::Philo} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

namespace DPP {

// Local objects -------------------------------------------------------------
static Philo l_philo[N_PHILO];   // storage for all Philos

// helper function to provide a randomized think time for Philos
inline QP::QTimeEvtCtr think_time() {
    return static_cast<QP::QTimeEvtCtr>((BSP::random() % BSP::TICKS_PER_SEC)
                                        + (BSP::TICKS_PER_SEC/2U));
}

// helper function to provide a randomized eat time for Philos
inline QP::QTimeEvtCtr eat_time() {
    return static_cast<QP::QTimeEvtCtr>((BSP::random() % BSP::TICKS_PER_SEC)
                                        + BSP::TICKS_PER_SEC);
}

// helper function to provide the ID of Philo
inline uint8_t PHILO_ID(Philo const * const philo) {
    return static_cast<uint8_t>(philo - l_philo);
}

enum InternalSignals {           // internal signals
    TIMEOUT_SIG = MAX_SIG
};

// Global objects ------------------------------------------------------------
QP::QActive * const AO_Philo[N_PHILO] = { // "opaque" pointers to Philo AO
    &l_philo[0],
    &l_philo[1],
    &l_philo[2],
    &l_philo[3],
    &l_philo[4]
};

} // namespace DPP

// Philo definition ----------------------------------------------------------
//.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//. Check for the minimum required QP version
#if (QP_VERSION < 670U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.7.0 or higher required
#endif
//.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//.$define${AOs::Philo} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace DPP {

//.${AOs::Philo} .............................................................
//.${AOs::Philo::Philo} ......................................................
Philo::Philo()
  : QActive(Q_STATE_CAST(&Philo::initial)),
    m_timeEvt(this, TIMEOUT_SIG, 0U)
{}

//.${AOs::Philo::SM} .........................................................
Q_STATE_DEF(Philo, initial) {
    //.${AOs::Philo::SM::initial}
    static bool registered = false; // starts off with 0, per C-standard
    (void)e; // suppress the compiler warning about unused parameter

    subscribe(EAT_SIG);
    subscribe(TEST_SIG);

    if (!registered) {
        registered = true;

        QS_OBJ_DICTIONARY(&l_philo[0].m_timeEvt);
        QS_OBJ_DICTIONARY(&l_philo[1].m_timeEvt);
        QS_OBJ_DICTIONARY(&l_philo[2].m_timeEvt);
        QS_OBJ_DICTIONARY(&l_philo[3].m_timeEvt);
        QS_OBJ_DICTIONARY(&l_philo[4].m_timeEvt);

        QS_FUN_DICTIONARY(&initial);
        QS_FUN_DICTIONARY(&thinking);
        QS_FUN_DICTIONARY(&hungry);
        QS_FUN_DICTIONARY(&eating);
    }

    QS_SIG_DICTIONARY(HUNGRY_SIG,  this); // signal for each Philos
    QS_SIG_DICTIONARY(TIMEOUT_SIG, this); // signal for each Philos
    return tran(&thinking);
}
//.${AOs::Philo::SM::thinking} ...............................................
Q_STATE_DEF(Philo, thinking) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Philo::SM::thinking}
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(think_time(), 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::thinking}
        case Q_EXIT_SIG: {
            (void)m_timeEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::thinking::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&hungry);
            break;
        }
        //.${AOs::Philo::SM::thinking::EAT, DONE}
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::thinking::TEST}
        case TEST_SIG: {
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
//.${AOs::Philo::SM::hungry} .................................................
Q_STATE_DEF(Philo, hungry) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Philo::SM::hungry}
        case Q_ENTRY_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, HUNGRY_SIG);
            pe->philoNum = PHILO_ID(this);
            AO_Table->POST(pe, this);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::hungry::EAT}
        case EAT_SIG: {
            //.${AOs::Philo::SM::hungry::EAT::[Q_EVT_CAST(TableEvt)->philoNum=~}
            if (Q_EVT_CAST(TableEvt)->philoNum == PHILO_ID(this)) {
                status_ = tran(&eating);
            }
            else {
                status_ = Q_RET_UNHANDLED;
            }
            break;
        }
        //.${AOs::Philo::SM::hungry::DONE}
        case DONE_SIG: {
            /* DONE must be for other Philos than this one */
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
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
//.${AOs::Philo::SM::eating} .................................................
Q_STATE_DEF(Philo, eating) {
    QP::QState status_;
    switch (e->sig) {
        //.${AOs::Philo::SM::eating}
        case Q_ENTRY_SIG: {
            m_timeEvt.armX(eat_time(), 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::eating}
        case Q_EXIT_SIG: {
            TableEvt *pe = Q_NEW(TableEvt, DONE_SIG);
            pe->philoNum = PHILO_ID(this);
            QP::QF::PUBLISH(pe, this);
            (void)m_timeEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${AOs::Philo::SM::eating::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&thinking);
            break;
        }
        //.${AOs::Philo::SM::eating::EAT, DONE}
        case EAT_SIG: // intentionally fall through
        case DONE_SIG: {
            // EAT or DONE must be for other Philos than this one
            Q_ASSERT(Q_EVT_CAST(TableEvt)->philoNum != PHILO_ID(this));
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

} // namespace DPP
//.$enddef${AOs::Philo} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
