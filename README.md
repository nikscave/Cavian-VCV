# Cavian Sequencer

Cavian is a powerful 8-channel multi-step sequencer for VCV Rack.  
A Eurorack Hardware version is in the works and as that will be based on ESP32 controller, your patterns generated on VCV can be saved to the external module and vice versa through your own wifi or via hotspot via the ESP32.
I have the prototype working, just not finished the PCB design yet but it's coming 2026.  
(Influenced by the Circadian Rhythm Gate and Trigger Sequence)

![Cavian Main Interface](/images/CavianMain.jpg)

---

## Logic & Editing Operations

In it's simplest form, use Cavian to send gates or triggers (click once for Step, and again for a Gate, once more to turn off) on it's 8 outputs.
It can get as complex as you need it to be by chaining your 8 channels into 8 presets then 8 Groups.  
In Vert mode, single clicks are used whilst on 8x8 and 1x64 single click or continue to drag the mouse around the grid to create your patterns. Each mouse drag will only affect the same step once.


### Randomization
1. Click the **Random** button.
2. Click any **Group**, **Preset**, or **Channel** button to randomise all steps within that target.
* **Weighting**: Right-click to activate the menu to adjust the probability weighting.  
![SubMenu](/images/CavianSubMenu.jpg)

### Load and Save
Right click and the menu options will be obvious. Everything in VCV is saved locally to your plugins folder

### Copy & Paste
These functions operate with the same "Select Action → Select Target" workflow:
1. Click **Copy**, then click your source (Group/Preset/Channel).
2. Click **Paste**, then click your destination.

---

## Keyboard Shortcuts
To use these shortcuts, ensure your mouse cursor is hovering over the module.

### Channel Management
* <kbd>1</kbd> – <kbd>8</kbd> : Toggle **Mute/Unmute** for the respective channel.
* <kbd>Space</kbd> : **Start/Stop** the sequencer.

### Navigation & Global Controls
Use the "Hold + Number" method to quickly navigate the engine:
* <kbd>G</kbd> + <kbd>1</kbd>–<kbd>8</kbd> : Switch **Group**
* <kbd>P</kbd> + <kbd>1</kbd>–<kbd>8</kbd> : Switch **Preset**
* <kbd>C</kbd> + <kbd>1</kbd>–<kbd>8</kbd> : Switch **Channel**

Changing Groups/Presets/Channels on 8x8 or 1x64 view mode only really makes sense for live jams, so I opted for only keyboard actions for these views to simplify the UI.  

### View Modes
Press <kbd>V</kbd> to cycle through display layouts:
* **Vertical**: Standard channel strip view.
* **8x8**: Grid overview of all active channels.
* **1x64**: Expanded view (optimized for use with looped groups and presets).

![SubMenu](/images/Cavian8x8.jpg)

---
## Inputs / Outputs
* 8 channel outputs
* 1 Clock Out
* 1 Clock in
* 1 Reset Out
* BPM Knob (double tap to enter manually)


---
## Future Features
* **Hardware Integration**: Built-in support for a live connection with the Cavian hardware counterpart is already present in the codebase. We just need the hardware to be available :-)
* **Ignore the "Enable Hardware Sync"** as only I have the software running on ESP32 as of now, so it will just time out and not do anything, this is expected behavior right now.


## Feedback & Support
Cavian is in active development. If you encounter any bugs or have suggestions for new features:
* Please open an issue in the **[Issues Tab](https://github.com/nikscave/Cavian-VCV/issues)** at the top of this page.
* Or just drop me a mail at cavian@nikscave.com or a message via Instagram

Follow me on Instagram: [![Instagram](https://img.shields.io/badge/@yourmodulesmayvary-%23E4405F.svg?style=flat&logo=Instagram&logoColor=white)](https://www.instagram.com/yourmodulesmayvary)

*Thank you for supporting YourModulesMayVary!*

![Downloads](https://img.shields.io/github/downloads/nikscave/Cavian-VCV/total?style=for-the-badge&color=blue)

