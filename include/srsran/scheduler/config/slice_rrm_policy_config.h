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

#include "srsran/ran/rrm.h"
#include "srsran/ran/s_nssai.h"
#include "srsran/scheduler/config/scheduler_expert_config.h"

namespace srsran {

/// Cell-specific Default RAN slice configuration.
struct slice_rrm_policy_config {
  /// RRM Policy identifier.
  rrm_policy_member rrc_member;
  /// Sets the minimum number of PRBs to be allocated to this group.
  unsigned min_prb = 0;
  /// Sets the maximum number of PRBs to be allocated to this group.
  unsigned max_prb = MAX_NOF_PRBS;
  /// Policy scheduler configuration for the slice.
  policy_scheduler_expert_config policy_sched_cfg = time_rr_scheduler_expert_config{};
  // ################################################################################ //
  /// Enables SR-free access to this group.
  bool sr_free_access_enable = false;
  /// Specifies the minimum size (in bytes) of uplink grants assigned 
  /// to this group when SR-free access is enabled.
  unsigned min_ul_grant_size = 0U;
  // ################################################################################ //
};

} // namespace srsran
