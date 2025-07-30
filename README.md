# Layuh‑Roblox External (C++)

> [!CAUTION]
> This software is designed to interact with a game client in an unofficial manner. The use of such tools may violate the game's terms of service, which could result in penalties for the user's account, including suspensions or bans. The developer is not responsible for any misuse of this software.


## this project is a Re-Uploead & Fork of the original project(taken down)

This repository contains the source code for **Layuh**, a C++ project designed to interact with the Roblox environment. It focuses on implementing advanced functionalities, including in-game enhancements and tools for client interaction.

## Features

* **Aimbot:** Module for in-game aiming assistance.
* **ESP (Extra Sensory Perception):** Functionality that provides augmented visual information about the game environment and players (e.g., seeing through walls).
* **User Interface (GUI):** Implementation of an interactive graphical user interface for controlling features, built with ImGui and rendered via DirectX.
* **Authentication/Licensing System:** Integration with KeyAuth for managing software access.
* **Network Communication:** Uses the cURL library for HTTP requests.
* **JSON Data Handling:** Utilizes the nlohmann/json library for data serialization and deserialization.
* **Obfuscation and Protection:** Incorporates modules like Oxorany to hinder reverse engineering and code analysis.
* **Kernel/System-Level Interaction:** Contains modules suggesting low-level interactions with the operating system or the Roblox client.

## Project Structure

The project is organized as follows:

* **`Layuh/`**: Main project directory.
    * `entry.cpp`, `entry.h`: Main entry point and initialization logic.
    * `kernel/`: Modules related to kernel/driver-level interaction (`driver.cpp`, `driver.h`).
* **`Layuh/cheat/`**: Contains the core logic for the functionalities.
    * `aimbot/`: Source files for the Aimbot module.
    * `esp/`: Source files for the ESP module.
    * `globals/`: Global definitions and Roblox-specific headers (`roblox.h`).
    * `images/`: Image handlers.
    * `menu/`: Modules for the user interface.
        * `ui/imgui/`: ImGui library and its adapters for DirectX and Win32.
        * `ui/render/`: Rendering and overlay drawing logic.
    * `misc/`: Miscellaneous functions and utilities.
    * `utils/`: General utility functions.
* **`Layuh/cheat/include/`**: Third-party libraries and dependencies.
    * `curl/`: cURL library for HTTP requests.
    * `keyauth/`: KeyAuth library files for authentication.
    * `nlohmann/`: JSON handling library.
    * `oxorany/`: Files related to code obfuscation.
    * `zlib/`: Data compression library.

## feel free to use it as a base to your projects !


