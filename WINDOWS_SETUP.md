# Windows + Visual Studio 2022 setup

This project builds on Windows with no code changes — it already uses
CMake + GLFW/GLEW/GLM, all cross-platform. This guide uses **vcpkg**
(Microsoft's C++ package manager) to fetch GLFW, GLEW and GLM, and
**Visual Studio 2022's built-in CMake support** to configure, build and
run — no separate MSBuild project files needed.

## 1. Install Visual Studio 2022 workloads

Open **Visual Studio Installer** → Modify your VS2022 install → make sure
these are checked, then Modify/Install:

- **Desktop development with C++** (this brings MSVC, the Windows SDK,
  and a bundled **Ninja** + **CMake**, so you don't need to install
  those separately)
- Under "Individual components", confirm **C++ CMake tools for Windows**
  is checked (it's included by the workload above by default)

## 2. Install and bootstrap vcpkg

Open a normal **Command Prompt** or **PowerShell** window:

```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

## 3. Set the VCPKG_ROOT environment variable

Still in that PowerShell window (this sets it permanently for your user
account):

```powershell
setx VCPKG_ROOT "C:\vcpkg"
```

**Close and reopen** any terminal / Visual Studio afterwards so the new
environment variable is picked up.

> You do **not** need to manually run `vcpkg install glfw3 glew glm` —
> the project ships a `vcpkg.json` manifest, so vcpkg installs exactly
> those three packages automatically the first time you configure the
> project in the next step (this first configure can take a few minutes
> while it compiles them).

## 4. Open the project in Visual Studio 2022

1. Unzip this project somewhere, e.g. `C:\dev\opengl-atom-orbitals-windows`
2. In Visual Studio 2022: **File → Open → Folder...** and select that
   folder (do **not** use "Open a project or solution" — there is no
   `.sln`; VS reads `CMakeLists.txt` directly).
3. Visual Studio will detect `CMakePresets.json` and start configuring.
   In the toolbar dropdown that normally shows a `.sln` config, you
   should see **"Windows x64 (vcpkg, Debug)"** — select it if it isn't
   already selected.
4. Watch the **Output** window (switch its dropdown to "CMake"). On
   first run it will download/build glfw3, glew and glm via vcpkg — this
   is normal and only happens once. Wait for `CMake generation finished`.

## 5. Build and run

- Set the startup item: in the toolbar, next to the config dropdown,
  choose **`atom_viewer.exe`** as the selected item (VS usually picks
  this automatically since it's the only executable target).
- Press **Ctrl+F5** (Start Without Debugging) or **F5** (Start Debugging).
- A window titled "Atomic Orbital Viewer" should open.

If you'd rather build from the command line instead of the VS GUI:

```powershell
cmake --preset windows-vcpkg
cmake --build --preset windows-vcpkg
.\build\windows-vcpkg\atom_viewer.exe
```

## Controls

| Key          | Action                                                    |
|--------------|------------------------------------------------------------|
| Up / Down    | increase / decrease n (principal quantum number)           |
| Left / Right | decrease / increase l (orbital angular momentum, s/p/d/f)   |
| , / .        | decrease / increase m (magnetic quantum number)             |
| - / =        | decrease / increase Z (nuclear charge)                      |
| Mouse drag   | orbit the camera                                            |
| Scroll       | zoom                                                        |
| Esc          | quit                                                        |

## Troubleshooting

- **"CMAKE_TOOLCHAIN_FILE ... not found" / vcpkg errors** — `VCPKG_ROOT`
  isn't set or VS was opened before you set it. Recheck step 3, then
  fully close and reopen Visual Studio.
- **Window opens but is black / shaders fail to load** — this build
  copies the `shaders/` folder next to `atom_viewer.exe` after every
  build (see `CMakeLists.txt`), so this should not happen; if it does,
  check the Output window for "Could not open shader file" and confirm
  `build\windows-vcpkg\Debug\` (or wherever the exe landed) also
  contains a `shaders\` subfolder.
- **First configure is slow** — vcpkg is compiling GLFW/GLEW/GLM from
  source the first time; subsequent configures reuse the built packages
  and are fast.
- **No "Open Folder" CMake option / no CMakePresets dropdown** — the
  "C++ CMake tools for Windows" individual component isn't installed;
  revisit step 1.
