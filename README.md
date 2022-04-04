# *cib* - Compile-time Initialization and Build

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)

## Overview

*cib* is a C++ header-only library for building embedded firmware with reusable
components. It implements the compile-time initialization and build pattern. 
Instead of initializing components and registering callbacks at runtime, this
process is executed at compile-time using constexpr or consteval functions.

Firmware using *cib* is implemented as a collection of **components**. Each 
component provides **services** and **features** to the build. Component
**features** extend and provide new functionality to **services**.