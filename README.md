<h1 align="center">
  <br>
  Shelly Recorder
  <br>
</h1>

<h4 align="center">A minimal screen recording application built with <a href="https://www.qt.io/" target="_blank">Qt</a> and FFmpeg.</h4>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#how-to-use">How To Use</a> •
  <a href="#download">Download</a> •
  <a href="#license">License</a>
</p>

![screenshot]([YOUR_SCREENSHOT_GIF])

## Features

* Screen Recording
  - Fullscreen or custom area selection
  - Adjustable quality and FPS
  - Support for multiple monitors
  - Region selection with resizable window
* Audio Recording
  - System audio capture
  - Multiple device support
  - Audio device selection
  - Volume level monitoring
* Recording Controls
  - Pause/Resume functionality
  - Stop and save options
  - Recording timer display
  - File format selection
* Minimal Interface
  - Clean and intuitive design
  - Dark/Light theme support
  - Customizable hotkeys
  - System tray integration

## Requirements

* Qt 6.2.0 or higher
* FFmpeg 4.0 or higher
* C++17 compatible compiler
* CMake 3.16 or higher

## How To Use

```bash
# Clone this repository
$ git clone https://github.com/sabari-tear/shelly-Recorder.git

# Install dependencies
$ sudo apt-get install qt6-base-dev ffmpeg libavcodec-dev libavformat-dev libavutil-dev

# Build the project
$ qmake
$ make

# Run the application
$ ./Shall_be_recorded
```

## Configuration

The application can be configured through the settings menu:
* Video quality settings
* Audio device selection
* Output format preferences
* Hotkey customization
* Theme selection

## Download

Download the latest version for Windows [here]([YOUR_DOWNLOAD_LINK]).

## License

MIT

---

> GitHub [@sabari-tear](https://github.com/sabari-tear)
