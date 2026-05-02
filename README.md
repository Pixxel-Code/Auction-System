# AuctionHub 🏷️

A desktop auction system built with **C++** and **Qt**, featuring real-time bidding, role-based dashboards, a persistent file-based database, and an in-app notification system.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Building the Project](#building-the-project)
  - [Running the App](#running-the-app)
- [Usage](#usage)
  - [Default Admin Credentials](#default-admin-credentials)
  - [Roles](#roles)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

AuctionHub is an academic C++ project that simulates a full-featured online auction platform. It uses the **Qt framework** for its graphical interface and stores all data in flat files, with no external database dependency. The system supports three user roles — Buyer, Seller, and Admin — each with a dedicated dashboard.

---

## Features

### Authentication
- User signup with role selection (Buyer / Seller)
- Secure login with credential validation
- Auto-created default admin account on first run

### Buyer
- Browse and search active auction listings (by title, category, or price range)
- Place bids with real-time outbid detection
- Watchlist management (add/remove items)
- View item descriptions
- In-app notifications (outbid alerts, auction results)
- Rate sellers after a completed auction

### Seller
- Create auction listings with title, description, category, starting price, and custom duration
- View and manage active listings
- Manually close auctions
- In-app notifications (new bids, auction results)
- Rate buyers after a completed auction

### Admin
- View all users, items, and bids
- Delete users and items (with cascading bid removal)
- Rate any user
- User reputation statistics

### Auction Engine
- Time-based auction expiry
- Automatic winner determination on close
- Outbid notifications to previous highest bidder
- Post-auction rating prompts for both buyer and seller
- Sound effects for bids, button clicks, and notifications (Windows)

---

## Architecture

The project follows a **layered service architecture**:

```
UI Layer          → LoginWindow, BuyerDashboard, SellerDashboard, AdminDashboard
Service Layer     → AuthService, AuctionService, ItemService, AdminService
Domain Layer      → Auction, AuctionManager, Item, Bid, User, Notification
Persistence Layer → DBManager (flat-file storage)
Utilities         → Validator, SoundManager
```

`DBManager` is implemented as a **Singleton** and acts as the central data store, loading from and writing to flat files on disk.

---

## Project Structure

```
AuctionHub/
│
├── main_gui.cpp              # Entry point — boots Qt app, loads DB, seeds admin
│
├── MainWindow.{h,cpp}        # Root window — manages page stack & routing
├── LoginWindow.h             # Login / Signup UI (landing, login, signup pages)
│
├── BuyerDashboard.{h,cpp}    # Buyer UI — browse, bid, watchlist, notifications
├── SellerDashboard.{h,cpp}   # Seller UI — listings, create item, notifications
├── AdminDashboard.h          # Admin UI — users, items, bids, rating
│
├── Auction.{h,cpp}           # Core auction logic — bidding, closing, notifications
├── AuctionManager.{h,cpp}    # Manages collection of active auctions
├── AuctionService.{h,cpp}    # Service wrapper for placing bids
│
├── Item.{h,cpp}              # Item model — price, expiry, description, category
├── Bid.{h,cpp}               # Bid model
├── User.{h,cpp}              # User model — roles, reputation, watchlist
├── Notification.{h,cpp}      # Notification model
│
├── AuthService.{h,cpp}       # Signup & login logic
├── ItemService.{h,cpp}       # Item creation logic
├── AdminService.{h,cpp}      # Admin actions — delete, rate, view
│
├── DBManager.{h,cpp}         # Singleton — flat-file persistence for all entities
├── Validator.h               # Input validation utilities
└── Soundmanager.h            # Windows sound effects (bid ding, click, notification)
```

---

## Getting Started

### Prerequisites

| Requirement | Version |
|---|---|
| C++ Compiler | C++17 or later (MSVC recommended on Windows) |
| Qt Framework | Qt 5.x or Qt 6.x |
| OS | Windows (required for sound effects via `winmm`) |

> **Note:** Sound support uses the Windows Multimedia API (`winmm.lib`). The app will compile on other platforms but sound effects will be unavailable.

### Building the Project

**Using Qt Creator (recommended):**

1. Open Qt Creator and select **File → Open Project**.
2. Open the `.pro` or `CMakeLists.txt` file at the project root.
3. Configure the kit (MSVC + Qt).
4. Add `winmm` to your linker dependencies:
   - In `.pro` file: `LIBS += -lwinmm`
   - In MSVC: Project Properties → Linker → Additional Dependencies → add `winmm.lib`
5. Click **Build** (Ctrl+B).

**Using qmake from the command line:**

```bash
qmake AuctionHub.pro
nmake        # on Windows with MSVC
```

### Running the App

Place the following sound files in the same directory as the executable:

```
bid_ding.wav
btn_click.wav
notification.wav
```

Then run:

```bash
./AuctionHub.exe
```

On first launch, a default admin account is created automatically if no users exist.

---

## Usage

### Default Admin Credentials

```
Username: admin
Password: admin123
```

### Roles

| Role | Capabilities |
|---|---|
| **Buyer** | Browse listings, place bids, manage watchlist, rate sellers |
| **Seller** | Create listings, manage auctions, rate buyers |
| **Admin** | View/delete all data, rate any user, monitor platform |

To register as a **Seller**, select the Seller role during signup. Buyers are the default role.

---

## Contributing

This is an academic project, but contributions are welcome.

1. Fork the repository.
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m "Add your feature"`
4. Push to the branch: `git push origin feature/your-feature`
5. Open a Pull Request.

Please keep code style consistent with the existing codebase and ensure the project builds cleanly before submitting a PR.

---

## License

This project is for academic/educational purposes. No license has been explicitly applied — please contact the repository owner before reusing or distributing.
