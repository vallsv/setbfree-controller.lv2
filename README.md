# setbfree-controller.lv2

Virtual controller based on [LV2](http://lv2plug.in/) to tune [setBfree synthesizer](http://setbfree.org).

It was designed for [Mod Devices](http://moddevices.com).

![Preview](./modgui/screenshot.png)

# Features

It uses the default MIDI settings initialized by setBfree to control:

- presets (global and upper and lower keyboard)
- 9x3 drawbars
- overdrive, reverb, volume
- percussion including few switches
- vibrato including few switches
- drawbar randomizer

MIDI notifications to update the GUI are not supported.

It does not really provide a convenient way to save and restore a configuration later. This is due to synchronization problem at start up. But an extra push button is provided, which can be triggered manually to ask the controller to emit the configuration again. If a preset is set, no other parameters are emitted. Else all the other parameters are emitted. This trigger can be binded to a physical button of the Mod device.

Still it is not a perfect solution without supporting notifications from setBfree.

# Build

To build the plugin:

```
make build
```
# Install

To install it to the user space: `~/.lv2/`

```
make install-user
```

To install it to your system: `/lib/lv2`

```
make install
```

# Deploy

Deploy the plugin to the Mod Duo X. It uses the default IP and it's web service.

```
make deploy
```
