---
pagetitle: Release Notes for HS DataLog CLI example 
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center> 
# Release Notes for <mark>HS DataLog CLI example</mark> 
Copyright &copy; 2023 STMicroelectronics
    
[![ST logo](_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>


# Purpose

The High-Speed DataLog firmware provides an easy way to save data from any combination 
of sensors and microphones configured at their maximum sampling rate.
Sensor data can be either stored on a micro SD Card, SDHC (Secure Digital High Capacity) 
formatted with the FAT32 file system, or streamed to a PC via USB (WinUSB class).

To save data via USB, a command line interface example is available in 
“Utilities/cli_example”.
USB_DataLog_Run.bat and USB_DataLog_Run.sh scripts provide a ready to use example.

README.md and README_Linux.md describes how to use cli_example properly in Windows and Linux environment.

:::

::: {.col-sm-12 .col-lg-8}
# Update History

::: {.collapse}
<input type="checkbox" id="collapse-section18" checked aria-hidden="true">
<label for="collapse-section18" aria-hidden="true">v3.6.1 / 15-May-26</label>
<div>


## Main Changes

### Patch Release

- Input from community: updated CMakeLists to let user recompile cli_example
- **cli_example is deprecated and in NRND state. It is Not Recommended for New Design. This is the last update and no further updates are planned. Please, consider moving to [STDATALOG_PYSDK](https://github.com/STMicroelectronics/stdatalog-pysdk).**


</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section17" checked aria-hidden="true">
<label for="collapse-section17" aria-hidden="true">v3.6.0 / 9-Apr-25</label>
<div>


## Main Changes

### Maintenance Release

- Added macos support.
- Updated libhs_datalog_v2 libraries for all the supported OS.
	- Updated libusb linking and added missing lirbary import for UNIX.
	- Added a new logging system to manage application messages with different levels (NONE, ERROR, WARNING, INFO, DEBUG).
	- Updated cmake_minimum_required version used to recompile libraries.
	- Added new hs_datalog_load_ucf_file_to_mlc API (deprecated old hs_datalog_load_ucf_to_mlc API).


</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section16" aria-hidden="true">
<label for="collapse-section16" aria-hidden="true">v3.5.0 / 19-Jul-24</label>
<div>

## Main Changes

### Maintenance release and product update

- Reshaped RaspberryPi support and fixed readme files

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section15" aria-hidden="true">
<label for="collapse-section15" aria-hidden="true">v3.4.0 / 16-Feb-24</label>
<div>

## Main Changes

### Maintenance release and product update

- Bugs fixed:
  - Enable USB channel only for enabled component
  - Reshaped state machine to manage libusb timeout error
- Added support for Raspberry PI 4, both 32 and 64 bit OS
- Rebuilt executables and dlls for all supported devices

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section14" aria-hidden="true">
<label for="collapse-section14" aria-hidden="true">v3.3.1 / 27-Nov-23</label>
<div>

## Main Changes

### Patch release

- Fixed bug in MLC management in exe: missing ism330dhcx get status callback
- Solved a segmentation fault in DLL: missing pointer declaration
- Added a new API in DLL: update_components_map
- Update README_Linux

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13" aria-hidden="true">
<label for="collapse-section13" aria-hidden="true">v3.3.0 / 29-Sep-23</label>
<div>

## Main Changes

### Maintenance release and product update

- Add PnPL SET and COMMAND responses management
- Regenerated exe and dll
- Updated setup files to better describe installation steps

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12" aria-hidden="true">
<label for="collapse-section12" aria-hidden="true">v3.2.0 / 7-Jul-23</label>
<div>

## Main Changes

### Maintenance release and product update

- Regenerated libhs_datalog_v2 DLLs:
	- Added hs_datalog_get_identity API
	- Added hs_datalog_register_usb_hotplug_callback API
- Fixed char warning in cli_example executable
- Linux setup script fixed
- Added missing readme

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11" aria-hidden="true">
<label for="collapse-section11" aria-hidden="true">v3.1.0 / 24-Mar-23</label>
<div>

## Main Changes

### Maintenance release and product update

- Regenerated libhs_datalog_v2 DLLs:
	- Support updated productID for DATALOG2 firwmares. New PID are: 0x5743 for DATALOG1 products and 0x5744 for DATALOG2 products
	- Added support for Actuator c_type
	- Added missing setType call
	- Updated get_presentation PnPL base command
- Fixed a print on screen on CLI: press ENTER key to start logging

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10" aria-hidden="true">v3.0.0 / 30-Sep-22</label>
<div>

## Main Changes

### Product update

- Added support for both DATALOG1 and DATALOG2

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9" aria-hidden="true">
<label for="collapse-section9" aria-hidden="true">v2.5.0 / 8-Jul-2022</label>
<div>

## Main Changes

### Product update

- Added configuration scripts to setup cli_example in Linux and Raspberry environment
- Updated libraries and exe for Windows (32bit and 64bit), Linux and Raspberry pi 3B
- Generalized ref to MLC sensors and boards into main.cpp
- Updated licensing schema

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8" aria-hidden="true">
<label for="collapse-section8" aria-hidden="true">v2.4.0 / 5-Nov-2021</label>
<div>

## Main Changes

### Product update

- **Added new Raspberry pi 3B cli example**
- Rebuilt libraries and exe for Windows (32bit and 64bit), Linux and Raspberry pi 3B

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7" aria-hidden="true">
<label for="collapse-section7" aria-hidden="true">v2.3.0 / 25-Jun-2021</label>
<div>

## Main Changes

### Product update

- **Added new Raspberry pi 3B dll**

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6" aria-hidden="true">
<label for="collapse-section6" aria-hidden="true">v2.2.0 / 18-Dec-2020</label>
<div>

## Main Changes

### Maintenance release and product update

- Bug solved: ucf not read correctly if bigger then DeviceConfig.json
- **Added new 64-bit dll**
- **Added new 64-bit cli_example.exe**

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5" aria-hidden="true">
<label for="collapse-section5" aria-hidden="true">v2.1.0 / 13-Nov-2020</label>
<div>

## Main Changes

### Maintenance release and product update

- Recompiled cli_example application, both for Linux and Windows
- Added -g option, to get current Device Configuration from the board
- Handling all the 8 MLC outputs available
- Minor issue solved


</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" aria-hidden="true">
<label for="collapse-section4" aria-hidden="true">v2.0.0 / 19-Jun-2020</label>
<div>

## Main Changes

### Maintenance release and product update

- Redesigned cli_example application, both for Linux and Windows
- Updated compiled libraries (.so and .dll)
- Minor issue solved and code cleaning


</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3" aria-hidden="true">
<label for="collapse-section3" aria-hidden="true">v1.0.0 / 05-Jul-2019</label>
<div>

## Main Changes

### First official release

- First official release
- New HS_DataLog.dll
- New batch scripts


</div>
:::

:::
:::

<footer class="sticky">
::: {.columns}
::: {.column width="95%"}
For complete documentation on **FP-SNS-DATALOG2** ,
visit: [www.st.com](https://www.st.com/en/embedded-software/fp-sns-datalog2.html)
:::
::: {.column width="5%"}
<abbr title="Based on template cx566953 version 2.0">Info</abbr>
:::
:::
</footer>
