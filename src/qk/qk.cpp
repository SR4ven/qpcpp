//$file${src::qk::qk.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qk::qk.cpp}
//
// This code has been generated by QM 5.2.1 <www.state-machine.com/qm>.
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
//$endhead${src::qk::qk.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @date Last updated on: 2022-06-30
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QK/C++ preemptive kernel core functions

#define QP_IMPL             // this is QF/QK implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope internal interface
#include "qassert.h"        // QP assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// protection against including this source file in a wrong project
#ifndef QK_HPP
    #error "Source file included in a project NOT based on the QK kernel"
#endif // QK_HPP

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qk")
} // unnamed namespace

//============================================================================
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QK::QK-base} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QK {

//${QK::QK-base::schedLock} ..................................................
QSchedStatus schedLock(std::uint_fast8_t const ceiling) noexcept {
    QF_CRIT_STAT_
    QF_CRIT_E_();

    //! @pre The QK scheduler lock:
    //! - cannot be called from an ISR;
    Q_REQUIRE_ID(600, !QK_ISR_CONTEXT_());

    // first store the previous lock prio if it is below the ceiling
    QSchedStatus stat;
    if (static_cast<std::uint_fast8_t>(QK_attr_.lockPrio) < ceiling) {
        stat = (static_cast<QSchedStatus>(QK_attr_.lockPrio) << 8U);
        QK_attr_.lockPrio = static_cast<std::uint8_t>(ceiling);

        QS_BEGIN_NOCRIT_PRE_(QS_SCHED_LOCK, 0U)
            QS_TIME_PRE_(); // timestamp
            // prev and new lock prio...
            QS_2U8_PRE_(stat, QK_attr_.lockPrio);
        QS_END_NOCRIT_PRE_()

        // add the previous lock holder priority
        stat |= static_cast<QSchedStatus>(QK_attr_.lockHolder);

        QK_attr_.lockHolder = QK_attr_.actPrio;
    }
    else {
       stat = 0xFFU;
    }
    QF_CRIT_X_();

    return stat; // return the status to be saved in a stack variable
}

//${QK::QK-base::schedUnlock} ................................................
void schedUnlock(QSchedStatus const stat) noexcept {
    // has the scheduler been actually locked by the last QK_schedLock()?
    if (stat != 0xFFU) {
        std::uint_fast8_t const lockPrio =
            static_cast<std::uint_fast8_t>(QK_attr_.lockPrio);
        std::uint_fast8_t const prevPrio =
            static_cast<std::uint_fast8_t>(stat >> 8U);
        QF_CRIT_STAT_
        QF_CRIT_E_();

        //! @pre The scheduler cannot be unlocked:
        //! - from the ISR context; and
        //! - the current lock priority must be greater than the previous
        Q_REQUIRE_ID(700, (!QK_ISR_CONTEXT_())
                          && (lockPrio > prevPrio));

        QS_BEGIN_NOCRIT_PRE_(QS_SCHED_UNLOCK, 0U)
            QS_TIME_PRE_(); // timestamp
            QS_2U8_PRE_(lockPrio, prevPrio); //before & after
        QS_END_NOCRIT_PRE_()

        // restore the previous lock priority and lock holder
        QK_attr_.lockPrio   = static_cast<std::uint8_t>(prevPrio);
        QK_attr_.lockHolder = static_cast<std::uint8_t>(stat & 0xFFU);

        // find the highest-prio thread ready to run
        if (QK_sched_() != 0U) { // priority found?
            QK_activate_(); // activate any unlocked basic threads
        }

        QF_CRIT_X_();
    }
}

} // namespace QK
} // namespace QP
//$enddef${QK::QK-base} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QK::QF-cust} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QF {

//${QK::QF-cust::init} .......................................................
void init() {
    QF::maxPool_ = 0U;
    QActive::subscrList_   = nullptr;
    QActive::maxPubSignal_ = 0;

    bzero(&QTimeEvt::timeEvtHead_[0], sizeof(QTimeEvt::timeEvtHead_));
    bzero(&QActive::registry_[0], sizeof(QActive::registry_));
    bzero(&QF::readySet_, sizeof(QF::readySet_));
    bzero(&QK_attr_, sizeof(QK_attr_));

    QK_attr_.actPrio  = 0U; // prio of QK idle loop
    QK_attr_.lockPrio = QF_MAX_ACTIVE; // locked

    #ifdef QK_INIT
    QK_INIT(); // port-specific initialization of the QK kernel
    #endif
}

//${QK::QF-cust::stop} .......................................................
void stop() {
    onCleanup();  // cleanup callback
    // nothing else to do for the QK preemptive kernel
}

//${QK::QF-cust::run} ........................................................
int_t run() {
    QF_INT_DISABLE();
    QK_attr_.lockPrio = 0U; // scheduler unlocked

    // any active objects need to be scheduled before starting event loop?
    if (QK_sched_() != 0U) {
        QK_activate_(); // activate AOs to process all events posted so far
    }

    onStartup();      // application-specific startup callback

    // produce the QS_QF_RUN trace record
    QS_BEGIN_NOCRIT_PRE_(QS_QF_RUN, 0U)
    QS_END_NOCRIT_PRE_()

    QF_INT_ENABLE();

    // the QK idle loop...
    for (;;) {
        QK::onIdle(); // application-specific QK on-idle callback
    }
    #ifdef __GNUC__  // GNU compiler?
    return 0;
    #endif
}

} // namespace QF
} // namespace QP
//$enddef${QK::QF-cust} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QK::QActive} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QK::QActive} .............................................................

//${QK::QActive::start} ......................................................
void QActive::start(
    std::uint_fast8_t const prio,
    QEvt const * * const qSto,
    std::uint_fast16_t const qLen,
    void * const stkSto,
    std::uint_fast16_t const stkSize,
    void const * const par)
{
    Q_UNUSED_PAR(stkSize); // not needed in QK

    //! @pre
    //! AO cannot be started from an ISR, the priority must be in range
    //! and the stack storage must not be provided, because the QK kernel
    //! does not need per-AO stacks.
    Q_REQUIRE_ID(300, (!QK_ISR_CONTEXT_())
        && (0U < prio) && (prio <= QF_MAX_ACTIVE)
        && (stkSto == nullptr));

    m_eQueue.init(qSto, qLen); // initialize the built-in queue

    m_prio = static_cast<std::uint8_t>(prio);  // set the QF prio of this AO
    register_(); // make QF aware of this AO

    this->init(par, m_prio); // take the top-most initial tran. (virtual)
    QS_FLUSH(); // flush the trace buffer to the host

    // See if this AO needs to be scheduled in case QK is already running
    QF_CRIT_STAT_
    QF_CRIT_E_();
    if (QK_sched_() != 0U) { // activation needed?
        QK_activate_();
    }
    QF_CRIT_X_();
}

} // namespace QP
//$enddef${QK::QActive} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//============================================================================
extern "C" {
//$define${QK-extern-C} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${QK-extern-C::QK_attr_} ...................................................
QK_Attr QK_attr_;

//${QK-extern-C::QK_sched_} ..................................................
std::uint_fast8_t QK_sched_() noexcept {
    // find the highest-prio AO with non-empty event queue
    std::uint_fast8_t p = QP::QF::readySet_.findMax();

    // is the highest-prio below the active prio?
    if (p <= static_cast<std::uint_fast8_t>(QK_attr_.actPrio)) {
        p = 0U; // active object not eligible
    }
    else if (p <= static_cast<std::uint_fast8_t>(QK_attr_.lockPrio)) {
        p = 0U; // active object not eligible
    }
    else {
        Q_ASSERT_ID(410, p <= QF_MAX_ACTIVE);
        QK_attr_.nextPrio = static_cast<std::uint8_t>(p); // next AO to run
    }
    return p;
}

//${QK-extern-C::QK_activate_} ...............................................
void QK_activate_() noexcept {
    std::uint_fast8_t const pin =
        static_cast<std::uint_fast8_t>(QK_attr_.actPrio);
    std::uint_fast8_t p = static_cast<std::uint_fast8_t>(QK_attr_.nextPrio);

    // QK_attr_.actPrio and QK_attr_.nextPrio must be in ragne
    Q_REQUIRE_ID(500,
        (pin <= QF_MAX_ACTIVE)
        && (0U < p) && (p <= QF_MAX_ACTIVE));

    // QK Context switch callback defined or QS tracing enabled?
    #if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)
    std::uint_fast8_t pprev = pin;
    #endif // QK_ON_CONTEXT_SW || Q_SPY

    QK_attr_.nextPrio = 0U; // clear for the next time

    // loop until no more ready-to-run AOs of higher prio than the initial
    QP::QActive *a;
    do {
        a = QP::QActive::registry_[p]; // obtain the pointer to the AO
        QK_attr_.actPrio = static_cast<std::uint8_t>(p); // new active prio

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_NEXT, a->m_prio)
            QS_TIME_PRE_();        // timestamp
            QS_2U8_PRE_(p, pprev); // sechduled prio & previous prio...
        QS_END_NOCRIT_PRE_()

    #if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)
        if (p != pprev) {  // changing threads?

    #ifdef QK_ON_CONTEXT_SW
            // context-switch callback
            QK_onContextSw(((pprev != 0U)
                           ? QP::QActive::registry_[pprev]
                           : nullptr), a);
    #endif // QK_ON_CONTEXT_SW

            pprev = p; // update previous priority
        }
    #endif // QK_ON_CONTEXT_SW || Q_SPY

        QF_INT_ENABLE();  // unconditionally enable interrupts

        // perform the run-to-completion (RTS) step...
        // 1. retrieve the event from the AO's event queue, which by this
        //    time must be non-empty and QActive_get_() asserts it.
        // 2. dispatch the event to the AO's state machine.
        // 3. determine if event is garbage and collect it if so
        //
        QP::QEvt const * const e = a->get_();
        a->dispatch(e, a->m_prio);
        QP::QF::gc(e);

        // determine the next highest-priority AO ready to run...
        QF_INT_DISABLE();

        if (a->m_eQueue.isEmpty()) { // empty queue?
            QP::QF::readySet_.rmove(p);
        }

        // find new highest-prio AO ready to run...
        p = QP::QF::readySet_.findMax();

        // is the new priority below the initial preemption threshold?
        if (p <= pin) {
            p = 0U; // active object not eligible
        }
        else if (p <= static_cast<std::uint_fast8_t>(QK_attr_.lockPrio)) {
            p = 0U; // active object not eligible
        }
        else {
            Q_ASSERT_ID(510, p <= QF_MAX_ACTIVE);
        }
    } while (p != 0U);

    QK_attr_.actPrio = static_cast<std::uint8_t>(pin); // restore the prio

    #if (defined QK_ON_CONTEXT_SW) || (defined Q_SPY)

    if (pin != 0U) { // resuming an active object?
        a = QP::QActive::registry_[pin]; // the pointer to the preempted AO

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_RESUME, a->m_prio)
            QS_TIME_PRE_(); // timestamp
            QS_2U8_PRE_(pin, pprev); // resumed prio & previous prio...
        QS_END_NOCRIT_PRE_()
    }
    else {  // resuming priority==0 --> idle
        a = nullptr; // QK idle loop

        QS_BEGIN_NOCRIT_PRE_(QP::QS_SCHED_IDLE, 0U)
            QS_TIME_PRE_();    // timestamp
            QS_U8_PRE_(pprev); // previous prio
        QS_END_NOCRIT_PRE_()
    }

    #ifdef QK_ON_CONTEXT_SW
    QK_onContextSw(QP::QActive::registry_[pprev], a); // context-switch callback
    #endif // QK_ON_CONTEXT_SW

    #endif // QK_ON_CONTEXT_SW || Q_SPY
}
//$enddef${QK-extern-C} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
} // extern "C"
