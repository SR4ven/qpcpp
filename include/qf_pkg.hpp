//$file${include::qf_pkg.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: qpcpp.qm
// File:  ${include::qf_pkg.hpp}
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
//$endhead${include::qf_pkg.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//! @file
//! @brief Internal (package scope) QF/C++ interface.

#ifndef QP_INC_QF_PKG_HPP_
#define QP_INC_QF_PKG_HPP_

//============================================================================
//! helper macro to cast const away from an event pointer
#define QF_CONST_CAST_(type_, ptr_) const_cast<type_>(ptr_)

//$declare${QF::QF-pkg} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
namespace QP {
namespace QF {

//${QF::QF-pkg::readySet_} ...................................................
//! "Ready-set" of all threads used in the built-in kernels
extern QPSet readySet_;

//${QF::QF-pkg::ePool_[QF_MAX_EPOOL]} ........................................
//! event pools managed by QF
#if (QF_MAX_EPOOL > 0U)
extern QF_EPOOL_TYPE_ ePool_[QF_MAX_EPOOL];
#endif //  (QF_MAX_EPOOL > 0U)

//${QF::QF-pkg::maxPool_} ....................................................
//! number of initialized event pools
extern std::uint_fast8_t maxPool_;

//${QF::QF-pkg::bzero} .......................................................
//! Clear a specified region of memory to zero
//!
//! @details
//! Clears a memory buffer by writing zeros byte-by-byte.
//!
//! @param[in] start  pointer to the beginning of a memory buffer.
//! @param[in] len    length of the memory buffer to clear (in bytes)
//!
//! @note The main application of this function is clearing the internal
//! QF variables upon startup. This is done to avoid problems with
//! non-standard startup code provided with some compilers and toolchains
//! (e.g., TI DSPs or Microchip MPLAB), which does not zero the
//! uninitialized variables, as required by the C++ standard.
void bzero(
    void * const start,
    std::uint_fast16_t const len) noexcept;

} // namespace QF
} // namespace QP
//$enddecl${QF::QF-pkg} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//============================================================================
namespace QP {

//............................................................................
//! Structure representing a free block in the Native QF Memory Pool
//! @sa QP::QMPool
struct QFreeBlock {
    QFreeBlock * volatile m_next; //!< link to the next free block
};

//............................................................................
// The following flags and flags and bitmasks are for the fields of the
// `QEvt.refCtr_` attribute of the QP::QTimeEvt class (subclass of QP::QEvt).
// This attribute is NOT used for reference counting in time events
// because the `QEvt.poolId_` attribute is zero ("static event").
//
constexpr std::uint8_t TE_IS_LINKED    = 1U << 7U;  // flag
constexpr std::uint8_t TE_WAS_DISARMED = 1U << 6U;  // flag
constexpr std::uint8_t TE_TICK_RATE    = 0x0FU;     // bitmask

// internal helper inline functions

//! return the Pool-ID of an event `e`
inline std::uint8_t QF_EVT_POOL_ID_ (QEvt const * const e) noexcept {
    return e->poolId_;
}

//! return the Reference Conter of an event `e`
inline std::uint8_t QF_EVT_REF_CTR_ (QEvt const * const e) noexcept {
    return e->refCtr_;
}

//! increment the refCtr_ of an event `e`
inline void QF_EVT_REF_CTR_INC_(QEvt const * const e) noexcept {
    (QF_CONST_CAST_(QEvt*, e))->refCtr_ = e->refCtr_ + 1U;
}

//! decrement the refCtr_ of an event `e`
inline void QF_EVT_REF_CTR_DEC_(QEvt const * const e) noexcept {
    (QF_CONST_CAST_(QEvt*, e))->refCtr_ = e->refCtr_ - 1U;
}

} // namespace QP

//============================================================================
// QF-specific critical section...
#ifndef QF_CRIT_STAT_TYPE
    //! This is an internal macro for defining the critical section
    //! status type.
    //!
    //! @details
    //! The purpose of this macro is to enable writing the same code for the
    //! case when critical section status type is defined and when it is not.
    //! If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    //! provides the definition of the critical section status variable.
    //! Otherwise this macro is empty.
    //! @sa #QF_CRIT_STAT_TYPE
    #define QF_CRIT_STAT_

    //! This is an internal macro for entering a critical section.
    //!
    //! @details
    //! The purpose of this macro is to enable writing the same code for the
    //! case when critical section status type is defined and when it is not.
    //! If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    //! invokes QF_CRIT_ENTRY() passing the key variable as the parameter.
    //! Otherwise QF_CRIT_ENTRY() is invoked with a dummy parameter.
    //! @sa QF_CRIT_ENTRY()
    #define QF_CRIT_E_()        QF_CRIT_ENTRY(dummy)

    //! This is an internal macro for exiting a critical section.
    //!
    //! @details
    //! The purpose of this macro is to enable writing the same code for the
    //! case when critical section status type is defined and when it is not.
    //! If the macro #QF_CRIT_STAT_TYPE is defined, this internal macro
    //! invokes QF_CRIT_EXIT() passing the key variable as the parameter.
    //! Otherwise QF_CRIT_EXIT() is invoked with a dummy parameter.
    //! @sa QF_CRIT_EXIT()
    //!
    #define QF_CRIT_X_()        QF_CRIT_EXIT(dummy)

#elif (!defined QF_CRIT_STAT_)
    #define QF_CRIT_STAT_       QF_CRIT_STAT_TYPE critStat_;
    #define QF_CRIT_E_()        QF_CRIT_ENTRY(critStat_)
    #define QF_CRIT_X_()        QF_CRIT_EXIT(critStat_)
#endif  // QF_CRIT_STAT_TYPE

// Assertions inside the critical section ------------------------------------
#ifdef Q_NASSERT // Q_NASSERT defined--assertion checking disabled

    #define Q_ASSERT_CRIT_(id_, test_)  static_cast<void>(0)
    #define Q_REQUIRE_CRIT_(id_, test_) static_cast<void>(0)
    #define Q_ERROR_CRIT_(id_)          static_cast<void>(0)

#else  // Q_NASSERT not defined--assertion checking enabled

    #define Q_ASSERT_CRIT_(id_, test_) do {                          \
        if ((test_)) {} else {                                       \
            QF_CRIT_X_();                                            \
            Q_onAssert(&Q_this_module_[0], static_cast<int_t>(id_)); \
        } \
    } while (false)

    #define Q_REQUIRE_CRIT_(id_, test_) Q_ASSERT_CRIT_((id_), (test_))

    #define Q_ERROR_CRIT_(id_) do {                                  \
        QF_CRIT_X_();                                                \
        Q_onAssert(&Q_this_module_[0], static_cast<int_t>(id_));     \
    } while (false)

#endif // Q_NASSERT

//! helper macro to test that a pointer `x_` is in range between
//! `min_` and `max_`
//!
//! @details
//! This macro is specifically and exclusively used for checking the range
//! of a block pointer returned to the pool. Such a check must rely on the
//! pointer arithmetic not compliant with the [AUTOSAR Rule M5-0-18].
//! Defining a specific macro for this purpose allows to selectively
//! disable the warnings for this particular case.
#define QF_PTR_RANGE_(x_, min_, max_)  (((min_) <= (x_)) && ((x_) <= (max_)))

#endif // QP_INC_QF_PKG_HPP_
