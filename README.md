
# AMD Open Source Driver for Vulkan&reg;
The AMD Open Source Driver for Vulkan&reg; is an open-source Vulkan driver for Radeon&trade; graphics adapters on Linux&reg;. It is built on top of AMD's Platform Abstraction Library (PAL), a shared component that is designed to encapsulate certain hardware and OS-specific programming details for many of AMD's 3D and compute drivers. Leveraging PAL can help provide a consistent experience across platforms, including support for recently released GPUs and compatibility with AMD developer tools.

Shaders that compose a particular `VkPipeline` object are compiled as a single entity using the LLVM-Based Pipeline Compiler (LLPC) library.  LLPC builds on LLVM's existing shader compilation infrastructure for AMD GPUs to generate code objects compatible with PAL's pipeline ABI. Notably, AMD's closed-source Vulkan driver currently uses a different pipeline compiler, which is the major difference between AMD's open-source and closed-source Vulkan drivers.


![High-Level Architecture Diagram](topLevelArch.png)

### Product Support
The AMD Open Source Driver for Vulkan is designed to support the following AMD GPUs:

* Radeon&trade; HD 7000 Series
* Radeon&trade; HD 8000M Series
* Radeon&trade; R5/R7/R9 200/300 Series
* Radeon&trade; RX 400/500 Series
* Radeon&trade; M200/M300/M400 Series
* Radeon&trade; RX Vega Series
* AMD FirePro&trade; Workstation Wx000/Wx100/Wx300 Series
* Radeon&trade; Pro WX x100 Series
* Radeon&trade; Pro 400/500 Series


### Operating System Support
The AMD Open Source Driver for Vulkan is designed to support following distros on both the AMDGPU upstream driver stack and the [AMDGPU Pro driver stack](http://support.amd.com/en-us/kb-articles/Pages/Radeon-Software-for-Linux-Release-Notes.aspx):
* Ubuntu 16.04.3 (64-bit version)
* RedHat 7.4 (64-bit version)

The driver has not been tested on other distros. You may try it out on other distros of your choice.

### Feature Support and Performance
The AMD Open Source Driver for Vulkan is designed to support the following features:

* Vulkan 1.1
* More than 30 extensions
* [Radeon&trade; GPUProfiler](https://github.com/GPUOpen-Tools/Radeon-GPUProfiler) tracing
* Built-in debug and profiling tools
* Mid-command buffer preemption and SR-IOV virtualization

The following features and improvements are planned in future releases (Please refer to [Release Notes](https://github.com/GPUOpen-Drivers/AMDVLK/wiki/Release-notes) for update of each release):
* Upcoming versions of the Vulkan API
* Hardware performance counter collection through [RenderDoc](https://renderdoc.org/)
* LLPC optimizations to improve GPU-limited performance and compile time
* Optimizations to improve CPU-limited performance


### Known Issues
* Dawn of War III show corruption with max setting on Radeon&trade; RX Vega Series
* CTS may hang in VK.synchronization.internally_synchronized_objects.pipeline_cache_compute with Linux kernel versions lower than 4.13

### How to Contribute
You are welcome to submit contributions of code to the AMD Open Source Driver for Vulkan.

The driver is built from source code in three repositories: [LLVM](https://github.com/GPUOpen-Drivers/llvm), [XGL](https://github.com/GPUOpen-Drivers/xgl) (including both Vulkan API translation and LLPC) and [PAL](https://github.com/GPUOpen-Drivers/pal).

For changes to LLVM, you should submit contribution to the [LLVM trunk](http://llvm.org/svn/llvm-project/llvm/trunk/). Commits there will be evaluated to merge into the amd-vulkan-master branch periodically.

For changes to XGL or PAL, please [create a pull request](https://help.github.com/articles/creating-a-pull-request/) against the dev branch. After your change is reviewed and if it is accepted, it will be evaluated to merge into the master branch in a subsequent regular promotion.

**IMPORTANT**: By creating a pull request, you agree to allow your contribution to be licensed by the project owners under the terms of the [MIT License](LICENSE.txt).

When contributing to XGL and PAL, your code should:
* Match the style of nearby existing code. Your code may be edited to comply with our coding standards when it is merged into the master branch.
* Avoid adding new dependencies, including dependencies on STL.

Please make each contribution reasonably small. If you would like to make a big contribution, like a new feature or extension, please raise an issue first to allow planning to evaluate and review your work.

> **Note:** Since PAL is a shared component that must support other APIs, other operating systems, and pre-production hardware, you might be asked to revise your PAL change for reasons that may not be obvious from a pure Linux Vulkan driver perspective.

## Build Instructions

### System Requirements
It is recommended to install 16GB RAM in your build system.

### Install Dev and Tools Packages
#### Ubuntu
```
sudo apt-get install build-essential python3 cmake curl g++-multilib gcc-multilib

sudo apt-get install libx11-dev libxcb1-dev x11proto-dri2-dev libxcb-dri3-dev libxcb-dri2-0-dev libxcb-present-dev libxshmfence-dev libx11-dev:i386 libxcb1-dev:i386 x11proto-dri2-dev:i386 libxcb-dri3-dev:i386 libxcb-dri2-0-dev:i386 libxcb-present-dev:i386 libxshmfence-dev:i386 libwayland-dev libwayland-dev:i386
```
#### RedHat
```
wget https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm

sudo yum localinstall epel-release-latest-7.noarch.rpm

sudo yum update

sudo yum -y install gcc-c++ cmake3 python34 curl glibc-devel glibc-devel.i686 libstdc++-devel libstdc++-devel.i686 libxcb-devel libxcb-devel.i686 libX11-devel libX11-devel.i686 libxshmfence-devel libxshmfence-devel.i686
```

### Get Repo Tools

```
mkdir ~/bin
curl https://storage.googleapis.com/git-repo-downloads/repo > ~/bin/repo
chmod a+x ~/bin/repo
```

### Get Source Code

```
mkdir vulkandriver
cd vulkandriver
~/bin/repo init -u https://github.com/GPUOpen-Drivers/AMDVLK.git -b master
~/bin/repo sync
```

> **Note:** Source code in dev branch can be gotten by using "-b dev" in the "repo init" command

### 64-bit Build
#### Ubuntu
```
cd <root of vulkandriver>/drivers/xgl

cmake -H. -Bbuilds/Release64

cd builds/Release64

make -j$(nproc)
```

#### RedHat
```
cd <root of vulkandriver>/drivers/xgl

cmake3 -H. -Bbuilds/Release64

cd builds/Release64

make -j$(nproc)
```

### 32-bit Build
#### Ubuntu
```
cd <root of vulkandriver>/drivers/xgl

cmake -H. -Bbuilds/Release -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32

cd builds/Release

make -j$(nproc)
```
#### RedHat
```
cd <root of vulkandriver>/drivers/xgl

cmake3 -H. -Bbuilds/Release -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32

cd builds/Release

make -j$(nproc)
```
> **Note:**  
* If the build runs into errors like "collect2: fatal error: ld terminated with signal 9 [Killed]" due to out of memory, you could try  with reducing the number of threads in "make" command.  
* Debug build can be done by using -DCMAKE_BUILD_TYPE=Debug.
* To enable Wayland support, you need to build the driver by using -DBUILD_WAYLAND_SUPPORT=ON and install the Wayland [WSA libarary](https://github.com/GPUOpen-Drivers/wsa). 

## Installation Instructions
### Install Vulkan SDK
Refer to installation instructions [here](http://support.amd.com/en-us/kb-articles/Pages/Install-LunarG-Vulkan-SDK.aspx).

### Uninstall Previously Installed JSON Files
Please make sure all JSON files for AMD GPUs under below folders are uninstalled: 

```
/etc/vulkan/icd.d
/usr/share/vulkan/icd.d
```

### Copy Driver and JSON Files
#### Ubuntu
```
sudo cp <root of vulkandriver>/drivers/xgl/builds/Release64/icd/amdvlk64.so /usr/lib/x86_64-linux-gnu/
sudo cp <root of vulkandriver>/drivers/xgl/builds/Release/icd/amdvlk32.so /usr/lib/i386-linux-gnu/
sudo cp <root of vulkandriver>/drivers/AMDVLK/json/Ubuntu/* /etc/vulkan/icd.d/
```
#### RedHat
```
sudo cp <root of vulkandriver>/drivers/xgl/builds/Release64/icd/amdvlk64.so /usr/lib64/
sudo cp <root of vulkandriver>/drivers/xgl/builds/Release/icd/amdvlk32.so /usr/lib/
sudo cp <root of vulkandriver>/drivers/AMDVLK/json/Redhat/* /etc/vulkan/icd.d/
```

> **Note:** The remaining steps are only required when running the AMDGPU upstream driver stack.

### Turn on DRI3 and disable modesetting X driver
Add following lines in /usr/share/X11/xorg.conf.d/10-amdgpu.conf:
```
Section "Device"

Identifier "AMDgpu"

Option  "DRI" "3"

EndSection
```

And make sure following line is **NOT** included in the section:
```
Driver      "modesetting"
```

### Required Settings
On the AMDGPU upstream driver stack, the max number of command streams per submission **MUST** be limited to 4 (the default setting in AMD Open Source driver for Vulkan is 16). This can be accomplished via the [Runtime Settings](#runtime-settings) mechanism by adding the following line to /etc/amd/amdPalSettings.cfg:
```
MaxNumCmdStreamsPerSubmit,4
```

## Runtime Settings
The driver exposes many settings that can customize the driver's behavior and facilitate debugging.  Add/edit settings in /etc/amd/amdPalSettings.cfg, formatted with one `name,value` pair per line.  Some example settings are listed below:

| Setting Name             | Valid Values                                                | Comment                                                                                                                           |
| ------------------------ | ----------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| `ShaderCacheMode`        | 0: disable cache<br/>1: runtime cache<br/>2: cache to disk  | Runtime cache is the default mode.                                                                                                |
| `IFH`                    | 0: default<br/>1: drop all submits<br/>                     | Infinitely Fast Hardware.  Submit calls are dropped before being sent to hardware.  Useful for measuring CPU-limited performance. |
| `EnableVmAlwaysValid`    | 0: disable<br/>1: default<br/>2:  force enable<br/>                               | 1 is the default setting which enables the VM-always-valid feature for kernel 4.16 and above.  The feature can reduce command buffer submission overhead related to virtual memory management.     |
| `IdleAfterSubmitGpuMask` | Bitmask of GPUs (i.e., bit 0 is GPU0, etc.)                 | Forces the CPU to immediately wait for each GPU submission to complete on the specified set of GPUs.                              |

*All* available settings can be determined by examining the .cfg source files that define them.

* .../xgl/icd/settings/settings.cfg (API layer settings)
* .../pal/src/core/settings.cfg (PAL hardware-independent settings)
* .../pal/src/core/hw/gfxip/gfx6/gfx6PalSettings.cfg (PAL GFX6-8 settings)
* .../pal/src/core/hw/gfxip/gfx9/gfx9PalSettings.cfg (PAL GFX9+ settings)

Runtime settings are only read at device initialization, and cannot be changed without restarting the application. If running on a system with multiple GPUs, the same settings will apply to all of them.  Lines in the settings file that start with `;` will be treated as comments.


## PAL GpuProfiler Layer
The GpuProfiler is an optional layer that is designed to intercept the PAL interface to provide basic GPU profiling support.  Currently, this layer is controlled exclusively through runtime settings and outputs its results to file.

You can use the following [Runtime Settings](#runtime-settings) to generate a .csv file with GPU timings of work performed during the designated frames:

| Setting Name                     | Value                            | Comment                                                                                                                                                                                               |
| -------------------------------- | -------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `GpuProfilerMode`                | 0: disable<br/>1: enable with sqtt off<br/>2: enable with sqtt for thread trace<br/>3: enable with sqtt for RGP                     | Enables and sets the SQTT mode for the GPU performance profiler layer. Actual capture of performance data must be specified via frame number with GpuProfilerStartFrame or by holding shift-F11.     |
| `GpuProfilerLogDirectory`        | <nobr>&lt;directory-path></nobr> | Must be a directory that your application has write permissions for.                                                                                                                                  |
| `GpuProfilerGranularity`         | 0: per-draw<br/>1: per-cmdbuf    | Defines what is measured/profiled.  *Per-draw* times individual commands (such as draw, dispatch, etc.) inside command buffers, while *per-cmdbuf* only profiles entire command buffers in aggregate. |
| `GpuProfilerStartFrame`          | Positive integer                 | First frame to capture data for.  If StartFrame and FrameCount are not set, all frames will be profiled.                                                                                              |
| `GpuProfilerFrameCount`          | Positive integer                 | Number of frames to capture data for.                                                                                                                                                                 |
| `GpuProfilerRecordPipelineStats` | 0, 1                             | Gathers pipeline statistic query data per entry if enabled.                                                                                                                                           |

## PAL Debug Overlay
PAL's debug overlay can be enabled to display real time statistics and information on top of a running application.  This includes a rolling FPS average, CPU and GPU frame times, and a ledger tracking how much video memory has been allocated from each available heap.  Benchmarking (i.e., "Benchmark (F11)") is currently unsupported.

| Setting Name                    | Value                                                                                                                   | Comment                                                                                                                             |
| ------------------------------- | ----------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| `DebugOverlayEnabled`           | 0, 1                                                                                                                    | Enables the debug overlay.                                                                                                          |
| `DebugOverlayLocation`          | <nobr>0: top-left</nobr><br/><nobr>1: top-right</nobr><br/><nobr>2: bottom-left</nobr><br/><nobr>3: bottom-right</nobr> | Determines where the overlay text should be displayed.  Can be used to avoid collision with important rendering by the application. |
| `PrintFrameNumber`              | 0, 1                                                                                                                    | Reports the current frame number.  Useful when determining a good frame range for profiling with the GpuProfiler layer.             |
| `TimeGraphEnable`               | 0, 1                                                                                                                    | Enables rendering of a graph of recent CPU and GPU frame times.                                                                     |


## Third Party Software
The AMD Open Source Driver for Vulkan contains code written by third parties.
* LLVM is distributed under the terms of the University of Illinois/NCSA Open Source License. See LICENSE.TXT file in the top directory of the LLVM repository.
* Please see the README.md file in the [PAL](https://github.com/GPUOpen-Drivers/pal) and [XGL](https://github.com/GPUOpen-Drivers/xgl) repositories for information on third party software used by those libraries.


#### DISCLAIMER
The information contained herein is for informational purposes only, and is subject to change without notice. This document may contain technical inaccuracies, omissions and typographical errors, and AMD is under no obligation to update or otherwise correct this information. Advanced Micro Devices, Inc. makes no representations or warranties with respect to the accuracy or completeness of the contents of this document, and assumes no liability of any kind, including the implied warranties of noninfringement, merchantability or fitness for particular purposes, with respect to the operation or use of AMD hardware, software or other products described herein.  No license, including implied or arising by estoppel, to any intellectual property rights is granted by this document.  Terms and limitations applicable to the purchase or use of AMD's products are as set forth in a signed agreement between the parties or in AMD's Standard Terms and Conditions of Sale.

AMD, the AMD Arrow logo, Radeon, FirePro, and combinations thereof are trademarks of Advanced Micro Devices, Inc.  Other product names used in this publication are for identification purposes only and may be trademarks of their respective companies.

Vega is a codename for AMD architecture, and is not a product name. 

Linux is the registered trademark of Linus Torvalds in the U.S. and other countries.

Vulkan and the Vulkan logo are registered trademarks of the Khronos Group, Inc.

