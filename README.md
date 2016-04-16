# Music Magister

A simple music player application for linux terminal.

## Installation

### Dependencies

Music Magister depends on the C++ libraries mpg123 and ao. These can be installed in Debian systems using apt-get.

```bash
sudo apt-get install libmpg123-dev libao-dev
```

### Building

Download the source code and use the `make` command to build the source.

```bash
make clean
make
```

The application will be built into the *bin* directory in the folder containing the source code.

To install the application, so you may run it from any path, you can use the `make install` command.

```bash
make install
```

## Usage

Open up the terminal and run `mm` with appropriate command to use the player.

```bash
mm <command>
```

#### List of commands

`start`
Start the music player.

`stop`
Stop the music player.

`next`
Skip to next song in the playlist. <br>
