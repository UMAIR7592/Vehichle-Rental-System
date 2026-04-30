<div align="center">

```
╔══════════════════════════════════════════════════════════════════╗
║          V  E  L  O  C  E  —  Vehicle Rental Management         ║
║                   Enterprise Edition  ·  v2.0.0                  ║
╚══════════════════════════════════════════════════════════════════╝
```

**A production-grade, terminal-based Vehicle Rental Management System**  
built with modern **C++17** — featuring a full booking lifecycle, dynamic billing engine,  
customer registry, insurance tiers, and a rich ANSI-rendered UI.

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![Build](https://img.shields.io/badge/build-passing-brightgreen?style=flat-square)]()
[![License](https://img.shields.io/badge/license-MIT-orange?style=flat-square)]()
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey?style=flat-square)]()

</div>

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Fleet Catalog](#fleet-catalog)
- [Getting Started](#getting-started)
- [Usage Guide](#usage-guide)
- [Billing Engine](#billing-engine)
- [Booking Lifecycle](#booking-lifecycle)
- [Project Structure](#project-structure)
- [Design Patterns & C++17 Features](#design-patterns--c17-features)
- [Contributing](#contributing)

---

## Overview

**Veloce** is a fully self-contained, single-file C++17 application that simulates a professional vehicle rental management system. It is designed to demonstrate enterprise-level software architecture in a terminal environment — complete with ANSI color rendering, structured exception handling, domain-driven design, and a multi-stage billing engine.

This project was written as a refactor/upgrade from a beginner prototype to a production-quality codebase. It is ideal as a portfolio piece, a learning reference, or a foundation for a real-world rental management CLI tool.

---

## Features

### Core Functionality
- 🚗  **Vehicle Catalog** — 4 vehicle types (Car, Bike, SUV, Van) with 14+ models across Economy, Sport, Luxury, and Hypercar tiers
- 👤  **Customer Registry** — Register customers with full name, phone, and CNIC/passport; each customer gets a unique `CUS-XXXX` ID
- 📋  **Booking Management** — Create, list, search, and return bookings; each booking has a unique `VRS-XXXXX` ID
- 🔄  **Full Booking Lifecycle** — `PENDING → ACTIVE → RETURNED / CANCELLED` state machine
- 🧾  **Professional Invoice** — Itemized final invoice with base rent, insurance, late fees, damage charges, and deposit deduction
- 📊  **Analytics Dashboard** — Live stats: total customers, bookings, lifetime revenue, fleet overview

### Billing Engine
- 💰  **Base Rent** — Daily rate × actual days used
- 🛡   **Insurance Tiers** — None / Basic (Rs. 200/day) / Premium (Rs. 500/day)
- ⚠️  **Damage Challan** — Flat penalty per vehicle type if returned damaged
- ⏱   **Late Return Fee** — Per-day overdue charge when actual days exceed planned days
- 🏦  **Security Deposit** — Collected at booking, deducted from final invoice

### Terminal UI
- ANSI 256-color rendering with `╔═╗` box frames and `╠═╣` dividers
- Color-coded status badges: ACTIVE (cyan), RETURNED (green), CANCELLED (red)
- Section headers, key-value tables, and formatted itemized invoices
- Input validation with clear, styled error prompts
- ISO-8601 timestamps on all records

---

## System Architecture

```
VehicleRentalSystem.cpp
│
├── SECTION 1  ── ANSI::           Terminal color/style renderer
├── SECTION 2  ── VRS::            Custom exception hierarchy
├── SECTION 3  ── VRS::            Domain models (Booking, Customer, VehicleType, etc.)
├── SECTION 4  ── VRS::Utils::     Input handling, formatting, ID generation
├── SECTION 5  ── VRS::UI::        Terminal layout primitives
├── SECTION 6  ── VRS::            VehicleCatalog (static fleet database)
├── SECTION 7  ── VRS::            CustomerRegistry
├── SECTION 8  ── VRS::            BookingEngine (create, return, list, lookup)
├── SECTION 9  ── VRS::            Dashboard (analytics)
├── SECTION 10 ── VRS::            VeloceApp (main controller / event loop)
└── SECTION 11 ── main()           Entry point with top-level exception guard
```

### Class Relationships

```
VeloceApp
 ├── VehicleCatalog       (immutable fleet data)
 ├── CustomerRegistry     (owns Customer map)
 └── BookingEngine        (owns Booking map, refs VehicleCatalog)
      └── Booking          (state machine + billing logic)
           └── InsuranceTier, BookingStatus  (enums)
```

---

## Fleet Catalog

| Type | Models | Damage Challan | Late Fee/Day |
|------|--------|---------------|-------------|
| 🚗 Car  | Lamborghini Huracán, Ferrari 488, BMW 5 Series, Honda Civic, Toyota Corolla | Rs. 8,000  | Rs. 1,500 |
| 🏍 Bike | Yamaha R1, Honda CG125, Ducati Panigale V4, Harley-Davidson Iron 883 | Rs. 5,000  | Rs. 800   |
| 🚙 SUV  | Toyota Fortuner, Kia Sportage, Audi Q7, Land Cruiser V8 | Rs. 12,000 | Rs. 2,000 |
| 🚐 Van  | Toyota HiAce, Ford Transit, Mercedes Vito | Rs. 10,000 | Rs. 2,500 |

---

## Getting Started

### Prerequisites

- **GCC** 7+ or **Clang** 5+ with C++17 support
- A terminal emulator with ANSI color support (Linux/macOS default terminals, Windows Terminal, or VS Code terminal)

### Compile

```bash
g++ -std=c++17 -O2 -Wall -Wextra -o veloce VehicleRentalSystem.cpp
```

> **Clang alternative:**
> ```bash
> clang++ -std=c++17 -O2 -o veloce VehicleRentalSystem.cpp
> ```

> **Windows (MinGW):**
> ```bash
> g++ -std=c++17 -O2 -o veloce.exe VehicleRentalSystem.cpp
> ```

### Run

```bash
./veloce
```

---

## Usage Guide

### Main Menu

```
  [1]  🚗  New Rental Booking
  [2]  🔄  Process Vehicle Return
  [3]  📋  View All Bookings
  [4]  🔍  Lookup Booking by ID
  [5]  👤  Register New Customer
  [6]  📊  Analytics Dashboard
  [7]  🚪  Exit System
```

### Creating a Booking

1. Select **[1] New Rental Booking**
2. Register a new customer or enter an existing Customer ID
3. Choose a vehicle type → model → rental duration
4. Select an insurance tier (None / Basic / Premium)
5. Review the booking summary and confirm
6. Your unique Booking ID (e.g. `VRS-47821`) is issued

### Returning a Vehicle

1. Select **[2] Process Vehicle Return**
2. Enter the Booking ID (e.g. `VRS-47821`)
3. Enter actual days used
4. Indicate if the vehicle was damaged
5. A full itemized invoice is printed with the net amount due

### Booking ID Format

```
VRS-XXXXX
 │    └── 5-digit random numeric suffix
 └──── System prefix
```

### Customer ID Format

```
CUS-XXXX
 │    └── 4-digit random numeric suffix
 └──── Customer prefix
```

---

## Billing Engine

The final invoice is computed as:

```
GROSS TOTAL  =  Base Rent
             +  Insurance Cost
             +  Damage Charge      (if applicable)
             +  Late Return Fee    (if actual days > planned days)

NET AMOUNT DUE  =  GROSS TOTAL  −  Security Deposit
```

### Insurance Tiers

| Tier    | Daily Rate | Coverage          |
|---------|-----------|-------------------|
| None    | Rs. 0     | No coverage        |
| Basic   | Rs. 200   | Liability only     |
| Premium | Rs. 500   | Full comprehensive |

### Late Return Fee Example

| Vehicle | Planned | Actual | Overdue Days | Late Fee Rate | Penalty  |
|---------|---------|--------|-------------|--------------|---------|
| Fortuner | 5 days  | 8 days | 3 days       | Rs. 2,000/day | Rs. 6,000 |

---

## Booking Lifecycle

```
  [CREATE]
     │
     ▼
  PENDING ──── user cancels ────► CANCELLED
     │
     │  confirmed
     ▼
  ACTIVE  ──── processReturn() ──► RETURNED
                                       │
                                       ▼
                                  Invoice Generated
```

---

## Project Structure

```
VehicleRentalSystem.cpp     Single-file application (~1,070 lines)
README.md                   This file
```

> The entire system is intentionally kept as a single, self-contained `.cpp` file for portability and ease of submission/deployment. In a real production system, each section/class would be split into its own `.hpp`/`.cpp` pair.

---

## Design Patterns & C++17 Features

### Design Patterns Used

| Pattern    | Where Applied                                              |
|------------|------------------------------------------------------------|
| Factory    | `VehicleCatalog::build()` constructs the full fleet        |
| Registry   | `CustomerRegistry` and `BookingEngine` as central stores   |
| State      | `BookingStatus` enum drives the booking lifecycle          |
| Strategy   | `InsuranceTier` selects cost strategy at billing time      |
| Facade     | `VeloceApp` provides a single controller for all subsystems|

### C++17 Features Used

| Feature            | Usage                                               |
|--------------------|-----------------------------------------------------|
| `std::optional`    | `CustomerRegistry::find()` returns optional ref     |
| `std::string_view` | Exception constructors and utility functions        |
| `std::unordered_map` | O(1) average lookup for customers and bookings   |
| Structured bindings| `for (const auto& [id, b] : bookings_)`             |
| `constexpr`        | ANSI color codes as compile-time string constants   |
| `std::chrono`      | ISO-8601 timestamp generation                       |
| `std::mt19937`     | Mersenne Twister for unique ID generation           |
| RAII               | All resources managed through stack/smart ownership |
| Move semantics     | `std::move` on strings and structs during insertion |

---

## Exception Hierarchy

```
std::runtime_error
 └── VRSException
      ├── InvalidInputException          — bad user input / out-of-range index
      ├── BookingNotFoundException       — unknown booking ID queried
      └── VehicleUnavailableException    — reserved for inventory extension
```

All exceptions are caught at the top-level `VeloceApp::run()` event loop, printed with a styled error message, and the system recovers gracefully without crashing.

---

## Contributing

Contributions, feature requests, and bug reports are welcome.

Suggested enhancements for future versions:

- [ ] Persist data to JSON / SQLite between sessions
- [ ] Multi-user / staff role access control
- [ ] Availability calendar (prevent double-booking)
- [ ] Email/SMS receipt generation
- [ ] Vehicle maintenance scheduling
- [ ] Web frontend via a REST API wrapper

---

<div align="center">

Built with ❤️ and modern C++17  
*"Write code as if the next person to read it is a senior engineer on a bad day."*

</div>
