# Music Magister

A simple command-line music player application for linux.

## Installation

### Dependencies

Music Magister depends on the C++ libraries mpg123 and ao.

These can be installed in Debian systems using apt-get.
```bash
sudo apt-get install libmpg123-dev libao-dev
```

These can be installed in Arch linux using pacman.
```bash
sudo pacman -S lib32-mpg123 libao
```

### Building

Download the source code and use the `make` command to build the source. The application will be built into the *bin* directory in the folder containing the source code.
```bash
make
```

To install the application, so you may run it from any path, you can use the `make install` command.
```bash
sudo make install
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

`prev`
Skip back to previous song in the playlist.

`next`
Skip to next song in the playlist.

#### Configuration

The configuration file `.mmconfig` is located in the `home` directory.
