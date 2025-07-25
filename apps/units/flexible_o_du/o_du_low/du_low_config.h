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

#include "apps/services/worker_manager/os_sched_affinity_manager.h"
#include <optional>
#include <string>
#include <vector>

namespace srsran {

/// Expert upper physical layer configuration.
struct du_low_unit_expert_upper_phy_config {
  // ################################################################################ //
  /// \brief Sets the maximum allowed downlink processing delay in slots.
  ///
  /// Higher values increase the downlink processing pipeline length, which improves performance and stability for
  /// demanding cell configurations, such as using large bandwidths or higher order MIMO. Higher values also increase
  /// the round trip latency of the radio link.
  float max_processing_delay_slots = 5.0F;
  // ################################################################################ //
  /// Number of PUSCH LDPC decoder iterations.
  unsigned pusch_decoder_max_iterations = 6;
  /// Set to true to enable the PUSCH LDPC decoder early stop.
  bool pusch_decoder_early_stop = true;
  /// \brief Selects a PUSCH SINR calculation method.
  ///
  /// Available methods:
  /// -\c channel_estimator: SINR is calculated by the channel estimator using the DM-RS.
  /// -\c post_equalization: SINR is calculated using the post-equalization noise variances of the equalized RE.
  /// -\c evm: SINR is obtained from the EVM of the PUSCH symbols.
  std::string pusch_sinr_calc_method = "post_equalization";
  /// \brief Request headroom size in slots.
  ///
  /// The request headroom size is the number of delayed slots that the upper physical layer will accept, ie, if the
  /// current slot is M, the upper phy will consider the slot M - nof_slots_request_headroom as valid and process it.
  unsigned nof_slots_request_headroom = 0U;
  
  // ################################################################################ //
  /// \brief Sets the maximum allowed downlink processing delay in slots (integer part).
  unsigned integer_processing_delay_slots = 5U;
  /// \brief Sets the maximum allowed downlink processing delay in slots (decimal part).
  float decimal_processing_delay_slots = 0.0F;
  /// \brief Sets the maximum allowed preparation time for radio heads.
  unsigned radio_heads_prep_time = 3U;
  // ################################################################################ //
};

/// DU low logging functionalities.
struct du_low_unit_logger_config {
  srslog::basic_levels phy_level = srslog::basic_levels::warning;
  srslog::basic_levels hal_level = srslog::basic_levels::warning;
  /// Set to true to log broadcasting messages and all PRACH opportunities.
  bool broadcast_enabled = false;
  /// Maximum number of bytes to write when dumping hex arrays.
  int hex_max_size = 0;
  /// Set to a valid file path to print the received symbols.
  std::string phy_rx_symbols_filename;
  /// Set to a valid Rx port number or empty for all ports.
  std::optional<unsigned> phy_rx_symbols_port = 0;
  /// If true, prints the PRACH frequency-domain symbols.
  bool phy_rx_symbols_prach = false;
};

/// CPU affinities configuration for the cell.
struct du_low_unit_cpu_affinities_cell_config {
  /// L1 uplink CPU affinity mask.
  os_sched_affinity_config l1_ul_cpu_cfg = {sched_affinity_mask_types::l1_ul, {}, sched_affinity_mask_policy::mask};
  /// L1 downlink workers CPU affinity mask.
  os_sched_affinity_config l1_dl_cpu_cfg = {sched_affinity_mask_types::l1_dl, {}, sched_affinity_mask_policy::mask};
};

/// Expert threads configuration of the gNB app.
struct du_low_unit_expert_threads_config {
  du_low_unit_expert_threads_config()
  {
    unsigned nof_threads = cpu_architecture_info::get().get_host_nof_available_cpus();

    if (nof_threads < 4) {
      nof_ul_threads            = 1;
      nof_pusch_decoder_threads = 0;
      nof_dl_threads            = 2;
    } else if (nof_threads < 8) {
      nof_ul_threads            = 1;
      nof_pusch_decoder_threads = 1;
      nof_dl_threads            = 4;
    } else if (nof_threads < 16) {
      nof_ul_threads            = 1;
      nof_pusch_decoder_threads = 1;
      nof_dl_threads            = 4;
    } else {
      nof_ul_threads            = 2;
      nof_pusch_decoder_threads = 2;
      nof_dl_threads            = 6;
    }
  }

  /// \brief PDSCH processor type.
  ///
  /// Use of there options:
  /// - \c automatic: selects \c lite implementation if \c nof_pdsch_threads is one, otherwise \c concurrent, or
  /// - \c generic: for using unoptimized PDSCH processing, or
  /// - \c concurrent: for using a processor that processes code blocks in parallel, or
  /// - \c lite: for using a memory optimized processor.
  std::string pdsch_processor_type = "auto";
  /// \brief Number of threads for concurrent PUSCH decoding.
  ///
  /// If the number of PUSCH decoder threads is greater than zero, the PUSCH decoder will enqueue received soft bits and
  /// process them asynchronously. Otherwise, PUSCH decoding will be performed synchronously.
  ///
  /// In non-real-time operations (e.g., when using ZeroMQ), setting this parameter to a non-zero value can potentially
  /// introduce delays in uplink HARQ feedback.
  unsigned nof_pusch_decoder_threads = 0;
  /// Number of threads for processing PUSCH and PUCCH.
  unsigned nof_ul_threads = 1;
  /// Number of threads for processing PDSCH, PDCCH, NZP CSI-RS and SSB. It is set to 1 by default.
  unsigned nof_dl_threads = 1;
};

/// Expert configuration of the gNB app.
struct du_low_unit_expert_execution_config {
  /// Expert thread configuration of the gNB app.
  du_low_unit_expert_threads_config threads;
  /// \brief CPU affinities per cell of the gNB app.
  ///
  /// \note Add one cell by default.
  std::vector<du_low_unit_cpu_affinities_cell_config> cell_affinities = {{}};
};

/// Hardware-accelerated PDSCH encoder configuration of the DU low.
struct hwacc_pdsch_appconfig {
  /// \brief Number of hardware-accelerated PDSCH encoding functions.
  unsigned nof_hwacc;
  /// \brief Operation mode of the PDSCH encoder (CB = true, TB = false [default]).
  bool cb_mode = false;
  /// \brief Maximum supported buffer size in bytes (CB mode will be forced for larger TBs). Only used in TB mode to
  /// size the mbufs.
  ///
  /// Set to the maximum supported size by default.
  std::optional<unsigned> max_buffer_size;
  /// \brief Type of hardware queue usage (dedicated = true [default], shared = false). In case of a shared usage, the
  /// accelerated function needs to reseve a hardware-queue for each operation.
  bool dedicated_queue = true;
};

/// Hardware-accelerated PUSCH decoder configuration of the DU low.
struct hwacc_pusch_appconfig {
  /// \brief Number of hardware-accelerated PUSCH decoding functions.
  unsigned nof_hwacc;
  /// \brief Defines if the soft-buffer is implemented in the accelerator (true [default]) or not (false).
  bool ext_softbuffer = true;
  /// \brief Size of the HARQ context repository.
  ///
  /// Set to the maximum number of CBs supported by the gNB config by default.
  std::optional<unsigned> harq_context_size;
  /// \brief Type of hardware queue usage (dedicated = true [default], shared = false). In case of a shared usage, the
  /// accelerated function needs to reseve a hardware-queue for each operation.
  bool dedicated_queue = true;
};

/// BBDEV configuration of the DU low.
struct bbdev_appconfig {
  /// \brief Type of BBDEV hardware-accelerator.
  std::string hwacc_type;
  /// \brief ID of the BBDEV-based hardware-accelerator.
  unsigned id;
  /// \brief Structure providing the configuration of hardware-accelerated PDSCH encoding functions.
  std::optional<hwacc_pdsch_appconfig> pdsch_enc;
  /// \brief Structure providing the configuration of hardware-accelerated PUSCH decoding functions.
  std::optional<hwacc_pusch_appconfig> pusch_dec;
  /// \brief Size (in bytes) of each DPDK memory buffer (mbuf) used to exchange unencoded and unrate-matched messages
  /// with the accelerator.
  ///
  /// Set to the maximum supported size by default.
  std::optional<unsigned> msg_mbuf_size;
  /// \brief Size (in bytes) of each DPDK memory buffer (mbuf) used to exchange encoded and rate-matched messages with
  /// the accelerator.
  ///
  /// Set to the maximum supported size by default.
  std::optional<unsigned> rm_mbuf_size;
  /// \brief Number of DPDK memory buffers (mbufs) in each memory pool.
  ///
  /// Set to the maximum number of CBs supported by the gNB config by default.
  std::optional<unsigned> nof_mbuf;
};

// HAL configuration of the DU low.
struct du_low_unit_hal_config {
  /// BBDEV-based hardware-accelerator arguments.
  std::optional<bbdev_appconfig> bbdev_hwacc;
};

/// DU low configuration.
struct du_low_unit_config {
  /// Loggers.
  du_low_unit_logger_config loggers;
  /// Expert physical layer configuration.
  du_low_unit_expert_upper_phy_config expert_phy_cfg;
  /// Expert execution parameters for the DU low.
  du_low_unit_expert_execution_config expert_execution_cfg;
  /// HAL configuration.
  std::optional<du_low_unit_hal_config> hal_config;
};

} // namespace srsran
