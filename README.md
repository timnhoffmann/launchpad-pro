# ising - a cellular automaton and step sequencer for the launchpad pro
This is a still rather experimental sequencer running on the launchpad pro. This is possible due to the wonderful opem firmware made available for this device. See below for the original readme on that and for information on how to compile and install it. 

## Functionality
Currently the system has three modes accessed by the upper right buttons "Session", "Note", and "Device". "Note" will give you a plain grid for playing an instrument in the now already usual way: columns are a semi-tone apart, rows are a 4th.

"Session" gives a 8 track or instrument pattern sequencer. Again in a rather usual way: It allwos for realtime recording or step input. The sequences are organized in 8 patterns of 16 steps, the sequence length can be anything from 1 to 128 steps.

"Device" enters the mode that is probably the most interesting part: A 8 track or instrument sequencer driven by a cellular automaton. Namely the deterministic ising model. The 8x8 pads repersent the cells of the automaton with neigbourhood seen periodic across the borders (the rightmost pad in a row will have the leftmost one in the same row as its right neighbour etc...). The cells evolve in each timestep acordingg to the following rule: A cell switches its state (on or off) exactly if two of its direct neighbours are on (and two are off). This is done forst for the even and then for the odd "halfgrid" (think checkerboard). Now, each of the eight rows controls an instrument in the following way: if the number of on cells is odd, a note is played. The velocity is given by the number of on cells (v = number*18+1, actually) and the pitch is derived by the following rule: Each cell that is "on" when the instrument plays will add (form left to right) -12, 12, 7, 5, 3, 2, 2, and 1 semi-note resp. to the base note of that instrument. However, each cell can be configured to not contribute (on a per instrument base), making it possible for an instrument to only play its (configurable) base pitch (usefull for drums, I suppose...). 

### Step sequencer (Session)
This mode gives a 8 instrument and step sequencer.
- Click: switches between internal clock and sync to MIDI-clock (globally)
- Circle: start or stop the sequencer.
- Duplicate: duplicated the current pattern of 16 steps to the next one
- Delete: erases all notes for the current instrument in the current 16 note pattern. Holding shift deletes all 8 patterns for the instrument.
- Quantize: sets the velocities and step lengths for the whole current pattern to the current step's values
- Double: resets the step sequencer to 1st step of the starting pattern
- Record arm: toggles the record mode. This allows the current instrument to have its steps recorded in realtime (still kind of experimental - might behave weirdly if more than one note is played etc...)
- Sends: sets the starting 16 step pattern for the current instrument
- stop clip: holding this and pressing a step pad (lower 2 rows) sets the last step of the sequence. With this the lenth of the sequence can be anything from 1 to 128 steps (if the starting pattern is the first and the last step is set to the 16th of the 8th pattern). The seq-length is per instrument, of course.
- Track select: toggles scene arrows (right) function: select an instrument (for step editing) or mute/unmute an instrument.
- arrows (right): depending on the track select button state: mute or unmute an instrument or select an instrument (for step editing)
- holding solo lets you set the length of the currently selected step in quarters of a step fractions
- holding volume lets you set the velocity of the currenetly selected step (in increments of 2)
- the lower two pad rows (blue): select a step.
- the thirs pad row (red) selects one of the eight patterns for editing. Holding send while pressing sets the first/starting pattern for playback. Holding shift while pressing does the same for all instruments at once.
- pad rows 4-8: select the note for the instrument and step. The note C is highlighted in orange and middle C in light blue for orientation. This grid can be shifted around with the arrow buttons (upper left). The velocity of the step is recorded.
- the lower two pad rows (blue): depending on the state of the "mute" button: mute/unmute a step or select the step for editing (making it current).

### "Piano-grid" (Note)
A playable (velocity sensitive) note grid. The spacing is halfsteps to the sides and 4th up and down. The notes C are highlighted in orange and middle C in light blue for orientation. The grid can be shifted around with the arrow buttons (upper left). 

Pressing "setup" (upper left corner) will enter the setup mode. Then
- lower two pad rows (blue): select the MIDI channel for the istrument.

### CA sequencer (Device)
- Click: switches between internal clock and sync to MIDI-clock (globally)
- Undo: reverse time
- Delete: clear the grid
- Quantize: fill (pseudo-) randomly
- Duplicate: unused as of now
- Double: a sequencer tick is a half or two half-timesteps.
- circle: start or stop the sequencer.
- Arrows (upper left): shift the pattern (cyclically) on the grid.
- scene arrows (triangles right): mute or unmute the 8 instruments. They are off when muted, dimmly lit when active, and fully lit when playing a note.

pressing "setup" (upper left corner) will enter the setup mode. Here one can -- after selecting an instrument with the scene arrows (right) --
- lower two pad rows (blue): select the MIDI channel for the istrument.
- pad rows 3-7: select the (base) note for the instrument. The note C is highlighted in orange and middle C in light blue for orientation. This grid can be shifted around with the arrow buttons (upper left).
- upper pad row (blue): set which of the "bits" or cells in the ca pattern in the row of the selected instrument alter the pitch. If all of them are deselected the instrument will only play the base note. If switched on the cells will add (form left to right) -12, 12, 7, 5, 3, 2, 2, and 1 resp. whenever the corresponding cells are light when the instrument plays.
## global setup
Pressing setup while holding shift opens a global setup page.
- the lowest four pad rows (blue) plus scene arrows encode the tempo in bpm tempo setting for standalone operation with one fractional digit: the bottom row gives 1/10th bpm the second one fitst digit etc. so 
109.3 would have lit (lowest first): 3dr pad, scene arrow, no pad and 1st pad in the first four pad rows.
- Undo: toggles between DIN-MIDI and USB-MIDI (lit = DIN-MIDI).


In all modes shift-circle button will stop both sequencers.

# Launchpad Pro
Open source firmware for the Novation Launchpad Pro grid controller!  By customising this code, you can:

- Implement your own unique standalone apps
- Create chorders, sequencers, light shows, games and more
- Have fun!

You'll definitely need *some* C programming experience, but we've deliberately kept much of the firmwarey nastiness tucked away, to make the process a little friendlier.

This project is still at an early stage, and no "interesting" example apps have yet been developed.  You might want to hang on until there's something more detailed before you get stuck in.  Or not!

# Philosophy
We could have released the full source for the factory shipping firmware, but we decided not to for a variety of reasons.  Instead, we created a simplified framework for developing "apps" on Launchpad Pro, which comprises a build environment, application entry points / API, and a library of low level source code.  Our reasoning is as follows:

- There is no value in customising low level routines such as LED multiplexing or ADC scanning - this code has been carefully tweaked over many months to deliver the best results, and is not something you'd want to mess with.
- There is very little value in customising main() or other low level features, and again these things are hard to do well.  Interrupt priorities? No.
- If we shipped the application firmware as-is, we'd have a support nightmare on our hands (imagine the phone calls - my "Launchpad Pro is behaving strangely...").  Instead, we wanted to create a clear boundary between "normal" usage with Ableton, and custom firmware.  As such, Ableton integration has been removed from this firmware, as has the setup / navigation functionality. In addition, the "Live" USB MIDI port has been removed, and the device has a different name and USB PID.
- If we left the Ableton integration and menu structure in place, open firmware developers would have to work around it.  They would also potentially consume precious RAM/CPU resources.  I've a feeling this isn't what you'd want, but we're interested to hear your feedback.
- Licensing requirements for the CMSIS library version we use are ambiguous.  Yes, we could port to the public version, but why bother, given the above reasoning - I'd prefer to spend my time on good documentation and examples.  As such, all the CMSIS code is compiled into launchpad_pro.a, and we do not need to distribute the headers.

I'm sure you'll have feedback for us, so please do get in touch!  I'm [blogging the process too](http://launchpadfirmware.tumblr.com/) if you'd like to read my musings.

# Setup the Development Environment

## Using Vagrant

To use [Vagrant](https://www.vagrantup.com/) to manage the build environment you need to:

1. Clone this repository on your host computer (if using the command line, make sure you `git clone --recursive`).
2. Install [Vagrant](https://www.vagrantup.com/)
3. Install [VirtualBox](https://www.virtualbox.org/wiki/Downloads)
4. Open a command prompt, and navigate to the project directory
5. Type `vagrant up`, hit enter and grab a beverage of your choice.  Maybe two - it is building a lovely fresh development machine just for you!

If you have a poor internet connection, ummm, find a better one :)

### Building
Once your new "box" is up and running, you can build the app in one of two ways.  In the spirit of experimentation, we've created a full Eclipse development environment for you to use.  However, you might prefer to do things on the command line.

###  To use the command line interface:
1. SSH into the Vagrant "box" by doing `vagrant ssh`
2. At the command prompt, simply type `make`

### To build using Eclipse GUI

**Make sure you wait until the `vagrant up` command has fully completed** before logging in to your VM.  The GUI appears long before the provisioning script finishes.  If you don't, you'll have to log out and log back in again before Eclipse can see the correct path.

1. Log in to the Ubuntu GUI (the password is, as is the convention, **vagrant**).
2. Launch Eclipse from the doodah on the top left (it's a bit like Spotlight)
3. Accept the default when Eclipse asks you for a workspace.  I can't figure out how to store the workspace in source control, so you need to import it.
4. Click "Workbench" at the Eclipse startup screen.
5. In Eclipse, choose "File->Import..."
6. Under "C/C++", choose "Existing Code as Makefile Project", hit "Next"
7. Give the project any name you like (launchpad?)
8. Under "Existing Code Location" type `/vagrant`.  The toolchain isn't important, the compiler is part of the Makefile.
9. Hit Finish - you should now see your project.
10. Select your project by clicking on it.
11. Click the hammer icon at the top, and wait while the project builds.

Either of the above methods will generate the firmware image, `launchpad_pro.syx`, in the project `build` directory.  You can then upload this to your Launchpad Pro from the host!

## Using OS X

On OS X you can easily install the GCC ARM toolchain using the [homebrew package manager](http://brew.sh). The EABI tools are maintained in an external repository which you need to put on tap first. You can then run ```make``` to directly compile the code:

```
brew tap PX4/homebrew-px4
brew install gcc-arm-none-eabi
make
```

# Uploading to a Launchpad Pro
Now you've got some nice new code to run! To upload it to your Launchpad Pro, you'll need a sysex tool for your host platform (I'd love to get it working from the virtual machine, but that's for later).  I recommend [Sysex Librarian](http://www.snoize.com/SysExLibrarian/) on OS X, and [MIDI OX](http://www.midiox.com/) on Windows.  On Linux, I'll bet you already have a tool in mind.

I won't describe how to use these tools, I'm sure you already know - and if you don't, their documentation is superior to mine!  Here's what you need to do:

1. Unplug your Launchpad Pro
2. Hold the "Setup" button down while connecting it to your host via USB (ensure it's connected to the host, and not to a virtual machine!)
3. The unit will start up in "bootloader" mode
4. Send your launchpad_pro.syx file to the device MIDI port - it will briefly scroll "upgrading..." across the grid.
5. Wait for the update to complete, and for the device to reboot!

Tip - set the delay between sysex messages to as low a value as possible, so you're not waiting about for ages while the firmware uploads!

# Bricked it!
Don't worry - even if you upload toxic nonsense to the device, you cannot brick it - the bootloader is stored in a protected area of flash.  If your new firmware doesn't boot, you'll get stuck at step (3) above, or with a crashed unit. Simply repeat the above process with the shipping firmware image (`resources/Launchpad Pro-1.0.154.syx`) to restore your unit to the factory defaults.  Better yet, fix the bugs :)

# The API
The API works in two directions - from the HAL (hardware abstraction layer) to the app, and from the app to the HAL.  The HAL calls into your app to:

- Receive user events from the pads and buttons
- Receive messages from the MIDI/USB ports
- Receive a tick message to drive timer based code
- Be notified when someone connects or disconnects a MIDI cable

By calling into the HAL, your app can:

- Write colours to the LEDs
- Send messages to the MIDI/USB ports

The best way to learn about these is to read the documentation in `app.h`, and to study the (very basic) example code!

Currently the HAL/app interface does not support reading or writing the flash memory.

# Debugging
We decided not to support or encourage using a hardware debugger, as opening a Launchpad Pro to fit a debugging header can easily damage the FSR (force sensitive resistor) sheet.

Instead, you're going to have to do things the old fashioned way - by blinking LEDs or sending MIDI messages (though hopefully no need for a 'scope!).  For what it's worth, that's the way I've developed this version of the firmware - dogfooding all the way ;)

If do you want to debug interactively (and of course you do), you can use the interactive desktop simulator on OS X.  

1. Build the Xcode project located in `/tools/osx`
2. Connect your Launchpad Pro
3. Install the factory firmware on your Launchpad Pro
4. Put the Launchpad Pro into "Programmer" mode using the Setup button (you'll get odd behaviour otherwise)
5. Start debugging in Xcode!

Currently it only supports button presses and LED messages - there's no virtual MIDI IO, setup button or aftertouch (yet).  It has a really awful busywaiting timer for the 1kHz tick.  However, it does allow you to debug your application logic using Xcode!

You can also use the simple command-line simulator located in the `/tools` directory.  It is compiled and ran as part of the build process, so it serves as a very basic test of your app before it is baked into a sysex dump - more of a test harness.

To debug the simulator interactively in Eclipse:

1. Click the down arrow next to the little "bug" icon in the toolbar
2. Choose "Debug configurations..."
3. Right click "C/C++ Application" and choose "New...:
4. Under "C/C++ Application" click Browse... and locate the simulator binary at `/vagrant/build/simulator`
5. Hit "Debug"!

# The Hardware
The Launchpad Pro is based around an ARM Cortex M3 from STMicroelectronics.  Specifically, an [STM32F103RBT6](http://www.st.com/web/catalog/mmc/FM141/SC1169/SS1031/LN1565/PF164487).  It's clocked at 72MHz, and has 20k RAM (I'm not sure how much of this we're using in the open build yet - should be a fair amount left but I haven't measured it).  The low level LED multiplexing and pad/switch scanning consume a fair bit of CPU time in interrupt mode, but have changed a little in the open firmware library (so again, I don't have measurements for how many cycles they're using).

It has 128k of flash memory, but I'm pretty sure we won't be exposing all of it as part of this API (dangerously easy to corrupt things!).  Current thinking is to expose a few pages for use by apps, though this would still introduce risks (accidentally wearing out sectors, for example).  To be continued...

# Vagrant tips
When you're done developing, simply type `vagrant suspend` to halt your VM without destroying it - this will make `vagrant up` a lot quicker next time.  If you're really finished, `vagrant destroy` will completely remove the VM from your system (but not any of your code).

If you only want to build using the command line, you might want to run your Vagrant box headless, which you can do by modifying the Vagrantfile: `vb.gui = false`.  You can also add more CPUs, RAM etc. if you want.

If prefer, you can install the gcc-arm toolchain on your local machine, or you might already have it.  You can find all you need [here](http://gnuarmeclipse.livius.net/).

It appears that VirtualBox does not yet work on Windows 10.

If your connection drops out while updating the Vagrant box, you can get stuck, unable to `vagrant up`.  To resolve, you need to delete the temp file - `~/vagrant.d/tmp`.

# Firmware development tips
OK - we're not going to need to use the MISRA rules, but there are a few things to avoid.  Dynamic memory allocation is a no (well it will work, but it's best avoided). Floating point will work, but it's implemented in software and will be slooooow.   C++ ought to work, but you'll definitely want to avoid exceptions and RTTI!
