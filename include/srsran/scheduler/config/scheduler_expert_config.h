/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#pragma once

/// \file
/// \brief Configuration structs passed to scheduler implementation.

#include "srsran/adt/interval.h"
#include "srsran/ran/direct_current_offset.h"
#include "srsran/ran/pdcch/aggregation_level.h"
#include "srsran/ran/pdsch/pdsch_mcs.h"
#include "srsran/ran/resource_block.h"
#include "srsran/ran/sch/sch_mcs.h"
#include "srsran/ran/sib/sib_configuration.h"
#include "srsran/ran/slot_pdu_capacity_constants.h"
#include <chrono>
#include <variant>
#include <vector>

namespace srsran {

/// \brief Proportional fair policy scheduler expert parameters.
struct time_pf_scheduler_expert_config {
  /// Fairness Coefficient to use in Proportional Fair policy scheduler.
  double pf_sched_fairness_coeff = 2.0F;
};

/// \brief Round-Robin policy scheduler expert parameters.
struct time_rr_scheduler_expert_config {};

/// \brief Policy scheduler expert parameters.
using policy_scheduler_expert_config = std::variant<time_rr_scheduler_expert_config, time_pf_scheduler_expert_config>;

struct ul_power_control {
  /// Enable closed-loop PUSCH power control.
  bool enable_pusch_cl_pw_control = false;
  /// Target PUSCH SINR to be achieved with Close-loop power control, in dB.
  /// Only relevant if \c enable_closed_loop_pw_control is set to true.
  float target_pusch_sinr{10.0f};
  /// Path-loss at which the Target PUSCH SINR is expected to be achieved, in dB.
  /// This is used to compute the path loss compensation for PUSCH fractional power control.
  /// Only relevant if \c enable_closed_loop_pw_control is set to true.
  float path_loss_for_target_pusch_sinr{70.0f};
};

/// \brief UE scheduling statically configurable expert parameters.
struct scheduler_ue_expert_config {
  /// Range of allowed MCS indices for DL UE scheduling. To use a fixed mcs, set the minimum mcs equal to the maximum.
  interval<sch_mcs_index, true> dl_mcs;
  /// Sequence of redundancy versions used for PDSCH scheduling. Possible values: {0, 1, 2, 3}.
  std::vector<uint8_t> pdsch_rv_sequence;
  /// Range of allowed MCS indices for UL UE scheduling. To use a fixed mcs, set the minimum mcs equal to the maximum.
  interval<sch_mcs_index, true> ul_mcs;
  /// Sequence of redundancy versions used for PUSCH scheduling. Possible values: {0, 1, 2, 3}.
  std::vector<uint8_t> pusch_rv_sequence;
  unsigned             initial_cqi;
  unsigned             max_nof_harq_retxs;
  /// Maximum MCS index that can be assigned when scheduling MSG4.
  sch_mcs_index max_msg4_mcs;
  /// Initial UL SINR value used for Dynamic UL MCS computation (in dB).
  double initial_ul_sinr;
  /// Enable multiplexing of CSI-RS and PDSCH.
  bool enable_csi_rs_pdsch_multiplexing;
  /// Set boundaries, in number of RBs, for UE PDSCH grants.
  interval<unsigned> pdsch_nof_rbs{1, MAX_NOF_PRBS};
  /// Set boundaries, in number of RBs, for UE PUSCH grants.
  interval<unsigned> pusch_nof_rbs{1, MAX_NOF_PRBS};
  /// \defgroup ta_manager_params
  /// \brief Time Advance (TA) manager parameters.
  ///
  /// These parameters define the behaviour of the Time Advance manager and on how the Time Advance Command (\f$T_A\f$)
  /// is triggered.
  ///
  /// The TA measurement is reported from the physical layer, averaged over a \ref ta_measurement_slot_period and
  /// outliers are filtered out. The final estimated TA is rounded to the nearest TA unit.
  ///
  /// \remark T_A is defined in TS 38.213, clause 4.2.
  /// @{
  /// Measurements periodicity in nof. slots over which the new Timing Advance Command is computed.
  unsigned ta_measurement_slot_period{80};
  /// \brief Timing Advance Command (T_A) offset threshold.
  ///
  /// A TA command is triggered if the estimated TA is equal to or greater than this threshold. Possible valid values
  /// are {0,...,32}.
  ///
  /// If set to less than zero, issuing of TA Command is disabled.
  int8_t ta_cmd_offset_threshold;
  /// \brief Timing Advance target in units of TA.
  ///
  /// Offsets the target TA measurements so the signal from the UE is kept delayed. This parameter is useful for
  /// avoiding negative TA when the UE is getting away.
  float ta_target;
  /// UL SINR threshold (in dB) above which reported N_TA update measurement is considered valid.
  float ta_update_measurement_ul_sinr_threshold;
  /// @}
  /// Direct Current (DC) offset, in number of subcarriers, used in PUSCH, by default. The gNB may supersede this DC
  /// offset value through RRC messaging. See TS38.331 - "txDirectCurrentLocation".
  dc_offset_t initial_ul_dc_offset{dc_offset_t::center};
  /// Maximum number of PDSCH grants per slot.
  unsigned max_pdschs_per_slot = MAX_PDSCH_PDUS_PER_SLOT;
  /// Maximum number of PUSCH grants per slot.
  unsigned max_puschs_per_slot = MAX_PUSCH_PDUS_PER_SLOT;
  /// Maximum number of PUCCH grants per slot.
  unsigned max_pucchs_per_slot{31U};
  /// Maximum number of PUSCH + PUCCH grants per slot.
  unsigned max_ul_grants_per_slot{32U};
  /// Possible values: {1, ..., 7}.
  // [Implementation-defined] As min_k1 is used for both common and dedicated PUCCH configuration, and in the UE
  // fallback scheduler only allow max k1 = 7, we restrict min_k1 to 7.
  uint8_t min_k1 = 4;
  /// Maximum number of PDCCH grant allocation attempts per slot. Default: Unlimited.
  unsigned max_pdcch_alloc_attempts_per_slot = std::max(MAX_DL_PDCCH_PDUS_PER_SLOT, MAX_UL_PDCCH_PDUS_PER_SLOT);
  /// CQI offset increment used in outer loop link adaptation (OLLA) algorithm. If set to zero, OLLA is disabled.
  float olla_cqi_inc{0.001};
  /// DL Target BLER to be achieved with OLLA.
  float olla_dl_target_bler{0.01};
  /// Maximum CQI offset that the OLLA algorithm can apply to the reported CQI.
  float olla_max_cqi_offset{4.0};
  /// UL SNR offset increment in dB used in OLLA algorithm. If set to zero, OLLA is disabled.
  float olla_ul_snr_inc{0.001};
  /// UL Target BLER to be achieved with OLLA.
  float olla_ul_target_bler{0.01};
  /// Maximum UL SNR offset that the OLLA algorithm can apply on top of the estimated UL SINR.
  float olla_max_ul_snr_offset{5.0};
  /// Threshold for drop in CQI of the first HARQ transmission above which HARQ retransmissions are cancelled.
  uint8_t dl_harq_la_cqi_drop_threshold{2};
  /// Threshold for drop in nof. layers of the first HARQ transmission above which HARQ retransmission is cancelled.
  uint8_t dl_harq_la_ri_drop_threshold{1};
  /// Automatic HARQ acknowledgement (used for NTN cases with no HARQ feedback)
  bool auto_ack_harq{false};

  /// Boundaries in RB interval for resource allocation of UE PDSCHs.
  crb_interval pdsch_crb_limits{0, MAX_NOF_PRBS};
  /// Boundaries in RB interval for resource allocation of UE PUSCHs.
  crb_interval pusch_crb_limits{0, MAX_NOF_PRBS};
  /// Expert parameters to be passed to the policy scheduler.
  policy_scheduler_expert_config strategy_cfg = time_rr_scheduler_expert_config{};
  /// Expert PUCCH/PUSCH power control parameters.
  ul_power_control ul_power_ctrl = ul_power_control{};
};

/// \brief System Information scheduling statically configurable expert parameters.
struct scheduler_si_expert_config {
  /// As per TS 38.214, Section 5.1.3.1, only an MCS with modulation order 2 allowed for SIB1.
  sch_mcs_index     sib1_mcs_index          = 5;
  aggregation_level sib1_dci_aggr_lev       = aggregation_level::n4;
  sch_mcs_index     si_message_mcs_index    = 5;
  aggregation_level si_message_dci_aggr_lev = aggregation_level::n4;
  /// SIB1 retx period.
  sib1_rtx_periodicity sib1_retx_period = sib1_rtx_periodicity::ms160;
};

/// \brief Random Access scheduling statically configurable expert parameters.
struct scheduler_ra_expert_config {
  sch_mcs_index rar_mcs_index;
  sch_mcs_index msg3_mcs_index;
  unsigned      max_nof_msg3_harq_retxs;
};

/// \brief Paging scheduling statically configurable expert parameters.
struct scheduler_paging_expert_config {
  /// As per TS 38.214, Section 5.1.3.1, only an MCS with modulation order 2 allowed for Paging.
  sch_mcs_index     paging_mcs_index    = 5;
  aggregation_level paging_dci_aggr_lev = aggregation_level::n4;
  unsigned          max_paging_retries  = 2;
};

/// \brief Scheduling statically configurable expert parameters.
struct scheduler_expert_config {
  scheduler_si_expert_config     si;
  scheduler_ra_expert_config     ra;
  scheduler_paging_expert_config pg;
  scheduler_ue_expert_config     ue;
  bool                           log_broadcast_messages       = false;
  bool                           log_high_latency_diagnostics = false;
  std::chrono::milliseconds      metrics_report_period{1000};
};

} // namespace srsran
