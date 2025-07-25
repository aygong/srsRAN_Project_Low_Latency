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

#include "du_low_config_cli11_schema.h"
#include "apps/services/logger/logger_appconfig_cli11_utils.h"
#include "apps/services/worker_manager/cli11_cpu_affinities_parser_helper.h"
#include "du_low_config.h"
#include "srsran/adt/expected.h"
#include "srsran/support/cli11_utils.h"
#include "srsran/support/config_parsers.h"

using namespace srsran;

template <typename Integer>
static expected<Integer, std::string> parse_int(const std::string& value)
{
  try {
    return std::stoi(value);
  } catch (const std::invalid_argument& e) {
    return make_unexpected(e.what());
  } catch (const std::out_of_range& e) {
    return make_unexpected(e.what());
  }
}

static void configure_cli11_log_args(CLI::App& app, du_low_unit_logger_config& log_params)
{
  app_services::add_log_option(app, log_params.phy_level, "--phy_level", "PHY log level");
  app_services::add_log_option(app, log_params.hal_level, "--hal_level", "HAL log level");

  add_option(app,
             "--broadcast_enabled",
             log_params.broadcast_enabled,
             "Enable logging in the physical and MAC layer of broadcast messages and all PRACH opportunities")
      ->always_capture_default();
  app.add_option("--phy_rx_symbols_filename",
                 log_params.phy_rx_symbols_filename,
                 "Set to a valid file path to print the received symbols.")
      ->always_capture_default();
  app.add_option_function<std::string>(
         "--phy_rx_symbols_port",
         [&log_params](const std::string& value) {
           if (value == "all") {
             log_params.phy_rx_symbols_port = std::nullopt;
           } else {
             log_params.phy_rx_symbols_port = parse_int<unsigned>(value).value();
           }
         },
         "Set to a valid receive port number to dump the IQ symbols from that port only, or set to \"all\" to dump the "
         "IQ symbols from all UL receive ports. Only works if \"phy_rx_symbols_filename\" is set.")
      ->default_str("0")
      ->check(CLI::NonNegativeNumber | CLI::IsMember({"all"}));
  app.add_option("--phy_rx_symbols_prach",
                 log_params.phy_rx_symbols_prach,
                 "Set to true to dump the IQ symbols from all the PRACH ports. Only works if "
                 "\"phy_rx_symbols_filename\" is set.")
      ->capture_default_str();

  add_option(
      app, "--hex_max_size", log_params.hex_max_size, "Maximum number of bytes to print in hex (zero for no hex dumps)")
      ->capture_default_str()
      ->check(CLI::Range(0, 1024));
}

static void configure_cli11_cell_affinity_args(CLI::App& app, du_low_unit_cpu_affinities_cell_config& config)
{
  add_option_function<std::string>(
      app,
      "--l1_dl_cpus",
      [&config](const std::string& value) { parse_affinity_mask(config.l1_dl_cpu_cfg.mask, value, "l1_dl_cpus"); },
      "CPU cores assigned to L1 downlink tasks");

  add_option_function<std::string>(
      app,
      "--l1_ul_cpus",
      [&config](const std::string& value) { parse_affinity_mask(config.l1_ul_cpu_cfg.mask, value, "l1_ul_cpus"); },
      "CPU cores assigned to L1 uplink tasks");

  add_option_function<std::string>(
      app,
      "--l1_dl_pinning",
      [&config](const std::string& value) {
        config.l1_dl_cpu_cfg.pinning_policy = to_affinity_mask_policy(value);
        if (config.l1_dl_cpu_cfg.pinning_policy == sched_affinity_mask_policy::last) {
          report_error("Incorrect value={} used in {} property", value, "l1_dl_pinning");
        }
      },
      "Policy used for assigning CPU cores to L1 downlink tasks");

  add_option_function<std::string>(
      app,
      "--l1_ul_pinning",
      [&config](const std::string& value) {
        config.l1_ul_cpu_cfg.pinning_policy = to_affinity_mask_policy(value);
        if (config.l1_ul_cpu_cfg.pinning_policy == sched_affinity_mask_policy::last) {
          report_error("Incorrect value={} used in {} property", value, "l1_ul_pinning");
        }
      },
      "Policy used for assigning CPU cores to L1 uplink tasks");
}

static void configure_cli11_upper_phy_threads_args(CLI::App& app, du_low_unit_expert_threads_config& config)
{
  auto pdsch_processor_check = [](const std::string& value) -> std::string {
    if ((value == "auto") || (value == "generic") || (value == "concurrent") || (value == "lite")) {
      return {};
    }
    return "Invalid PDSCH processor type. Accepted values [auto,generic,concurrent,lite]";
  };

  add_option(app,
             "--pdsch_processor_type",
             config.pdsch_processor_type,
             "PDSCH processor type: auto, generic, concurrent and lite.")
      ->capture_default_str()
      ->check(pdsch_processor_check);
  add_option(app, "--nof_pusch_decoder_threads", config.nof_pusch_decoder_threads, "Number of threads to decode PUSCH.")
      ->capture_default_str()
      ->check(CLI::Number);
  add_option(app, "--nof_ul_threads", config.nof_ul_threads, "Number of upper PHY threads to process uplink.")
      ->capture_default_str()
      ->check(CLI::Number);
  add_option(app, "--nof_dl_threads", config.nof_dl_threads, "Number of upper PHY threads to process downlink.")
      ->capture_default_str()
      ->check(CLI::Number);
}

static void configure_cli11_expert_execution_args(CLI::App& app, du_low_unit_expert_execution_config& config)
{
  // Threads section.
  CLI::App* threads_subcmd = add_subcommand(app, "threads", "Threads configuration")->configurable();

  // Upper PHY threads.
  CLI::App* upper_phy_threads_subcmd =
      add_subcommand(*threads_subcmd, "upper_phy", "Upper PHY thread configuration")->configurable();
  configure_cli11_upper_phy_threads_args(*upper_phy_threads_subcmd, config.threads);

  // Cell affinity section.
  add_option_cell(
      app,
      "--cell_affinities",
      [&config](const std::vector<std::string>& values) {
        config.cell_affinities.resize(values.size());
        for (unsigned i = 0, e = values.size(); i != e; ++i) {
          CLI::App subapp("DU low expert execution cell CPU affinities",
                          "DU low expert execution cell CPU affinities config, item #" + std::to_string(i));
          subapp.config_formatter(create_yaml_config_parser());
          subapp.allow_config_extras();
          configure_cli11_cell_affinity_args(subapp, config.cell_affinities[i]);
          std::istringstream ss(values[i]);
          subapp.parse_from_stream(ss);
        }
      },
      "Sets the cell CPU affinities configuration on a per cell basis");
}

static void configure_cli11_expert_phy_args(CLI::App& app, du_low_unit_expert_upper_phy_config& expert_phy_params)
{
  auto pusch_sinr_method_check = [](const std::string& value) -> std::string {
    if ((value == "channel_estimator") || (value == "post_equalization") || (value == "evm")) {
      return {};
    }
    return "Invalid PUSCH SINR calculation method. Accepted values [channel_estimator,post_equalization,evm]";
  };

  // ################################################################################ //
  add_option(app,
             "--max_proc_delay",
             expert_phy_params.max_processing_delay_slots,
             "Maximum allowed DL processing delay in slots.")
      ->capture_default_str()
      ->check(CLI::Range(0.0, 30.0));
  // ################################################################################ //
  add_option(app,
             "--pusch_dec_max_iterations",
             expert_phy_params.pusch_decoder_max_iterations,
             "Maximum number of PUSCH LDPC decoder iterations")
      ->capture_default_str()
      ->check(CLI::Number);
  add_option(app,
             "--pusch_dec_enable_early_stop",
             expert_phy_params.pusch_decoder_early_stop,
             "Enables PUSCH LDPC decoder early stop")
      ->capture_default_str();
  add_option(app,
             "--pusch_sinr_calc_method",
             expert_phy_params.pusch_sinr_calc_method,
             "PUSCH SINR calculation method: channel_estimator, post_equalization and evm.")
      ->capture_default_str()
      ->check(pusch_sinr_method_check);
  add_option(app,
             "--max_request_headroom_slots",
             expert_phy_params.nof_slots_request_headroom,
             "Maximum request headroom size in slots.")
      ->capture_default_str()
      ->check(CLI::Range(0, 30));
  // ################################################################################ //
  add_option(app,
             "--radio_heads_prep_time",
             expert_phy_params.radio_heads_prep_time,
             "Maximum allowed preparation time for radio heads.")
      ->capture_default_str()
      ->check(CLI::Range(1, 30));
  // ################################################################################ //
}

static void configure_cli11_hwacc_pdsch_enc_args(CLI::App& app, std::optional<hwacc_pdsch_appconfig>& config)
{
  config.emplace();

  app.add_option("--nof_hwacc", config->nof_hwacc, "Number of hardware-accelerated PDSCH encoding functions")
      ->capture_default_str()
      ->check(CLI::Range(0, 64));
  app.add_option("--cb_mode", config->cb_mode, "Operation mode of the PDSCH encoder (CB = true, TB = false [default])")
      ->capture_default_str();
  app.add_option("--max_buffer_size",
                 config->max_buffer_size,
                 "Maximum supported buffer size in bytes (CB mode will be forced for larger TBs)")
      ->capture_default_str();
  app.add_option("--dedicated_queue",
                 config->dedicated_queue,
                 "Hardware queue use for the PDSCH encoder (dedicated = true [default], shared = false)")
      ->capture_default_str();
}
static void configure_cli11_hwacc_pusch_dec_args(CLI::App& app, std::optional<hwacc_pusch_appconfig>& config)
{
  config.emplace();

  app.add_option("--nof_hwacc", config->nof_hwacc, "Number of hardware-accelerated PDSCH encoding functions")
      ->capture_default_str()
      ->check(CLI::Range(0, 64));
  app.add_option("--ext_softbuffer",
                 config->ext_softbuffer,
                 "Defines if the soft-buffer is implemented in the accelerator or not")
      ->capture_default_str();
  app.add_option("--harq_context_size", config->harq_context_size, "Size of the HARQ context repository")
      ->capture_default_str();
  app.add_option("--dedicated_queue",
                 config->dedicated_queue,
                 "Hardware queue use for the PUSCH decoder (dedicated = true [default], shared = false)")
      ->capture_default_str();
}

static void configure_cli11_bbdev_hwacc_args(CLI::App& app, std::optional<bbdev_appconfig>& config)
{
  config.emplace();

  app.add_option("--hwacc_type", config->hwacc_type, "Type of BBDEV hardware-accelerator")->capture_default_str();
  app.add_option("--id", config->id, "ID of the BBDEV-based hardware-accelerator.")
      ->capture_default_str()
      ->check(CLI::Range(0, 65535));

  // (Optional) Hardware-accelerated PDSCH encoding functions configuration.
  CLI::App* hwacc_pdsch_enc_subcmd =
      app.add_subcommand("pdsch_enc", "Hardware-accelerated PDSCH encoding functions configuration");
  configure_cli11_hwacc_pdsch_enc_args(*hwacc_pdsch_enc_subcmd, config->pdsch_enc);

  // (Optional) Hardware-accelerated PUSCH decoding functions configuration.
  CLI::App* hwacc_pusch_dec_subcmd =
      app.add_subcommand("pusch_dec", "Hardware-accelerated PUSCH decoding functions configuration");
  configure_cli11_hwacc_pusch_dec_args(*hwacc_pusch_dec_subcmd, config->pusch_dec);

  app.add_option("--msg_mbuf_size",
                 config->msg_mbuf_size,
                 "Size of the mbufs storing unencoded and unrate-matched messages (in bytes)")
      ->capture_default_str()
      ->check(CLI::Range(0, 64000));
  app.add_option("--rm_mbuf_size",
                 config->rm_mbuf_size,
                 "Size of the mbufs storing encoded and rate-matched messages (in bytes)")
      ->capture_default_str()
      ->check(CLI::Range(0, 64000));
  app.add_option("--nof_mbuf", config->nof_mbuf, "Number of mbufs in the memory pool")->capture_default_str();
}

static void configure_cli11_hal_args(CLI::App& app, std::optional<du_low_unit_hal_config>& config)
{
  config.emplace();

  // (Optional) BBDEV-based hardware-accelerator configuration.
  CLI::App* bbdev_hwacc_subcmd =
      add_subcommand(app, "bbdev_hwacc", "BBDEV-based hardware-acceleration configuration parameters");
  configure_cli11_bbdev_hwacc_args(*bbdev_hwacc_subcmd, config->bbdev_hwacc);
}

static void manage_hal_optional(CLI::App& app, du_low_unit_config& parsed_cfg)
{
  // Clean the HAL optional.
  if (app.get_subcommand("hal")->count_all() == 0) {
    parsed_cfg.hal_config.reset();

    return;
  }

  const auto& hal = app.get_subcommand("hal");
  if (hal->get_subcommand("bbdev_hwacc")->count_all() == 0) {
    parsed_cfg.hal_config->bbdev_hwacc.reset();
  }
}

void srsran::configure_cli11_with_du_low_config_schema(CLI::App& app, du_low_unit_config& parsed_cfg)
{
  // Loggers section.
  CLI::App* log_subcmd = add_subcommand(app, "log", "Logging configuration")->configurable();
  configure_cli11_log_args(*log_subcmd, parsed_cfg.loggers);

  // Expert upper PHY section.
  CLI::App* expert_phy_subcmd =
      add_subcommand(app, "expert_phy", "Expert physical layer configuration")->configurable();
  configure_cli11_expert_phy_args(*expert_phy_subcmd, parsed_cfg.expert_phy_cfg);

  // Expert execution section.
  CLI::App* expert_subcmd = add_subcommand(app, "expert_execution", "Expert execution configuration")->configurable();
  configure_cli11_expert_execution_args(*expert_subcmd, parsed_cfg.expert_execution_cfg);

  // HAL section.
  CLI::App* hal_subcmd = add_subcommand(app, "hal", "HAL configuration")->configurable();
  configure_cli11_hal_args(*hal_subcmd, parsed_cfg.hal_config);
}

void srsran::autoderive_du_low_parameters_after_parsing(CLI::App&           app,
                                                        du_low_unit_config& parsed_cfg,
                                                        duplex_mode         mode,
                                                        bool                is_blocking_mode_enabled,
                                                        unsigned            nof_cells)
{
  // ################################################################################ //
  // If max proc delay property is not present in the config, configure the default value.
  CLI::App* expert_cmd = app.get_subcommand("expert_phy");
  if (expert_cmd->count_all() == 0 || expert_cmd->count("--max_proc_delay") == 0) {
    switch (mode) {
      case duplex_mode::TDD:
        parsed_cfg.expert_phy_cfg.max_processing_delay_slots = 5.0f;
        break;
      case duplex_mode::FDD:
        parsed_cfg.expert_phy_cfg.max_processing_delay_slots = 2.0f;
        break;
      default:
        break;
    }
  }

  // Extract the maximum allowed downlink processing delay in slots (integer part).
  parsed_cfg.expert_phy_cfg.integer_processing_delay_slots = 
      static_cast<unsigned>(std::floor(parsed_cfg.expert_phy_cfg.max_processing_delay_slots));
  // Extract the maximum allowed downlink processing delay in slots (decimal part).
  parsed_cfg.expert_phy_cfg.decimal_processing_delay_slots = 
      parsed_cfg.expert_phy_cfg.max_processing_delay_slots - parsed_cfg.expert_phy_cfg.integer_processing_delay_slots;

  // If max request headroom slots property is present in the config, do nothing.
  if (expert_cmd->count_all() == 0 || expert_cmd->count("--max_request_headroom_slots") == 0) {
    parsed_cfg.expert_phy_cfg.nof_slots_request_headroom = parsed_cfg.expert_phy_cfg.integer_processing_delay_slots;
  }
  // ################################################################################ //

  // Ignore the default settings based in the number of CPU cores for ZMQ.
  if (is_blocking_mode_enabled) {
    du_low_unit_expert_threads_config& upper = parsed_cfg.expert_execution_cfg.threads;
    upper.nof_pusch_decoder_threads          = 0;
    upper.nof_ul_threads                     = 1;
    upper.nof_dl_threads                     = 1;
  }

  if (parsed_cfg.expert_execution_cfg.cell_affinities.size() < nof_cells) {
    parsed_cfg.expert_execution_cfg.cell_affinities.resize(nof_cells);
  }

  manage_hal_optional(app, parsed_cfg);
}
