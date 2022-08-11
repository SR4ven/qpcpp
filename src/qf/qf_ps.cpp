//$file${src::qf::qf_ps.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qf::qf_ps.cpp}
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
//$endhead${src::qf::qf_ps.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @date Last updated on: 2022-06-30
//! @version Last updated for: @ref qpcpp_7_0_1
//!
//! @file
//! @brief QF/C++ Publish-Subscribe services
//! definitions.

#define QP_IMPL             // this is QP implementation
#include "qf_port.hpp"      // QF port
#include "qf_pkg.hpp"       // QF package-scope interface
#include "qassert.h"        // QP embedded systems-friendly assertions
#ifdef Q_SPY                // QS software tracing enabled?
    #include "qs_port.hpp"  // QS port
    #include "qs_pkg.hpp"   // QS facilities for pre-defined trace records
#else
    #include "qs_dummy.hpp" // disable the QS software tracing
#endif // Q_SPY

// unnamed namespace for local definitions with internal linkage
namespace {
Q_DEFINE_THIS_MODULE("qf_ps")
} // unnamed namespace

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QF::QActive::subscrList_} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
QSubscrList * QActive::subscrList_;

} // namespace QP
//$enddef${QF::QActive::subscrList_} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QActive::maxPubSignal_} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
enum_t QActive::maxPubSignal_;

} // namespace QP
//$enddef${QF::QActive::maxPubSignal_} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QF::QActive::psInit} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::psInit} .....................................................
void QActive::psInit(
    QSubscrList * const subscrSto,
    enum_t const maxSignal) noexcept
{
    subscrList_   = subscrSto;
    maxPubSignal_ = maxSignal;
    QF::bzero(subscrSto, static_cast<unsigned>(maxSignal)
                         * sizeof(QSubscrList));
}

} // namespace QP
//$enddef${QF::QActive::psInit} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QActive::publish_} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::publish_} ...................................................
void QActive::publish_(
    QEvt const * const e,
    void const * const sender,
    std::uint_fast8_t const qs_id) noexcept
{
    Q_UNUSED_PAR(sender); // when Q_SPY not defined
    Q_UNUSED_PAR(qs_id); // when Q_SPY not defined

    //! @pre the published signal must be within the configured range
    Q_REQUIRE_ID(100, static_cast<enum_t>(e->sig) < maxPubSignal_);

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_PUBLISH, qs_id)
        QS_TIME_PRE_();                      // the timestamp
        QS_OBJ_PRE_(sender);                 // the sender object
        QS_SIG_PRE_(e->sig);                 // the signal of the event
        QS_2U8_PRE_(e->poolId_, e->refCtr_); // pool Id & refCtr of the evt
    QS_END_NOCRIT_PRE_()

    // is it a dynamic event?
    if (e->poolId_ != 0U) {
        // NOTE: The reference counter of a dynamic event is incremented to
        // prevent premature recycling of the event while the multicasting
        // is still in progress. At the end of the function, the garbage
        // collector step (QF::gc()) decrements the reference counter and
        // recycles the event if the counter drops to zero. This covers the
        // case when the event was published without any subscribers.
        //
        QF_EVT_REF_CTR_INC_(e);
    }

    // make a local, modifiable copy of the subscriber list
    QPSet subscrList = subscrList_[e->sig];
    QF_CRIT_X_();

    if (subscrList.notEmpty()) { // any subscribers?
        // the highest-prio subscriber
        std::uint_fast8_t p = subscrList.findMax();
        QF_SCHED_STAT_

        QF_SCHED_LOCK_(p); // lock the scheduler up to prio 'p'
        do { // loop over all subscribers */
            // the prio of the AO must be registered with the framework
            Q_ASSERT_ID(210, registry_[p] != nullptr);

            // POST() asserts internally if the queue overflows
            registry_[p]->POST(e, sender);

            subscrList.rmove(p); // remove the handled subscriber
            if (subscrList.notEmpty()) {  // still more subscribers?
                p = subscrList.findMax(); // the highest-prio subscriber
            }
            else {
                p = 0U; // no more subscribers
            }
        } while (p != 0U);
        QF_SCHED_UNLOCK_(); // unlock the scheduler
    }

    // The following garbage collection step decrements the reference counter
    // and recycles the event if the counter drops to zero. This covers both
    // cases when the event was published with or without any subscribers.
    //
    QF::gc(e);
}

} // namespace QP
//$enddef${QF::QActive::publish_} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QActive::subscribe} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::subscribe} ..................................................
void QActive::subscribe(enum_t const sig) const noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    Q_REQUIRE_ID(300, (Q_USER_SIG <= sig)
              && (sig < maxPubSignal_)
              && (0U < p) && (p <= QF_MAX_ACTIVE)
              && (registry_[p] == this));

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_SUBSCRIBE, m_prio)
        QS_TIME_PRE_();    // timestamp
        QS_SIG_PRE_(sig);  // the signal of this event
        QS_OBJ_PRE_(this); // this active object
    QS_END_NOCRIT_PRE_()

    subscrList_[sig].insert(p); // insert into subscriber-list
    QF_CRIT_X_();
}

} // namespace QP
//$enddef${QF::QActive::subscribe} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QActive::unsubscribe} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::unsubscribe} ................................................
void QActive::unsubscribe(enum_t const sig) const noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    //! @pre the signal and the priority must be in range, the AO must also
    // be registered with the framework
    Q_REQUIRE_ID(400, (Q_USER_SIG <= sig)
                      && (sig < maxPubSignal_)
                      && (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (registry_[p] == this));

    QF_CRIT_STAT_
    QF_CRIT_E_();

    QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_UNSUBSCRIBE, m_prio)
        QS_TIME_PRE_();         // timestamp
        QS_SIG_PRE_(sig);       // the signal of this event
        QS_OBJ_PRE_(this);      // this active object
    QS_END_NOCRIT_PRE_()

    subscrList_[sig].rmove(p); // remove from subscriber-list

    QF_CRIT_X_();
}

} // namespace QP
//$enddef${QF::QActive::unsubscribe} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QActive::unsubscribeAll} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::unsubscribeAll} .............................................
void QActive::unsubscribeAll() const noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    Q_REQUIRE_ID(500, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (registry_[p] == this));

    for (enum_t sig = Q_USER_SIG; sig < maxPubSignal_; ++sig) {
        QF_CRIT_STAT_
        QF_CRIT_E_();
        if (subscrList_[sig].hasElement(p)) {
            subscrList_[sig].rmove(p);

            QS_BEGIN_NOCRIT_PRE_(QS_QF_ACTIVE_UNSUBSCRIBE, m_prio)
                QS_TIME_PRE_();     // timestamp
                QS_SIG_PRE_(sig);   // the signal of this event
                QS_OBJ_PRE_(this);  // this active object
            QS_END_NOCRIT_PRE_()

        }
        QF_CRIT_X_();

        // prevent merging critical sections
        QF_CRIT_EXIT_NOP();
    }
}

} // namespace QP
//$enddef${QF::QActive::unsubscribeAll} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
