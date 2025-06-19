# AnalysisPipeline

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

## Overview

**AnalysisPipeline** is a modular C++ framework to define and execute analysis pipelines for processing ROOT data. It's main use case is processing MIDAS events into ROOT data products. Pipelines are defined via JSON configurations describing a directed graph of algorithm stages, which are dynamically instantiated from ROOT dictionaries. Execution uses Intel TBB for parallelism and supports thread-safe shared ROOT `TTree` access.

---

## Features

* JSON-configured, directed acyclic graph of algorithm stages
* Dynamic loading of stages via ROOT reflection (`TClass::New`)
* Parallel execution using Intel TBB flow graph
* Thread-safe shared ROOT `TTree` access for all stages
* Flexible logging configuration with `spdlog`
* Extensible architecture with stages implemented as ROOT `TObject`-derived plugins

---

## Requirements

* C++17 compatible compiler
* [ROOT](https://root.cern/) (with Core, Tree, RIO, Hist components)
* [MIDAS](https://midas.triumf.ca/) data acquisition system — must be installed and environment variable `MIDASSYS` set
* [Intel TBB](https://github.com/oneapi-src/oneTBB)
* [spdlog](https://github.com/gabime/spdlog)
* [nlohmann/json](https://github.com/nlohmann/json)
* [analysis_pipeline_stages](https://github.com/jaca230/analysis_pipeline_stages)
* CMake 3.14 or higher

---

## Getting Started

### Clone repository with submodules

```bash
git clone --recursive https://github.com/jaca230/analysis_pipeline.git
cd analysis_pipeline
```

If already cloned without submodules:

```bash
git submodule update --init --recursive
```

### Build

Simply run the provided build script to configure and build the project:

```bash
./scripts/build.sh
```

This will build the submodules and library.

---

## Usage

1. Prepare a JSON pipeline configuration describing the stages and their connections.

2. Run the pipeline executable with your config. The pipeline loads stages dynamically, builds the TBB flow graph, and executes the workflow.

3. Results are dynamically added to a `TTree` accessible by all stages.

---

Here's how you can include those two JSON snippets as **example configuration files** in your README. I formatted them clearly and added brief explanations so users know what they are:

---

## Example Configuration

### Logger Configuration (`config/logger_config.json`)

This JSON configures the logging system using spdlog, specifying log level, pattern, and sinks:

```json
{
  "logger": {
    "name": "Pipeline_Logger",
    "level": "info",
    "pattern": "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v",
    "sinks": {
      "console": {
        "enabled": true,
        "color": true
      },
      "file": {
        "enabled": false,
        "filename": "logs/app.log",
        "max_size": 10485760,
        "max_files": 3
      }
    }
  }
}
```

* **name**: Logger instance name.
* **level**: Minimum log level (`info` here).
* **pattern**: Log message format.
* **sinks.console.enabled**: Enable colored console logging.
* **sinks.file.enabled**: File logging disabled by default.
* **sinks.file.filename**: Path for the log file (if enabled).
* **max\_size** and **max\_files**: Log file rotation parameters.

---

### Pipeline Configuration (`config/pipeline_config.json`)

This JSON defines a simple pipeline of 4 stages, demonstrating branching and parameters:

```json
{
  "pipeline": [
    {
      "id": "stage0",
      "type": "DummyStage",
      "parameters": {
        "message": "Hello from stage0"
      },
      "next": ["stage1a", "stage1b"]
    },
    {
      "id": "stage1a",
      "type": "DummyStage",
      "parameters": {
        "message": "Hello from stage1a",
        "sleep_ms": 1000
      },
      "next": ["stage2"]
    },
    {
      "id": "stage1b",
      "type": "DummyStage",
      "parameters": {
        "message": "Hello from stage1b",
        "sleep_ms": 500
      },
      "next": ["stage2"]
    },
    {
      "id": "stage2",
      "type": "DummyStage",
      "parameters": {
        "message": "Hello from stage2"
      },
      "next": []
    }
  ]
}
```

* Each stage has a unique **id**.
* **type** specifies the class name of the stage plugin.
* **parameters** are JSON objects passed to the stage on initialization.
* **next** is an array of stage IDs to run after this one, enabling branching.


---

## Project Structure

* `src/` - Core pipeline source files
* `include/` - Public headers including stages
* `external/` - Dependencies as git submodules
* `scripts/` - Build and utility scripts
* `configs/` - Example JSON configuration files

---

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
