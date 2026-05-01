<div align="center">

# 🃏 FlipDeck

### Loupedeck-style USB HID PC Controller for Flipper Zero

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-orange.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Built with ufbt](https://img.shields.io/badge/Built%20with-ufbt-blueviolet)](https://github.com/flipperdevices/flipperzero-ufbt)
[![Flipper Zero](https://img.shields.io/badge/Flipper%20Zero-FAP%20App-ff6600)](https://flipperzero.one)
[![API](https://img.shields.io/badge/API-87.1-green)](#)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-blue)](#)

<br/>

> Turn your Flipper Zero into a compact **5-button macro pad** — control media, system shortcuts, browser, VS Code, OBS and custom text macros over USB HID. No drivers required.

<br/>

```
┌─────────────────────────┐
│  [1/6] MEDIA            │
│          Vol+           │
│  Prev   Play/Pause  Next│
│          Vol-           │
│  Back:Menu  LongL/R:Pg  │
└─────────────────────────┘
  Flipper Zero 128×64 OLED
```

</div>

---

## 📋 Table of Contents

- [✨ Features](#-features)
- [🗂️ Pages & Shortcuts](#️-pages--shortcuts)
- [🕹️ Navigation](#️-navigation)
- [🖥️ UI Layout](#️-ui-layout)
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
| 🎛️ **6 control pages** | Media · System · Browser · VS Code · OBS · Macro |
| 🔑 **30 shortcuts** | 5 directional actions per page |
| ⌨️ **Text macros** | Auto-type emails, greetings, URLs — char by char |
| 📳 **Haptic feedback** | Vibration on every command |
| 📟 **Status display** | Page name + active action labels on 128×64 OLED |
| 🔌 **Zero-driver** | Pure USB HID — works on Windows, macOS and Linux |
| 📖 **Built-in help** | 5-screen help guide accessible from any state |
| 🌐 **i18n-ready** | All strings centralised in `lang_en.json` |

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
| Direction | Text Typed |
|-----------|-----------|
| ⬆️ Up | `Hello,\nHow are you?\n` |
| ⬇️ Down | `Best regards,\n` |
| ⬅️ Left | `admin@example.com` |
| ➡️ Right | `+1 555 000 0000` |
| 🔘 OK | `https://github.com/` |

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
Left / Right   → Previous / next help screen (5 total)
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
│  [2/6] SYSTEM                  │  Page title
│────────────────────────────────│
│           Bright+              │  ↑ action
│  Lock    Task Mgr   Snip       │  ← | OK | →
│           Desktop              │  ↓ action
│────────────────────────────────│
│  Back:Menu          LongL/R:Pg │  Footer (split, no overflow)
└────────────────────────────────┘
```

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
Open `pccontrol.c` and edit the `MACRO` page strings:

```c
/* 5 -- MACRO */
{ "MACRO",
  {ActionTypeText, 0, "Hello,\nHow are you?\n", "Hello"   },
  {ActionTypeText, 0, "Best regards,\n",         "Sign Off"},
  {ActionTypeText, 0, "your@email.com",           "Email"   },
  {ActionTypeText, 0, "+1 555 000 0000",           "Phone"   },
  {ActionTypeText, 0, "https://github.com/",       "GitHub"  },
},
```

### 🎥 Changing OBS Hotkeys
The OBS page uses `Ctrl+Shift+R/S/M/V` by default. You can remap them in:
- **OBS** → Settings → Hotkeys
- **or** change the `HID_CTRL_SHIFT_*` defines in `pccontrol.c`

### ➕ Adding a New Page
1. Add a new `Page` entry to the `PAGES[]` array
2. `PAGE_COUNT` is computed automatically via `sizeof`

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
├── pccontrol.c          # Main application source
├── application.fam      # App manifest (name, id, icon, category)
├── flipdeck_icon.png    # 10×10 px 1-bit app icon
├── lang_en.json         # English UI strings reference
├── dist/                # Pre-built .fap binaries
└── README.md
```

---

## 🤝 Contributing

Contributions are welcome!

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/new-page`
3. Commit your changes: `git commit -m "feat: add new-page"`
4. Push and open a Pull Request

**Ideas for contribution:**
- 🎮 Gaming page (PTT, mute, overlay)
- 🎨 Photoshop / Figma shortcuts page
- 🌐 Multi-language support
- 🔢 Numpad page

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0**.  
See [LICENSE](LICENSE) for full terms.

---

<div align="center">

Made with ❤️ for the Flipper Zero community

[![Flipper Zero](https://img.shields.io/badge/Flipper%20Zero-Community-ff6600?style=for-the-badge)](https://flipperzero.one)

</div>
