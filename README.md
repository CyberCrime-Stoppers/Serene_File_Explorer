# Serene File Explorer

A feature-rich, Dolphin-inspired file explorer for **SereneOS**, themed in the Serene palette
(`#9db29a` green / `#ad5ab4` purple), with light & dark modes. Built with **Qt6 (C++)** and
designed to integrate with KDE Frameworks 6 where available.

**SPDX-License-Identifier: GPL-3.0-or-later**

---

## Feature Roadmap

### Milestone 1 — Core Browsing Shell ✅ (this release)
- Tabbed main window, closable/movable tabs, one DirectoryView per tab
- Dual view modes: icon & list, sortable by name/size/modified
- Async directory loading via QThreadPool (UI never blocks)
- Cut / copy / paste with internal clipboard, rename, new folder/file
- Move to trash (Qt6 `QFile::moveToTrash`), permanent delete with confirmation
- Hybrid search: `locate` index when available, live recursive scan fallback
- Transfer engine with queue + progress bar in status bar (pause/resume/cancel hooks)
- Serene theme, light/dark, applied via a single stylesheet
- Breadcrumb-style editable path bar with Up / Back / Forward history
- Context menus on the file view
- Control Panel launcher (Tools menu) — launches `sereneos-control-panel` if installed
- Go menu with standard xdg user folders (Home/Documents/Downloads/Pictures/Music/Videos)

### Milestone 2 — Views & Media 🚧 (planned)
- **Photo Albums** — Windows-style album grid grouping photos by folder/date
- **Document Viewer** — built-in PDF/text/markdown viewer with advanced options
- **Video Viewer** — built-in player (QtMultimedia) with playlist
- **Picture Viewer** — zoom/rotate/fit, EXIF panel, fullscreen slideshow
- Sidebar with places, devices, bookmarks, tags

### Milestone 3 — Storage & Network 🚧 (planned)
- **Remote Access** — SFTP/SSH (via `gio` or libssh), SMB (via gvfs-smb), FTP(S), WebDAV
- **Cloud Drive Connections** — Nextcloud/WebDAV first-class; rclone bridge for
  Google Drive/OneDrive/Dropbox/S3 and 40+ more providers
- **Drive Manager** — mount/unmount/format (udisks2: list, SMART status, partition info),
  mount-point shortcuts, disk usage graphs
- **Network Manager** — detect shares (smb/discover), connect/manage bookmarks,
  NetworkManager (nmcli/nm-applet hooks)
- Better mount & unmount flows than stock file managers
- Moving directories to other locations, e.g. external drives

### Milestone 4 — Polish & Integration 🚧 (planned)
- Full Preferences dialog (view, search backend, theme, integration commands)
- Per-file Properties dialog with permissions/ownership tab
- Terminal-emulator panel inside the explorer
- Split view (two panes side-by-side)
- Extensible plugin interface (KIPI-style plugins)
- Thumbnailer service (images, videos, documents)

## Build Instructions

Requires: `cmake >= 3.21`, a C++17 compiler, Qt6 (`qt6-base` dev packages),
and `qt6-tools` for deployment tools.

### KDE Neon / Ubuntu
```bash
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools
cd Serene_File_Explorer
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/serene-file-explorer
```

### Arch-like base (SereneOS)
```bash
sudo pacman -S base-devel cmake qt6-base
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/serene-file-explorer
```

## Project Layout

```
Serene_File_Explorer/
├── CMakeLists.txt
├── LICENSE                      (GPL-3.0)
├── README.md
└── src/
    ├── main.cpp                 entry point, CLI, theming
    ├── core/
    │   ├── types.h              FileEntry + metatypes
    │   ├── filesystem.{h,cpp}   listing, trash, delete, create, unique naming
    │   ├── clipboard.{h,cpp}    internal cut/copy state
    │   └── settings.{h,cpp}     QSettings persistence
    ├── ui/
    │   ├── theme.{h,cpp}        Serene light/dark stylesheet
    │   └── mainwindow.{h,cpp}    tabs, menus, toolbar, status bar
    ├── models/
    │   └── filemodel.{h,cpp}    async list model + sort/filter
    ├── jobs/
    │   ├── listjob.{h,cpp}      background directory listing
    │   └── searchjob.{h,cpp}    hybrid locate/live-scan search
    ├── transfer/
    │   └── transfermanager.{h,cpp}  queued copy/move engine
    ├── views/
    │   └── directoryview.{h,cpp}    per-tab view + breadcrumbs
    ├── dialogs/                 (milestone 2+: properties, preferences)
    └── utils/                   (milestone 2+: mimetypes, thumbnails)
```

## Repository

Part of the [SereneForge](https://www.sereneforge.org) project —
GitHub: [CyberCrime-Stoppers/Serene_File_Explorer](https://github.com/CyberCrime-Stoppers)
· Codeberg: [CyberCrime-Stoppers/Serene_File_Explorer](https://codeberg.org/CyberCrime-Stoppers)
