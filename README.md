<div align="center">

# 🃏 FlipDeck

### Loupedeck-style USB HID PC Controller for Flipper Zero

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-orange.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Built with ufbt](https://img.shields.io/badge/Built%20with-ufbt-blueviolet)](https://github.com/flipperdevices/flipperzero-ufbt)
[![Flipper Zero](https://img.shields.io/badge/Flipper%20Zero-FAP%20App-ff6600)](https://flipperzero.one)
[![API](https://img.shields.io/badge/API-87.1-green)](#)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](#)

<br/>

> Turn your Flipper Zero into a compact **5-button macro pad** — control media, system shortcuts, browser, VS Code, OBS, gaming, Photoshop, Window management and custom text macros over USB HID. No drivers required.

<br/>

```
┌─────────────────────────────┐
│  [1/9] MEDIA   ┌──────┐     │  ← last-action flash pill
│          Vol+  │ Vol+ │     │
│  Prev   Play/Pause   Next   │
│          Vol-               │
│  Back:Menu      LongL/R:Pg  │
└─────────────────────────────┘
  Flipper Zero 128×64 OLED
  v2.0 — 9 pages · 45 shortcuts
```

</div>

---

## 📋 Table of Contents

- [✨ Features](#-features)
- [🆕 What's New in v2](#-whats-new-in-v2)
- [🗂️ Pages & Shortcuts](#️-pages--shortcuts)
- [🕹️ Navigation](#️-navigation)
- [🖥️ UI Layout](#️-ui-layout)
- [💾 SD Card Macros](#-sd-card-macros)
- [🌍 Language File](#-language-file)
- [⚙️ Customisation](#️-customisation)
- [🔨 Building & Flashing](#-building--flashing)
- [📁 Project Structure](#-project-structure)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

---

## ✨ Features

| Feature | Details |
|---|---|
| 🎛️ **9 control pages** | Media · System · Browser · VS Code · OBS · Macro · Gaming · Photoshop · Win WM |
| 🔑 **45 shortcuts** | 5 directional actions per page |
| ⌨️ **Rich text macros** | Full symbol support — `_!#$%^&*()=?;'"` and tab |
| 💾 **SD card macros** | Edit `macros.txt` on SD — no recompile needed |
| 🧠 **State persistence** | Last active page saved to SD, restored on next launch |
| ⚡ **Last-action flash** | Executed label shown in a pill overlay for 1.5 s |
| 🔁 **Hold-to-repeat** | Hold Up/Down to repeatedly fire that action |
| 📳 **Haptic feedback** | Vibration on every command |
| 🔌 **Zero-driver** | Pure USB HID — works on Windows, macOS and Linux |
| 📖 **Built-in help** | 6-screen help guide accessible from any state |
| 🌐 **i18n-ready** | All strings centralised in `lang_en.json` |

---

## 🆕 What's New in v2

| # | Feature | Details |
|---|---|---|
| 1 | **3 new pages** | Gaming · Photoshop · Windows WM — total 6 → 9 pages |
| 2 | **SD card macros** | Edit `macros.txt` without recompiling |
| 3 | **State persistence** | Active page survives power cycle |
| 4 | **Last-action flash** | Top-right pill shows what just ran for 1.5 s |
| 5 | **Hold-to-repeat** | Hold Up or Down to repeat continuously |
| 6 | **20+ new typeable chars** | `_  !  #  $  %  ^  &  *  (  )  =  ?  ;  '  "  \t` |
| 7 | **Help expanded** | 5 → 6 screens with new pages documented |

---

## 🗂️ Pages & Shortcuts

### 🎵 Page 1 — MEDIA
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Volume Up | Consumer HID |
| ⬇️ Down | Volume Down | Consumer HID |
| ⬅️ Left | Previous Track | Consumer HID |
| ➡️ Right | Next Track | Consumer HID |
| 🔘 OK | Play / Pause | Consumer HID |

### 🖥️ Page 2 — SYSTEM
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Brightness Up | Consumer HID |
| ⬇️ Down | Show Desktop | `Win + D` |
| ⬅️ Left | Lock Screen | `Win + L` |
| ➡️ Right | Snipping Tool | `Win + Shift + S` |
| 🔘 OK | Task Manager | `Ctrl + Shift + Esc` |

### 🌐 Page 3 — BROWSER
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Reload | `F5` |
| ⬇️ Down | Fullscreen | `F11` |
| ⬅️ Left | Navigate Back | `Alt + ←` |
| ➡️ Right | Navigate Forward | `Alt + →` |
| 🔘 OK | New Tab | `Ctrl + T` |

### 💻 Page 4 — VS CODE
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Move Line Up | `Alt + ↑` |
| ⬇️ Down | Move Line Down | `Alt + ↓` |
| ⬅️ Left | Undo | `Ctrl + Z` |
| ➡️ Right | Redo | `Ctrl + Y` |
| 🔘 OK | Save | `Ctrl + S` |

### 🎥 Page 5 — OBS
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Mute Mic | `Ctrl + Shift + M` |
| ⬇️ Down | Toggle Camera | `Ctrl + Shift + V` |
| ⬅️ Left | Previous Scene | `Ctrl + ←` |
| ➡️ Right | Next Scene | `Ctrl + →` |
| 🔘 OK | Start Recording | `Ctrl + Shift + R` |

> **Note:** OBS shortcuts must be configured in **OBS → Settings → Hotkeys** to match.

### 📝 Page 6 — MACRO
| Direction | Default Text | Editable? |
|-----------|-------------|----------|
| ⬆️ Up | `Hello,\nHow are you?\n` | ✅ via `macros.txt` |
| ⬇️ Down | `Best regards,\n` | ✅ via `macros.txt` |
| ⬅️ Left | `admin@example.com` | ✅ via `macros.txt` |
| ➡️ Right | `+1 555 000 0000` | ✅ via `macros.txt` |
| 🔘 OK | `https://github.com/` | ✅ via `macros.txt` |

> See [💾 SD Card Macros](#-sd-card-macros) to edit without recompiling.

### 🎮 Page 7 — GAMING
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Push-to-Talk | `T` |
| ⬇️ Down | Mute Toggle | `Ctrl + Shift + M` |
| ⬅️ Left | Screenshot | `F12` |
| ➡️ Right | Overlay | `Shift + Tab` |
| 🔘 OK | Fullscreen | `Alt + Enter` |

### 🎨 Page 8 — PHOTOSHOP
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Zoom In | `Ctrl + =` |
| ⬇️ Down | Zoom Out | `Ctrl + -` |
| ⬅️ Left | Undo | `Ctrl + Z` |
| ➡️ Right | History Step Back | `Ctrl + Alt + Z` |
| 🔘 OK | Save As | `Ctrl + Shift + S` |

### 🪟 Page 9 — WIN WM
| Direction | Action | Shortcut |
|-----------|--------|----------|
| ⬆️ Up | Maximize | `Win + ↑` |
| ⬇️ Down | Minimize | `Win + ↓` |
| ⬅️ Left | Snap Left | `Win + ←` |
| ➡️ Right | Snap Right | `Win + →` |
| 🔘 OK | Next Virtual Desktop | `Ctrl + Win + →` |

---

## 🕹️ Navigation

### 📋 Main Menu
```
Up / Down      → Scroll through pages
OK             → Open selected page
Long OK        → Open help
Back           → Exit app
```

### 📄 Inside a Page
```
Up/Down/Left/Right/OK  → Execute action (+ vibration)
Long Left / Long Right → Jump to previous / next page
Long OK                → Open help
Back                   → Return to main menu
```

### ❓ Help Screen
```
Left / Right   → Previous / next help screen (6 total)
OK or Back     → Close help
```

---

## 🖥️ UI Layout

```
┌────────────────────────────────┐  ← 128 px
│         FlipDeck               │  Title bar (FontPrimary)
│────────────────────────────────│
│  1. MEDIA      ◀ selected      │
│  2. SYSTEM                     │  3-item scrolling list
│  3. BROWSER                    │
│────────────────────────────────│
│  OK:Open          LongOK:Help  │  Footer hints
└────────────────────────────────┘
           64 px tall
```

```
┌────────────────────────────────┐
│  [2/9] SYSTEM    ┌──────────┐  │  ← last-action pill (1.5 s)
│────────────────── └──────────┘ │
│           Bright+              │  ↑ action
│  Lock    Task Mgr   Snip       │  ← | OK box | →
│           Desktop              │  ↓ action
│────────────────────────────────│
│  Back:Menu          LongL/R:Pg │  Footer (split, no overflow)
└────────────────────────────────┘
```

> **Hold Up or Down** to continuously repeat that action (e.g. hold Up on MEDIA for continuous Vol+).

---

## 💾 SD Card Macros

Place a plain-text file at `/ext/apps_data/flip_deck/macros.txt` on your Flipper SD card.  
Each line corresponds to one macro action **in order: Up · Down · Left · Right · OK**.

```
Hello,\nHow are you?\n
Best regards,\n
your@email.com
+1 555 000 0000
https://github.com/yourname
```

| Rule | Detail |
|---|---|
| Max length | 63 characters per line |
| Line ending | `\n` (LF) or `\r\n` (CRLF) both accepted |
| Fewer than 5 lines | Remaining macros keep built-in defaults |
| Load timing | Read once on every app launch |
| Directory | Created automatically on first launch |
| Newline in text | Write `\n` literally — it becomes Enter when typed |

> The state file `/ext/apps_data/flip_deck/state.bin` stores the last selected page index (1 byte) and is read/written automatically.

---

## 🌍 Language File

All UI strings live in **`lang_en.json`** — no recompilation needed for text tweaks that map to the constant arrays.

```json
{
  "pages": {
    "media":   { "name": "MEDIA",   "up": "Vol+", "ok": "Play" },
    "system":  { "name": "SYSTEM",  "up": "Bright+", "ok": "Task Mgr" },
    "browser": { "name": "BROWSER", "up": "Reload",  "ok": "New Tab" }
  },
  "macros": {
    "greeting_text": "Hello,\nHow are you?\n",
    "email_text":    "admin@example.com"
  }
}
```

To add a new language, duplicate `lang_en.json` as `lang_xx.json` and update the string arrays in `pccontrol.c`.

---

## ⚙️ Customisation

### 📝 Changing Macro Text

**Option A — SD card (recommended, no recompile):**  
Edit `/ext/apps_data/flip_deck/macros.txt` — see [💾 SD Card Macros](#-sd-card-macros).

**Option B — hardcoded defaults:**  
Edit `macro_buf[]` in `pccontrol.c`:

```c
static char macro_buf[MACRO_COUNT][MACRO_LEN] = {
    "Hello,\nHow are you?\n",
    "Best regards,\n",
    "your@email.com",
    "+1 555 000 0000",
    "https://github.com/yourname",
};
```

### 🎥 Changing OBS Hotkeys
The OBS page uses `Ctrl+Shift+R/S/M/V` by default. You can remap them in:
- **OBS** → Settings → Hotkeys
- **or** change the `HID_CTRL_SHIFT_*` defines in `pccontrol.c`

### ➕ Adding a New Page
1. Add a new `Page` entry to the `PAGES[]` array in `pccontrol.c`
2. `PAGE_COUNT` is computed automatically via `sizeof` — no other changes needed

---

## 🔨 Building & Flashing

### Prerequisites
- [Flipper Zero](https://flipperzero.one) connected via USB-C
- [Python 3.8+](https://python.org)
- [uFBT](https://github.com/flipperdevices/flipperzero-ufbt) (`pip install ufbt`)

### Build only
```bash
python -m ufbt build
# Output: ~/.ufbt/build/flip_deck.fap
```

### Build + Flash + Launch
```bash
python -m ufbt launch
```

### Flash pre-built `.fap`
Copy `dist/flip_deck.fap` to your SD card under:
```
SD Card → apps → Tools → flip_deck.fap
```

---

## 📁 Project Structure

```
FlipDeck/
├── pccontrol.c          # Main application source (v2)
├── application.fam      # App manifest (name, id, icon, category)
├── flipdeck_icon.png    # 10×10 px 1-bit app icon
├── lang_en.json         # English UI strings reference
├── .gitignore
└── README.md

SD Card (auto-created on first launch):
/ext/apps_data/flip_deck/
├── macros.txt           # Optional: 5 macro texts, one per line
└── state.bin            # Last active page index (1 byte)
```

---

## 🤝 Contributing

Contributions are welcome!

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/new-page`
3. Commit your changes: `git commit -m "feat: add new-page"`
4. Push and open a Pull Request

**Ideas for contribution:**
- 🖼️ Figma / Canva shortcuts page
- 🔢 Numpad page (`ActionTypeText` with digits)
- 🌐 Multi-language support (new `lang_xx.json`)
- 📱 In-app macro editor (Flipper `text_input` widget)
- 🔵 BLE HID mode fallback when USB is disconnected
- 🗂️ Profile system — multiple page sets selectable from menu

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0**.  
See [LICENSE](LICENSE) for full terms.

---

<div align="center">

Made with ❤️ for the Flipper Zero community

[![Flipper Zero](https://img.shields.io/badge/Flipper%20Zero-Community-ff6600?style=for-the-badge)](https://flipperzero.one)

</div>
