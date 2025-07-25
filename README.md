# Towards URLLC with Open-Source 5G Software

[Aoyu Gong](https://aygong.com/), [Arman Maghsoudnia](https://people.epfl.ch/arman.maghsoudnia), [Raphael Cannatà](https://www.raphaelcannata.com/), [Eduard Vlad](https://evlad.de/), [Néstor Lomba Lomba](https://www.linkedin.com/in/nlomba), [Dan Mihai Dumitriu](https://www.linkedin.com/in/dmdumitriu), [Haitham Hassanieh](https://people.epfl.ch/haitham.alhassanieh?lang=en)

[[Paper](https://aygong.com/docu/openrit6g25-final5.pdf)] [Slides] [Citation] [[srsRAN Workshop](https://www.youtube.com/watch?v=o88ti2lHDuY)]

## Table of Contents
- [🧭 Overview](#-overview)
- [🔧 Environment Setup](#-environment-setup)
  - [Build From Source](#build-from-source)
  - [Docker Image](#docker-image)
- [⚙️ gNB Configuration Parameters](#-gnb-configuration-parameters)
- [📄 Citation](#-citation)
- [😋 Questions or Issues?](#-questions-or-issues)

## 🧭 Overview

<div align="center">
  <p>
    <img src="assets/sr_free_distribution.png" height="220"/> &nbsp;&nbsp;&nbsp;&nbsp;
    <img src="assets/mean_latency_bar.png" height="220"/>
  </p>
</div>

This repository provides the code associated with our paper: 
> Towards URLLC with Open-Source 5G Software


## 🔧 Environment Setup

Our modifications are built on top of the original [srsRAN](https://github.com/srsran/srsRAN_Project).  
### Build From Source
To build and run our modified version, please follow the same environment setup instructions as described in the official [srsRAN README](/srsRAN_README.md).

### Docker Image
Alternatively, you can use our [pre-built Docker image](https://hub.docker.com/r/sensepfl/srs-gnb-low-latency).
1. Pull the Docker image
1. Start the Docker container
1. Edit the configuration file `gnb_low_latency.yml` to match your setup (e.g., IP addresses, ports, etc.) 
   ```bash
   nano /srsRAN_Project_LowLatency/configs/gnb_low_latency.yml
   ```
1. Start the gNB (CU and DU are also available in the same image using `srscu` and `srsdu` commands)
    ```bash
    srsgnb -c /srsRAN_Project_LowLatency/configs/gnb_low_latency.yml
    ```

## ⚙️ gNB Configuration Parameters

> You can find the full list of srsRAN configuration parameters [here](https://docs.srsran.com/projects/project/en/latest/user_manuals/source/config_ref.html).

We introduce four new configuration parameters to support low-latency modifications:

- `sr_free_access_enable`
    - Enables Scheduling Request (SR)-free access to the slice. 
    - Optional BOOLEAN (false). Supported: [false, true].
- `min_ul_grant_size`
    - Specifies the minimum size (in bytes) of uplink grants assigned to the slice.
    - Optional INT (0). Supported: [0 - 100000].
- `max_proc_delay`
    - Sets the maximum allowed DL processing delay in slots.
    - Optional FLOAT (5.0). Supported: [0.0 - 30.0].
    - This value corresponds to the $M$-slot offset.
- `radio_heads_prep_time`
    - Sets the maximum allowed preparation time for radio heads in slots.
    - Optional INT (3). Supported: [1 - 30].
    - This value corresponds to the $H$-slot offset.

ℹ️ Note: We have extended the original `max_proc_delay` parameter to support `FLOAT` in addition to `INT`.

🧾 Example Configuration:

```
cell_cfg:
  slicing:
    - sst: 1
      sd: 0
      sched_cfg:
        sr_free_access_enable: false
        min_ul_grant_size: 512

expert_phy:
  max_proc_delay: 0.5
  radio_heads_prep_time: 1
```

💡 An example YAML configuration file is provided [here](/configs/gnb_low_latency.yml).


## 📄 Citation

If you find the codebase helpful, please consider giving a ⭐ and citing our paper:
```bibtex
@inproceedings{sens2025urllc,
  title={Towards URLLC with Open-Source 5G Software},
  author={Gong, Aoyu and Maghsoudnia, Arman and Cannatà, Raphael and Vlad, Eduard and Lomba Lomba, Néstor and Dumitriu, Dan Mihai and Hassanieh, Haitham},
  booktitle={Proceedings of the 2025 SIGCOMM Workshop on Open Research Infrastructures and Toolkits for 6G (OpenRIT6G '25)},
  year={2025},
  publisher={Association for Computing Machinery},
  address={New York, NY, USA}
}
```


## 😋 Questions or Issues?

If you run into problems or have suggestions, feel free to open an issue or reach out to us.
