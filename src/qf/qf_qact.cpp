//$file${src::qf::qf_qact.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${src::qf::qf_qact.cpp}
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
//$endhead${src::qf::qf_qact.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @file
//! @brief QP::QActive services and QF support code

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

//============================================================================
namespace { // unnamed local namespace
Q_DEFINE_THIS_MODULE("qf_qact")
} // unnamed namespace

//============================================================================
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${QF::QActive::registry_[QF_MAX_ACTIVE + 1U]} vvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
QActive * QActive::registry_[QF_MAX_ACTIVE + 1U];

} // namespace QP
//$enddef${QF::QActive::registry_[QF_MAX_ACTIVE + 1U]} ^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QF-base::intNest_} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QF {

//${QF::QF-base::intNest_} ...................................................
std::uint_fast8_t intNest_;

} // namespace QF
} // namespace QP
//$enddef${QF::QF-base::intNest_} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QF-pkg::readySet_} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QF {

//${QF::QF-pkg::readySet_} ...................................................
QPSet readySet_;

} // namespace QF
} // namespace QP
//$enddef${QF::QF-pkg::readySet_} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QF-pkg::bzero} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QF {

//${QF::QF-pkg::bzero} .......................................................
void bzero(
    void * const start,
    std::uint_fast16_t const len) noexcept
{
    std::uint8_t *ptr = static_cast<std::uint8_t *>(start);
    for (std::uint_fast16_t n = len; n > 0U; --n) {
        *ptr = 0U;
        ++ptr;
    }
}

} // namespace QF
} // namespace QP
//$enddef${QF::QF-pkg::bzero} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//============================================================================
//$define${QF::QActive::QActive} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::QActive} ....................................................
QActive::QActive(QStateHandler const initial) noexcept
  : QHsm(initial),
    m_prio(0U),
    m_pthre(0U)
{
    #ifdef QF_EQUEUE_TYPE
        QF::bzero(&m_eQueue, sizeof(m_eQueue));
    #endif

    #ifdef QF_OS_OBJECT_TYPE
        QF::bzero(&m_osObject, sizeof(m_osObject));
    #endif

    #ifdef QF_THREAD_TYPE
        QF::bzero(&m_thread, sizeof(m_thread));
    #endif
}

} // namespace QP
//$enddef${QF::QActive::QActive} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QActive::register_} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::register_} ..................................................
void QActive::register_() noexcept {
    if (m_pthre == 0U) { // preemption-threshold not defined?
        m_pthre = m_prio; // apply the default
    }

    #ifndef Q_NASSERT

    //! @pre
    //! 1. the "QF-priority" of the AO must be in range
    //! 2. the "QF-priority" must not be already in use (unique priority)
    //! 3. the "QF-priority" must not exceed the "preemption-threshold"
    Q_REQUIRE_ID(100, (0U < m_prio) && (m_prio <= QF_MAX_ACTIVE)
                       && (registry_[m_prio] == nullptr)
                       && (m_prio <= m_pthre));

    std::uint8_t prev_thre = m_pthre;
    std::uint8_t next_thre = m_pthre;
    std::uint_fast8_t p;

    for (p = static_cast<std::uint_fast8_t>(m_prio) - 1U; p > 0U; --p) {
        if (registry_[p] != nullptr) {
            prev_thre = registry_[p]->m_pthre;
            break;
        }
    }
    for (p = static_cast<std::uint_fast8_t>(m_prio) + 1U;
         p <= QF_MAX_ACTIVE; ++p)
    {
        if (registry_[p] != nullptr) {
            next_thre = registry_[p]->m_pthre;
            break;
        }
    }

    //! @post
    //! 1. the preceding pre-thre must not exceed the preemption-threshold
    //! 2. the preemption-threshold must not exceed the next pre-thre
    Q_ENSURE_ID(110, (prev_thre <= m_pthre) && (m_pthre <= next_thre));
    #endif // Q_NASSERT

    QF_CRIT_STAT_
    QF_CRIT_E_();
    // register the AO at the "QF-priority"
    registry_[m_prio] = this;
    QF_CRIT_X_();
}

} // namespace QP
//$enddef${QF::QActive::register_} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${QF::QActive::unregister_} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF::QActive::unregister_} ................................................
void QActive::unregister_() noexcept {
    std::uint_fast8_t const p = static_cast<std::uint_fast8_t>(m_prio);

    Q_REQUIRE_ID(200, (0U < p) && (p <= QF_MAX_ACTIVE)
                      && (registry_[p] == this));
    QF_CRIT_STAT_
    QF_CRIT_E_();
    registry_[p] = nullptr; // free-up the priority level
    m_state.fun = nullptr; // invalidate the state
    QF_CRIT_X_();
}

} // namespace QP
//$enddef${QF::QActive::unregister_} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//============================================================================
//$define${QF-types::QPSet::QF_LOG2} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {

//${QF-types::QPSet::QF_LOG2} ................................................
#ifndef QF_LOG2
std::uint_fast8_t QPSet::QF_LOG2(QP::QPSetBits x) const noexcept {
    static std::uint8_t const log2LUT[16] = {
        0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U,
        4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U
    };
    std::uint_fast8_t n = 0U;
    QP::QPSetBits t;

    #if (QF_MAX_ACTIVE > 16U)
    t = static_cast<QP::QPSetBits>(x >> 16U);
    if (t != 0U) {
        n += 16U;
        x = t;
    }
    #endif
    #if (QF_MAX_ACTIVE > 8U)
    t = (x >> 8U);
    if (t != 0U) {
        n += 8U;
        x = t;
    }
    #endif
    t = (x >> 4U);
    if (t != 0U) {
        n += 4U;
        x = t;
    }
    return n + log2LUT[x];
}

#endif // ndef QF_LOG2

} // namespace QP
//$enddef${QF-types::QPSet::QF_LOG2} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
