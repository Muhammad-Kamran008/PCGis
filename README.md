# PCGis – Procedural GIS Integration Plugin for Unreal Engine

**PCGis** is an Unreal Engine plugin that integrates **GIS shapefile data** with **Cesium 3D terrain streaming** and Unreal’s **Procedural Content Generation (PCG)** framework.
It enables scalable, automated, and geospatially accurate **procedural world generation** directly from real-world GIS data.

---

## Overview

PCGis bridges the gap between **geospatial data** and **real-time 3D visualization**.
It processes shapefile data (both **point** and **polygon** types), generates procedural content on Cesium terrain, and uses an internal **SQLite-based caching system** to achieve high performance for large-scale datasets.

The plugin supports workflows that can efficiently spawn millions of procedural assets—such as vegetation, cultural objects, or infrastructure—accurately clamped to real-world terrain.

---

## Core Features

### 1. GIS Data Integration

* Supports **Point** and **Polygon** shapefile data.
* Automatically extracts asset names, coordinates, and metadata.
* Converts geospatial data into Unreal Engine world positions using Cesium’s coordinate conversion utilities.

### 2. Cesium Terrain Compatibility

* Fully compatible with **Cesium World Terrain** and **Google Photorealistic 3D Tilesets**.
* Ensures accurate height placement via the `CesiumSampleHeightMostDetailed` API.
* Integrates smoothly with Cesium’s tile-based streaming for efficient large-area rendering.

### 3. Point-Based Procedural Spawning

* Reads point-type shapefile features to spawn cultural or environmental assets such as:

  * Trees, shrubs, stones, walls, buildings, and more.
* Uses asset or mesh names defined in shapefile attributes.
* Employs asynchronous height sampling with caching for accurate, terrain-aligned placement.

### 4. Polygon-Based Procedural Generation

* Handles polygon data to procedurally fill defined regions (e.g., vegetation or foliage areas).
* Creates **Actors** with **Spline Components** and **PCG Components** for each polygon.
* Includes a **custom PCG node** that samples interior points based on density values from the shapefile.
* Points are clamped to terrain using Cesium’s height functions before spawning content.

### 5. SQLite Data Caching

* Integrated **SQLite** system stores generated procedural points.
* On subsequent runs, if the shapefile is unchanged, PCGis loads data from the database rather than recalculating.
* This reduces runtime cost, avoids FPS drops, and supports **hierarchical runtime generation**.
* Enables efficient regeneration and loading of **millions of instances**.

<img width="1482" height="950" alt="image" src="https://github.com/user-attachments/assets/c5fa6db4-40b1-44c7-b056-dac0aed134b0" />
---
<img width="1484" height="956" alt="image" src="https://github.com/user-attachments/assets/57c0a18d-fcdb-4bca-a148-be0556c9bfe3" />
---
<img width="1485" height="959" alt="image" src="https://github.com/user-attachments/assets/e5292646-c2e2-403c-ac1a-d5d131285477" />

---

## System Architecture

| Pipeline       | Purpose                                                                                    | Execution Stage                                |
| -------------- | ------------------------------------------------------------------------------------------ | ---------------------------------------------- |
| **Pipeline 1** | Reads new/updated shapefiles, generates terrain-clamped points, and stores them in SQLite. | First run (data preparation phase)             |
| **Pipeline 2** | Loads precomputed points from SQLite and spawns runtime hierarchical PCG actors.           | Subsequent runs (optimized runtime generation) |

---

## Performance Highlights

* Optimized for large GIS datasets (thousands of polygons, millions of points).
* Database caching eliminates redundant Cesium height sampling.
* Runtime hierarchical PCG generation ensures smooth performance.
* Designed for use in **simulation**, **urban planning**, and **geospatial visualization** projects.

---

## Technical Stack

* **Engine:** Unreal Engine 5.4
* **Terrain System:** Cesium for Unreal
* **Procedural Framework:** Unreal PCG
* **Database:** SQLite
* **GIS Data:** ESRI Shapefile (Point and Polygon types)

---

## Example Applications

* Real-world environment reconstruction and digital twin creation.
* Simulation and training environments requiring accurate terrain data.
* City-scale visualization of infrastructure, vegetation, or urban assets.
* Research and development in procedural content generation with geospatial datasets.

---


