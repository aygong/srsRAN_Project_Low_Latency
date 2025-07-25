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

#include "srsran/ru/ru_generic_configuration.h"

namespace srsran {

namespace srs_du {
struct du_cell_config;
}

struct ru_sdr_unit_config;
struct worker_manager_config;

/// Converts and returns the given RU SDR application unit configuration to a SDR RU configuration.
ru_generic_configuration generate_ru_sdr_config(const ru_sdr_unit_config&          ru_cfg,
                                                span<const srs_du::du_cell_config> du_cells,
                                                // ################################################################################ //
                                                unsigned                           integer_processing_delay_slots,
                                                float                              decimal_processing_delay_slots,
                                                unsigned                           radio_heads_prep_time);
                                                // ################################################################################ //

/// Fills the SDR worker manager parameters of the given worker manager configuration.
void fill_sdr_worker_manager_config(worker_manager_config& config, const ru_sdr_unit_config& ru_cfg);

} // namespace srsran

