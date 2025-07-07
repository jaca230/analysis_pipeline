# AnalysisPipeline

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()
[![CMake](https://img.shields.io/badge/CMake-3.14+-blue)]()

A modular, high-performance C++ framework for building and executing data analysis pipelines. Originally designed for processing MIDAS events into ROOT data products, AnalysisPipeline provides a flexible, JSON-configurable system for creating complex data processing workflows. Aimed to provide a lightweight, portable, flexible alternative to [Gaudi](https://gaudi.web.cern.ch/gaudi/).

## ✨ Key Features

- **📋 JSON Configuration**: Define entire pipelines through simple JSON files - no code recompilation needed
- **🔗 Directed Acyclic Graphs**: Build complex workflows with branching, merging, and parallel execution paths
- **🚀 Parallel Execution**: Leverages Intel TBB for high-performance, thread-safe parallel processing
- **🔌 Plugin Architecture**: Dynamically load custom processing stages at runtime using ROOT's reflection system
- **🌳 ROOT Integration**: Native support for ROOT data structures with thread-safe `TTree` access
- **📊 Built-in Data Products**: Automatic serialization and management of analysis results
- **🔧 Flexible Logging**: Comprehensive logging with configurable levels and outputs via spdlog

## 🏗️ Architecture

The framework consists of several key components:

- **Pipeline**: The main execution engine that builds and runs the workflow
- **ConfigManager**: Handles JSON configuration parsing and validation
- **Plugin System**: Dynamic loading of custom stages via ROOT dictionaries

## 📋 Requirements

- **C++17** compatible compiler
- **[ROOT](https://root.cern/)** (Core, Tree, RIO, Hist components)
- **[Intel TBB](https://github.com/oneapi-src/oneTBB)** for parallel execution
- **[spdlog](https://github.com/gabime/spdlog)** for logging
- **[nlohmann/json](https://github.com/nlohmann/json)** for JSON parsing
- **[analysis_pipeline_stages](https://github.com/jaca230/analysis_pipeline_stages)** for base stage classes
- **CMake 3.14+**

## 🚀 Quick Start

### 1. Clone and Build

```bash
# Clone with submodules
git clone --recursive https://github.com/jaca230/analysis_pipeline.git
cd analysis_pipeline

# Build the project
./scripts/build.sh
```

If you've already cloned without submodules:

```bash
git submodule update --init --recursive
```

### 2. Create Configuration Files

Create three JSON configuration files:

**`config/logger.json`** - Configure logging behavior:
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

**`config/plugins.json`** - Specify plugin libraries to load:
```json
{
  "plugin_libraries": [
    "../build/examples/example_plugin/libexample_pipeline_plugin.so"
  ]
}
```

**`config/pipeline.json`** - Define your processing pipeline:
```json
{
  "pipeline": [
    {
      "id": "data_generator",
      "type": "RandomDataGeneratorStage",
      "parameters": {
        "product_name": "random_values",
        "min": 0.0,
        "max": 10.0,
        "seed": 1234
      },
      "next": ["histogram_builder"]
    },
    {
      "id": "histogram_builder",
      "type": "TH1BuilderStage",
      "parameters": {
        "input_product": "random_values",
        "histogram_name": "random_hist",
        "value_key": "fVal",
        "title": "Random Data Histogram",
        "bins": 50,
        "min": 0.0,
        "max": 10.0
      },
      "next": []
    }
  ]
}
```

### 3. Run the Pipeline

```bash
./scripts/run.sh
```

## 📖 Configuration Guide

### Pipeline Configuration

Each stage in the pipeline must specify:

- **`id`**: Unique identifier for the stage
- **`type`**: Class name of the stage (must be available in ROOT dictionary)
- **`parameters`**: JSON object with stage-specific configuration
- **`next`**: Array of stage IDs to execute after this stage completes

### Plugin Libraries

Specify shared libraries containing custom stages. Libraries are loaded using ROOT's `gSystem->Load()` mechanism.


## 🏃‍♂️ Example Workflows

### Simple Linear Pipeline
```
DataReader → DataProcessor → HistogramBuilder → ResultWriter
```

### Branching Pipeline
```
DataReader → Processor1 → FinalStage
          ↘ Processor2 ↗
```

### Complex Multi-Branch Pipeline
```
Input → Filter → Analysis1 → Combiner → Output
             ↘ Analysis2 ↗
             ↘ Analysis3 ↗
```

## 🔧 Advanced Features

### Thread-Safe Data Sharing
All stages can safely access shared data products through the `PipelineDataProductManager`, which handles thread synchronization automatically.

### Dynamic Stage Loading
Stages are loaded at runtime using ROOT's reflection system, allowing for flexible plugin architectures without recompiling the main framework.

### Parallel Execution
The framework automatically parallelizes independent stages using Intel TBB's flow graph, maximizing CPU utilization.

### JSON Serialization
All data products can be automatically serialized to JSON for debugging, monitoring, or data export.

## 📊 Performance

- **Parallel Processing**: Automatic parallelization of independent stages
- **Memory Efficient**: Shared data products reduce memory overhead
- **Scalable**: Handles large datasets with configurable memory management
- **Low Latency**: Minimal overhead for stage transitions

## 📝 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.