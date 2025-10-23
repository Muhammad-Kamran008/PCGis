Of course! This is a highly specialized and impressive plugin. A good README will showcase the technical depth while making its power and utility clear to potential users and employers.

Here is an attractive, portfolio-ready README for your GitHub repository.

---

# 🗺️ PCGis: Procedural GIS Data Loader for Unreal Engine & Cesium

**Bridge the gap between Geographic Information Systems (GIS) and high-fidelity Unreal Engine worlds.** PCGis is a powerful plugin that seamlessly imports real-world GIS data from Shapefiles and procedurally generates vast, dynamic landscapes directly onto Cesium's global 3D terrain.

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5+-%23313131?logo=unrealengine)](https://www.unrealengine.com)
[![Cesium](https://img.shields.io/badge/Cesium-Compatible-blue)](https://cesium.com/)
[![Plugin](https://img.shields.io/badge/Type-Unreal%20Plugin-black)](https://www.unrealengine.com/marketplace/en-US/store)

---

## ✨ Features

*   **Dual Data Type Support:** Effortlessly handle both **Point Data** (for placing specific assets like buildings, trees, and rocks) and **Polygon Data** (for procedural generation of forests, foliage, and other area-based content).
*   **Seamless Cesium Integration:** Automatically clamps all spawned assets and procedural points to the Cesium terrain, whether you're using Cesium ion's streaming terrain or Google Photorealistic 3D Tiles.
*   **Procedural Content Generation (PCG) Powered:** Leverages Unreal Engine 5's powerful PCG framework to generate millions of instances with complex rules, all perfectly aligned to the real-world terrain.
*   **Smart SQLite Caching System:** A custom-built caching layer stores generated point data. On subsequent loads, data is read from the cache for near-instantaneous regeneration, eliminating costly re-processing and stabilizing frame rates.
*   **High-Performance Runtime Generation:** Utilizes Unreal Engine's Hierarchical Instanced Static Mesh (HISM) components for optimal performance, allowing you to render millions of procedurally placed objects efficiently.
*   **Asynchronous Processing:** Manages the heavy lifting of terrain height sampling asynchronously to prevent engine lock-ups, providing a smoother user experience during initial processing.

---

## 🚀 How It Works

PCGis operates through an intelligent, two-stage pipeline designed for both flexibility and performance.

### Stage 1: Initial Processing & Caching (On Data Change)
1.  **Data Ingestion:** Loads `.shp` Shapefiles containing your Point or Polygon data.
2.  **Point Processing (For Polygons):** For polygon data, a custom PCG node samples the interior based on density attributes and uses Cesium's `SampleHeightMostDetailed` function to clamp each point to the terrain.
3.  **SQLite Cache Population:** The calculated world positions (georeferenced) for all points are stored in a local SQLite database. This one-time process ensures future loads are lightning-fast.

### Stage 2: High-Performance Runtime Generation
1.  **Cache Check:** On load, the plugin checks if the source Shapefile is unmodified.
2.  **Database-Driven Spawning:** If a cache exists, it bypasses heavy computation and reads the pre-calculated points directly from the SQLite database.
3.  **Efficient Actor Spawning:** Spawns actors with PCG components that use the cached data to populate the scene with Hierarchical Instanced Static Meshes, resulting in minimal performance impact and buttery-smooth frame rates.

---

## 🛠️ Technical Architecture

```
[Shapefile (.shp)]
        |
        v
[PCGis Plugin Core]
        |
        +---> [Point Data Path] ---> [Spawn Actor at Lat/Long (Clamped to Terrain)]
        |
        +---> [Polygon Data Path] ---> [Custom PCG Node] ---> [Cesium Height Sampling]
                    |                                          |
                    |                                          v
                    |                               [SQLite Database (Cache)]
                    |                                          |
                    v                                          |
        [Spline + PCG Component Actor] <-----------------------+
                    |
                    v
        [Runtime HISM Generation]
```

---

## 📸 Screenshots / Gallery

*(You'll place your screenshots and videos here)*

> **Example 1:** A cultural site with point-data-placed assets (stones, walls) and procedurally generated foliage inside polygon boundaries.
> **Example 2:** A vast forest generated from a land cover polygon Shapefile, showing millions of trees perfectly clamped to a mountainous Cesium terrain.
> **Example 3:** A video/GIF demonstrating the performance difference between the first (processing) run and the second (cached) run.

---

## 🚀 Installation & Usage

### Prerequisites
*   Unreal Engine 5.1+
*   [Cesium for Unreal](https://cesium.com/platform/cesium-for-unreal/) Plugin
*   A Cesium ion account (for terrain/imagery)

### Installation
1.  Clone this repository into your project's `Plugins` folder.
2.  Rebuild your project in Visual Studio or Xcode.
3.  Launch the editor and ensure `PCGis` is enabled in the Plugins list.

### Quick Start
1.  **Add a PCGis Manager** actor to your level.
2.  **Configure your Cesium Georeference.**
3.  **For Point Data:**
    *   Provide the path to your Point Shapefile.
    *   Map model names from the attribute table to Unreal Static Meshes.
4.  **For Polygon Data:**
    *   Provide the path to your Polygon Shapefile.
    *   Assign a PCG Graph that uses the included `PCG Cesium Clamp` node.
    *   Define density and other parameters.

---

## 🗺️ Use Cases

*   **Urban Planning & Architecture:** Visualize real-world city data with accurately placed buildings, trees, and street furniture.
*   **Film & Virtual Production:** Create massive, authentic digital backlots based on real geographic regions.
*   **Simulation & Training:** Generate high-fidelity environments for flight, driving, or military simulations.
*   **Game Development:** Populate open-world game maps with realistic, data-driven vegetation and cultural features.

---

<img width="1482" height="950" alt="image" src="https://github.com/user-attachments/assets/c5fa6db4-40b1-44c7-b056-dac0aed134b0" />
<img width="1484" height="956" alt="image" src="https://github.com/user-attachments/assets/57c0a18d-fcdb-4bca-a148-be0556c9bfe3" />
<img width="1485" height="959" alt="image" src="https://github.com/user-attachments/assets/e5292646-c2e2-403c-ac1a-d5d131285477" />
