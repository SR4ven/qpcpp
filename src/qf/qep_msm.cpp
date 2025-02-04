//$file${src::qf::qep_msm.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qf::qep_msm.cpp}
//
// This code has been generated by QM 5.2.2 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This code is covered by the following QP license:
// License #    : LicenseRef-QL-dual
// Issued to    : Any user of the QP/C++ real-time embedded framework
// Framework(s) : qpcpp
// Support ends : 2023-12-31
// License scope:
//
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${src::qf::qep_msm.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @file
//! @brief QP::QMsm implementation

#define QP_IMPL             // this is QP implementation
#include "qep_port.hpp"     // QEP port
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY
#include "qassert.h"        // QP embedded systems-friendly assertions

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qep_msm")
} // unnamed namespace

//============================================================================
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QEP::QMsm} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QEP::QMsm} ...............................................................
QMState const QMsm::msm_top_s  = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};


//${QEP::QMsm::init} .........................................................
void QMsm::init(
    void const * const e,
    std::uint_fast8_t const qs_id)
{
    //! @pre the top-most initial transition must be initialized, and the
    //! initial transition must not be taken yet.
    Q_REQUIRE_ID(200, (m_temp.fun != nullptr)
                      && (m_state.obj == &msm_top_s));

    // execute the top-most initial tran.
    QState r = (*m_temp.fun)(this, Q_EVT_CAST(QEvt));

    // initial tran. must be taken
    Q_ASSERT_ID(210, r == Q_RET_TRAN_INIT);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_STATE_INIT, qs_id)
        QS_OBJ_PRE_(this);  // this state machine object
        QS_FUN_PRE_(m_state.obj->stateHandler);          // source handler
        QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target handler
    QS_END_PRE_()

    // set state to the last tran. target
    m_state.obj = m_temp.tatbl->target;

    // drill down into the state hierarchy with initial transitions...
    do {
        r = execTatbl_(m_temp.tatbl, qs_id); // execute the tran-action table
    } while (r >= Q_RET_TRAN_INIT);

    QS_BEGIN_PRE_(QS_QEP_INIT_TRAN, qs_id)
        QS_TIME_PRE_();                         // time stamp
        QS_OBJ_PRE_(this);                      // this state machine object
        QS_FUN_PRE_(m_state.obj->stateHandler); // the new current state
    QS_END_PRE_()

    Q_UNUSED_PAR(qs_id); // when Q_SPY not defined
}

//${QEP::QMsm::dispatch} .....................................................
void QMsm::dispatch(
    QEvt const * const e,
    std::uint_fast8_t const qs_id)
{
    QMState const *s = m_state.obj;  // store the current state
    //! @pre current state must be initialized
    Q_REQUIRE_ID(300, s != nullptr);

    QS_CRIT_STAT_
    QS_BEGIN_PRE_(QS_QEP_DISPATCH, qs_id)
        QS_TIME_PRE_();               // time stamp
        QS_SIG_PRE_(e->sig);          // the signal of the event
        QS_OBJ_PRE_(this);            // this state machine object
        QS_FUN_PRE_(s->stateHandler); // the current state handler
    QS_END_PRE_()

    // scan the state hierarchy up to the top state...
    QMState const *t = s;
    QState r;
    do {
        r = (*t->stateHandler)(this, e); // call state handler function

        // event handled? (the most frequent case)
        if (r >= Q_RET_HANDLED) {
            break; // done scanning the state hierarchy
        }
        // event unhandled and passed to the superstate?
        else if (r == Q_RET_SUPER) {
            t = t->superstate; // advance to the superstate
        }
        // event unhandled and passed to a submachine superstate?
        else if (r == Q_RET_SUPER_SUB) {
            t = m_temp.obj; // current host state of the submachie
        }
        // event unhandled due to a guard?
        else if (r == Q_RET_UNHANDLED) {

            QS_BEGIN_PRE_(QS_QEP_UNHANDLED, qs_id)
                QS_SIG_PRE_(e->sig);    // the signal of the event
                QS_OBJ_PRE_(this);      // this state machine object
                QS_FUN_PRE_(t->stateHandler); // the current state
            QS_END_PRE_()

            t = t->superstate; // advance to the superstate
        }
        else {
            // no other return value should be produced
            Q_ERROR_ID(310);
        }
    } while (t != nullptr);

    // any kind of transition taken?
    if (r >= Q_RET_TRAN) {
    #ifdef Q_SPY
        QMState const * const ts = t; // transition source for QS tracing

        // the transition source state must not be nullptr
        Q_ASSERT_ID(320, ts != nullptr);
    #endif // Q_SPY

        do {
            // save the transition-action table before it gets clobbered
            QMTranActTable const * const tatbl = m_temp.tatbl;
            QHsmAttr tmp; // temporary to save intermediate values

            // was TRAN, TRAN_INIT, or TRAN_EP taken?
            if (r <= Q_RET_TRAN_EP) {
                exitToTranSource_(s, t, qs_id);
                r = execTatbl_(tatbl, qs_id);
                s = m_state.obj;
            }
            // was a transition segment to history taken?
            else if (r == Q_RET_TRAN_HIST) {
                tmp.obj = m_state.obj; // save history
                m_state.obj = s; // restore the original state
                exitToTranSource_(s, t, qs_id);
                static_cast<void>(execTatbl_(tatbl, qs_id));
                r = enterHistory_(tmp.obj, qs_id);
                s = m_state.obj;
            }
            // was a transition segment to an exit point taken?
            else if (r == Q_RET_TRAN_XP) {
                tmp.act = m_state.act; // save XP action
                m_state.obj = s; // restore the original state
                r = (*tmp.act)(this); // execute the XP action
                if (r == Q_RET_TRAN) { // XP -> TRAN ?
    #ifdef Q_SPY
                    tmp.tatbl = m_temp.tatbl; // save m_temp
    #endif // Q_SPY
                    exitToTranSource_(s, t, qs_id);
                    // take the tran-to-XP segment inside submachine
                    static_cast<void>(execTatbl_(tatbl, qs_id));
                    s = m_state.obj;
    #ifdef Q_SPY
                    m_temp.tatbl = tmp.tatbl; // restore m_temp
    #endif // Q_SPY
                }
                else if (r == Q_RET_TRAN_HIST) { // XP -> HIST ?
                    tmp.obj = m_state.obj; // save the history
                    m_state.obj = s; // restore the original state
    #ifdef Q_SPY
                    s = m_temp.obj; // save m_temp
    #endif // Q_SPY
                    exitToTranSource_(m_state.obj, t, qs_id);
                    // take the tran-to-XP segment inside submachine
                    static_cast<void>(execTatbl_(tatbl, qs_id));
    #ifdef Q_SPY
                    m_temp.obj = s; // restore me->temp
    #endif // Q_SPY
                    s = m_state.obj;
                    m_state.obj = tmp.obj; // restore the history
                }
                else {
                    // TRAN_XP must NOT be followed by any other tran type
                    Q_ASSERT_ID(330, r < Q_RET_TRAN);
                }
            }
            else {
                // no other return value should be produced
                Q_ERROR_ID(340);
            }

            t = s; // set target to the current state

        } while (r >= Q_RET_TRAN);

        QS_BEGIN_PRE_(QS_QEP_TRAN, qs_id)
            QS_TIME_PRE_();                // time stamp
            QS_SIG_PRE_(e->sig);           // the signal of the event
            QS_OBJ_PRE_(this);             // this state machine object
            QS_FUN_PRE_(ts->stateHandler); // the transition source
            QS_FUN_PRE_(s->stateHandler);  // the new active state
        QS_END_PRE_()
    }

    #ifdef Q_SPY
    // was the event handled?
    else if (r == Q_RET_HANDLED) {
        // internal tran. source can't be nullptr
        Q_ASSERT_ID(340, t != nullptr);

        QS_BEGIN_PRE_(QS_QEP_INTERN_TRAN, qs_id)
            QS_TIME_PRE_();               // time stamp
            QS_SIG_PRE_(e->sig);          // the signal of the event
            QS_OBJ_PRE_(this);            // this state machine object
            QS_FUN_PRE_(t->stateHandler); // the source state
        QS_END_PRE_()

    }
    // event bubbled to the 'top' state?
    else if (t == nullptr) {

        QS_BEGIN_PRE_(QS_QEP_IGNORED, qs_id)
            QS_TIME_PRE_();               // time stamp
            QS_SIG_PRE_(e->sig);          // the signal of the event
            QS_OBJ_PRE_(this);            // this state machine object
            QS_FUN_PRE_(s->stateHandler); // the current state
        QS_END_PRE_()

    }
    #endif // Q_SPY

    else {
        // empty
    }

    Q_UNUSED_PAR(qs_id); // when Q_SPY not defined
}

//${QEP::QMsm::isInState} ....................................................
bool QMsm::isInState(QMState const * const st) const noexcept {
    bool inState = false; // assume that this MSM is not in 'state'

    for (QMState const *s = m_state.obj;
         s != nullptr;
         s = s->superstate)
    {
        if (s == st) {
            inState = true; // match found, return 'true'
            break;
        }
    }
    return inState;
}

//${QEP::QMsm::childStateObj} ................................................
QMState const * QMsm::childStateObj(QMState const * const parent) const noexcept {
    QMState const *child = m_state.obj;
    bool isFound = false; // start with the child not found

    for (QMState const *s = m_state.obj;
         s != nullptr;
         s = s->superstate)
    {
        if (s == parent) {
            isFound = true; // child is found
            break;
        }
        else {
            child = s;
        }
    }

    //! @post the child must be found
    Q_ENSURE_ID(810, isFound);

    #ifdef Q_NASSERT
    Q_UNUSED_PAR(isFound);
    #endif

    return child; // return the child
}

//${QEP::QMsm::execTatbl_} ...................................................
QState QMsm::execTatbl_(
    QMTranActTable const * const tatbl,
    std::uint_fast8_t const qs_id)
{
    //! @pre the transition-action table pointer must not be nullptr
    Q_REQUIRE_ID(400, tatbl != nullptr);

    QState r = Q_RET_NULL;
    QS_CRIT_STAT_
    for (QActionHandler const *a = &tatbl->act[0]; *a != nullptr; ++a) {
        r = (*(*a))(this); // call the action through the 'a' pointer
    #ifdef Q_SPY
        if (r == Q_RET_ENTRY) {

            QS_BEGIN_PRE_(QS_QEP_STATE_ENTRY, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(m_temp.obj->stateHandler); // entered state handler
            QS_END_PRE_()
        }
        else if (r == Q_RET_EXIT) {

            QS_BEGIN_PRE_(QS_QEP_STATE_EXIT, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(m_temp.obj->stateHandler); // exited state handler
            QS_END_PRE_()
        }
        else if (r == Q_RET_TRAN_INIT) {

            QS_BEGIN_PRE_(QS_QEP_STATE_INIT, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(tatbl->target->stateHandler);        // source
                QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target
            QS_END_PRE_()
        }
        else if (r == Q_RET_TRAN_EP) {

            QS_BEGIN_PRE_(QS_QEP_TRAN_EP, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(tatbl->target->stateHandler);        // source
                QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target
            QS_END_PRE_()
        }
        else if (r == Q_RET_TRAN_XP) {

            QS_BEGIN_PRE_(QS_QEP_TRAN_XP, qs_id)
                QS_OBJ_PRE_(this); // this state machine object
                QS_FUN_PRE_(tatbl->target->stateHandler);        // source
                QS_FUN_PRE_(m_temp.tatbl->target->stateHandler); // target
            QS_END_PRE_()
        }
        else {
            // empty
        }
    #endif // Q_SPY
    }

    Q_UNUSED_PAR(qs_id); // when Q_SPY not defined
    m_state.obj = (r >= Q_RET_TRAN)
        ? m_temp.tatbl->target
        : tatbl->target;
    return r;
}

//${QEP::QMsm::exitToTranSource_} ............................................
void QMsm::exitToTranSource_(
    QMState const * s,
    QMState const * const ts,
    std::uint_fast8_t const qs_id)
{
    // exit states from the current state to the tran. source state
    while (s != ts) {
        // exit action provided in state 's'?
        if (s->exitAction != nullptr) {
            // execute the exit action
            (*s->exitAction)(this);

            QS_CRIT_STAT_
            QS_BEGIN_PRE_(QS_QEP_STATE_EXIT, qs_id)
                QS_OBJ_PRE_(this);            // this state machine object
                QS_FUN_PRE_(s->stateHandler); // the exited state handler
            QS_END_PRE_()
        }

        s = s->superstate; // advance to the superstate

        // reached the top of a submachine?
        if (s == nullptr) {
            s = m_temp.obj; // the superstate from QM_SM_EXIT()
            Q_ASSERT_ID(510, s != nullptr);
        }
    }
    Q_UNUSED_PAR(qs_id); // when Q_SPY not defined
}

//${QEP::QMsm::enterHistory_} ................................................
QState QMsm::enterHistory_(
    QMState const * const hist,
    std::uint_fast8_t const qs_id)
{
    QMState const *s = hist;
    QMState const *ts = m_state.obj; // transition source
    QMState const *epath[MAX_ENTRY_DEPTH_];

    QS_CRIT_STAT_

    QS_BEGIN_PRE_(QS_QEP_TRAN_HIST, qs_id)
        QS_OBJ_PRE_(this);               // this state machine object
        QS_FUN_PRE_(ts->stateHandler);   // source state handler
        QS_FUN_PRE_(hist->stateHandler); // target state handler
    QS_END_PRE_()

    std::int_fast8_t i = 0;  // entry path index
    while (s != ts) {
        if (s->entryAction != nullptr) {
            Q_ASSERT_ID(620, i < MAX_ENTRY_DEPTH_);
            epath[i] = s;
            ++i;
        }
        s = s->superstate;
        if (s == nullptr) {
            ts = s; // force exit from the for-loop
        }
    }

    // retrace the entry path in reverse (desired) order...
    while (i > 0) {
        --i;
        // run entry action in epath[i]
        (*epath[i]->entryAction)(this);

        QS_BEGIN_PRE_(QS_QEP_STATE_ENTRY, qs_id)
            QS_OBJ_PRE_(this);
            QS_FUN_PRE_(epath[i]->stateHandler); // entered state handler
        QS_END_PRE_()
    }

    m_state.obj = hist; // set current state to the transition target

    // initial tran. present?
    QState r;
    if (hist->initAction != nullptr) {
        r = (*hist->initAction)(this); // execute the transition action
    }
    else {
        r = Q_RET_NULL;
    }

    Q_UNUSED_PAR(qs_id); // when Q_SPY not defined
    return r;
}

} // namespace QP
//$enddef${QEP::QMsm} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
